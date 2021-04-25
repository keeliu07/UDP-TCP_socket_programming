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
// using namespace std;

Server_State_T server_state;

void _prepare_send_data_packet(string data) {
    strcpy(remotebuf, data.c_str());
    Data_Msg_T data_msg = {.data = *remotebuf};
    sendto(sk, &data_msg, strlen((const char *)&data_msg), 0, (struct sockaddr *)&remote, rlen);
}

void _prepare_send_msg_packet(uint8_t cmd, uint32_t size, uint16_t port, uint16_t error) {
    Cmd_Msg_T msg = {.cmd = cmd, .size = size, .port = port, .error = error};
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
     int counter = 0;
    while (true) {
        usleep(100);

        switch (server_state) {
        case WAITING:
        {
            if (counter == 0)
                cout << "Waiting UDP command @: " << ntohs(local.sin_port) << endl;
            Cmd_Msg_T msg;
            while(true){
                msglen = recvfrom(sk, &msg, strlen((const char *)&msg), 0, (struct sockaddr *)&remote, &rlen);
                if (msglen < 0 || msglen > 0)
                    break;
            }
            cout << "[CMD RECEIVED]: " << CMD_TAG_MAP[msg.cmd] << endl;
            server_state = SERVER_STATE_MAP[msg.cmd];
            counter++;
            break;
        }
        case PROCESS_LS: 
        {
            invoke_ls();
            server_state = WAITING;
            break;
        }
        case PROCESS_SEND:
        {
            server_state = WAITING;
            break;
        }
        case PROCESS_REMOVE:
        {
            server_state = WAITING;
            break;
        }
        case PROCESS_RENAME:
        {
            server_state = WAITING;
            break;
        }
        case SHUTDOWN:
        {
        }
        default:
        {
            server_state = WAITING;
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

int invoke_ls() {
    vector<string> files_vect;
    getDirectory("../backup/", files_vect);
    if (files_vect.size() == 0){
        string data_msg = "- server backup folder is empty.";
        cout << data_msg << endl;
        _prepare_send_data_packet(data_msg);
    }else{
        for (int i = 0; i < files_vect.size(); i++) {
            cout << " - " << files_vect[i] << endl;
        }
    }

    return 0;
}
