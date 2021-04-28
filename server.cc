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

#include "message.h"
#include "server.h"
#include "socket-server.h"

Server_State_T server_state;
int counter = 0;

void _prepare_and_send_data_packet(string data) {
    Data_Msg_T data_msg;
    memcpy(data_msg.data, data.c_str(), BUFLEN);
    sendto(sk, &data_msg, sizeof(data_msg), 0, (struct sockaddr *)&remote, rlen);
}

void _prepare_and_send_msg_packet(uint8_t cmd, uint32_t size) {
    Cmd_Msg_T msg;
    msg.cmd = cmd;
    msg.size = htonl(size);
    sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, rlen);
}

void resetState(){
    server_state = WAITING;
    counter = 0;
}

int create_tcp_socket(){
    // create and set up socket
    tcp_sk = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sk == -1) {
        perror("opening datagram socket");
        return 1;
    }
    tcp.sin_family = AF_INET;
    tcp.sin_addr.s_addr = INADDR_ANY;
    tcp.sin_port = htons(0);

    // bind address name to a port
    if (::bind(tcp_sk, (struct sockaddr *)&tcp, tlen) == -1) {
        perror("binding datagram socket");
        return 1;
    }

    // get port name
    if (::getsockname(tcp_sk, (struct sockaddr *)&tcp, &tlen) == -1)
    {
        perror("getting socket name");
        return 1;
    }
    cout << " - listen @: " << ntohs(tcp.sin_port) << endl;
    return 0;
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
            cout << " - filename: " << msg.filename << endl;
            cout << " - filesize: " << ntohl(msg.size) << endl;
            string path = "../backup/" + string(msg.filename);

            Cmd_Msg_T response = {.cmd = CMD_SEND};
            if(checkFile(path.c_str())){
                // File is alreay exists
                response.error = 2;
                cout << "file " << msg.filename << " exists; overwrite?" << endl;
            }else{
                // File does not exist
                create_tcp_socket();
                response.error = 0;
                response.port = tcp.sin_port;
            }
            sendto(sk, &response, sizeof(response), 0, (struct sockaddr *)&remote, rlen);

            if(response.error == 2){
                // wait for a response
                Cmd_Msg_T receive;
                msglen = recvfrom(sk, &receive, sizeof(receive), 0, (struct sockaddr *)&remote, &rlen);
                if (msglen == -1) {
                    cout << "error: " << errno << endl;
                } else {
                    if(receive.error == 1){
                        response.error = 0;
                        response.port = tcp.sin_port;
                        sendto(sk, &response, sizeof(response), 0, (struct sockaddr *)&remote, rlen);
                    }else{
                        resetState();
                        break;
                    }
                }
            }

            // start listen for client's connection
            listen(tcp_sk, 1);
            int connection = accept(tcp_sk, (struct sockaddr *)&tcp, &tlen);
            if(connection != -1){
                cout << " - connected with client." << endl;
                // read data
                long bytes = ntohl(msg.size);
                char *file_buffer = (char *)malloc(bytes);
                long bytes_read = 0;
                while(bytes_read < bytes){
                    long result = read(connection, file_buffer + bytes_read, bytes - bytes_read);
                    if(result < 1) cout << " - message reception error." << endl;
                    cout << result << endl;
                    bytes_read += result;
                    cout << " - total bytes received: " << bytes_read << endl;
                }
                cout << " - " << msg.filename << " has been received." << endl;

                // create and write data to file
                FILE* file = fopen(path.c_str(), "wb");
                fwrite(file_buffer, 1, bytes_read, file);
                fclose(file);

                // send ack
                Cmd_Msg_T ack = {.cmd = CMD_ACK};
                ack.error = 0;
                sendto(sk, &ack, sizeof(ack), 0, (struct sockaddr *)&remote, sizeof(remote));
                cout << "- send acknowledgement." << endl;
            }else{
                cout << " - failed to accept TCP connection." << endl;
            }
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
                    cout << " - " << path << " has been removed." << endl;
                    ack.error = 0;
                }
            }else{
                ack.error = 1;
                cout << " - file doesn't exist." << endl;
            }
            cout << " - send acknowledgement." << endl;
            sendto(sk, &ack, sizeof(ack), 0, (struct sockaddr *)&remote, sizeof(remote));
            resetState();
            break;
        }
        case PROCESS_RENAME:
        {
            Cmd_Msg_T ack = {.cmd = CMD_ACK};
            string path = "../backup/";
            string orig_filename = path + string(msg.filename);
            string expect_filename = path + string(msg.expected_filename);
            if(checkFile(orig_filename.c_str())){
                if (rename(orig_filename.c_str(), expect_filename.c_str())==0){
                    cout << " - the file has been renamed to " << expect_filename << "." << endl;
                    ack.error = 0;
                }else{
                    ack.error = 1;
                    cout << " - error renaming file." << endl;
                }
            }else{
                ack.error = 1;
                cout << " - file doesn't exist." << endl;
            }
            cout << " - send acknowledgement." << endl;
            sendto(sk, &ack, sizeof(ack), 0, (struct sockaddr *)&remote, sizeof(remote));
            resetState();
            break;
        }
        case SHUTDOWN:
        {
            Cmd_Msg_T ack;
            ack.cmd = CMD_ACK;
            ack.error = 0;
            cout << " - send acknowledgement." << endl;
            sendto(sk, &ack, sizeof(ack), 0, (struct sockaddr *)&remote, sizeof(remote));
            close(tcp_sk);
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
    _prepare_and_send_msg_packet(CMD_LS, (uint32_t)files_vect.size());
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
