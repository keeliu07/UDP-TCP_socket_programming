#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <time.h>

#include <random>
#include "message.h"
#include "server.h"
#include "socket-server.h"
#include "helper.h"
// using namespace std;

Server_State_T server_state;
int counter = 0;

void _prepare_and_send_data_packet(string data) {
    Data_Msg_T data_msg;
    memcpy(data_msg.data, data.c_str(), BUFLEN);
    sendto(sk, &data_msg, sizeof(data_msg), 0, (struct sockaddr *)&remote, rlen);
}

void _prepare_and_send_msg_packet(uint8_t cmd, uint32_t size) {
    Cmd_Msg_T msg = {.cmd = cmd, .size = htonl(size)};
    sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, rlen);
}

void resetState(){
    server_state = WAITING;
    counter = 0;
}

int main(int argc, char *argv[]) {

    unsigned short udp_port = 0;
    if ((argc != 1) && (argc != 3)){
        cout << "Usage: " << argv[0];
        cout << " [-p <udp_port>]" << endl;
        return 1;
    }else{
        //system("clear");
        //process input arguments
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-p") == 0)
                udp_port = (unsigned short)atoi(argv[++i]);
            else{
                cout << "Usage: " << argv[0];
                cout << " [-p <udp_port>]" << endl;
                return 1;
            }
        }
    }

    // create and set up socket
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk == -1){
        perror("opening datagram socket");
        return 1;
    }
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(udp_port);

    // bind address name to a port
    if (::bind(sk, (struct sockaddr *)&local, sizeof(local)) == -1) {
        perror("binding datagram socket");
        return 1;
    }

    // get port name
    if (::getsockname(sk, (struct sockaddr *)&local, &len) == -1) {
        perror("getting socket name");
        exit(1);
    }
    // cout << "socket has port " << ntohs(local.sin_port) << "\n";
    // cout << "socket has addr " << local.sin_addr.s_addr << "\n";

    string in_cmd;
    Cmd_Msg_T msg;
    while (true) {
        usleep(100);

        switch (server_state) {
        case WAITING:
        {
            if (counter == 0)
                cout << "Waiting UDP command @: " << ntohs(local.sin_port) << endl;
            while(true){
                msglen = recvfrom(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, &rlen);
                if (msglen < 0 || msglen > 0) break;
            }
            cout << "[CMD RECEIVED]: " << CMD_TAG_MAP[msg.cmd] << endl;
            server_state = SERVER_STATE_MAP[msg.cmd];
            counter++;
            break;
        }
        case PROCESS_LS: 
        {
            invoke_ls(msg);
            resetState();
            break;
        }
        case PROCESS_SEND:
        {
            resetState();
            break;
        }
        case PROCESS_REMOVE:
        {
            Cmd_Msg_T ack = {.cmd = CMD_ACK};
            string path = "../backup/" + string(msg.filename);
            if(checkFile(path.c_str())){
                if(remove(path.c_str()) != 0){
                    // cout << "File deletion failed.";
                }else{
                    // cout << "File Deleted.";
                    ack.error = 0;
                }
            }else{
                ack.error = 1;
                cout << " - file doesn't exist." << endl;
            }
            sendto(sk, &ack, sizeof(ack), 0, (struct sockaddr *)&remote, sizeof(remote));
            resetState();
            break;
        }
        case PROCESS_RENAME:
        {
            Cmd_Msg_T ack = {.cmd = CMD_ACK};
            string path = "../backup/";
            if(checkFile((path+string(msg.filename)).c_str())){
                if (rename((path + string(msg.filename)).c_str(), (path + string(msg.expected_filename)).c_str())==0){
                    ack.error = 0;
                }else{
                    ack.error = 1;
                    cout << " - error renaming file." << endl;
                }
            }else{
                ack.error = 1;
                cout << " - file doesn't exist." << endl;
            }
            sendto(sk, &ack, sizeof(ack), 0, (struct sockaddr *)&remote, sizeof(remote));
            resetState();
            break;
        }
        case SHUTDOWN:
        {
            Cmd_Msg_T ack = {.cmd = CMD_ACK, .error = 0};
            sendto(sk, &ack, sizeof(ack), 0, (struct sockaddr *)&remote, sizeof(remote));
            close(sk);
            return 0;
        }
        default:
        {
            resetState();
            break;
        }
        }
    }
    return 0;
}

//this function check if the backup folder exist
int checkDirectory(string dir) {
    DIR *dp;
    if ((dp = opendir(dir.c_str())) == NULL) {
        //cout << " - error(" << errno << ") opening " << dir << endl;
        if (mkdir(dir.c_str(), S_IRWXU) == 0)
            cout << " - Note: Folder " << dir << " does not exist. Created." << endl;
        else
            cout << " - Note: Folder " << dir << " does not exist. Cannot created." << endl;
        return errno;
    }
    closedir(dp);
    return 0;
}

//this function is used to get all the filenames from the
//backup directory
int getDirectory(string dir, vector<string> &files) {
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL) {
        //cout << " - error(" << errno << ") opening " << dir << endl;
        if (mkdir(dir.c_str(), S_IRWXU) == 0)
            cout << " - Note: Folder " << dir << " does not exist. Created." << endl;
        else
            cout << " - Note: Folder " << dir << " does not exist. Cannot created." << endl;
        return errno;
    }

    int j = 0;
    while ((dirp = readdir(dp)) != NULL) {
        //do not list the file "." and ".."
        if ((string(dirp->d_name) != ".") && (string(dirp->d_name) != ".."))
            files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

//this function check if the file exists
bool checkFile(const char *fileName) {
    ifstream infile(fileName);
    return infile.good();
}

int invoke_ls(Cmd_Msg_T incoming_msg) {
    vector<string> files_vect;
    getDirectory("../backup/", files_vect);
    _prepare_and_send_msg_packet(incoming_msg.cmd, (uint32_t)files_vect.size());
    if (files_vect.size() == 0) {
        string msg = " - server backup folder is empty.";
        _prepare_and_send_data_packet(msg);
        cout << msg;
    }else{
        for (int i = 0; i < files_vect.size(); i++) {
            string msg = " - " + files_vect[i] + " \0";
            _prepare_and_send_data_packet(msg);
            cout << msg;
        }
    }
    cout << endl;
    return 0;
}
