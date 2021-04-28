#include <sys/types.h>
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
#include <netdb.h>
#include <algorithm>
#include <sys/stat.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include "message.h"
#include "client.h"
#include "socket-client.h"

// accepts absoulte and relative filepath
string getFileAbsolutePath(string filepath) {
    char buffer[PATH_MAX];
    char *absolute_path = realpath(filepath.c_str(), buffer);
    if (absolute_path == NULL) {
        cout << "Cannot find file at " << filepath << endl;
        return "";
    } else {
        return string(buffer);
    }
}

long long getFileSize(string filepath) {
    string absolutepath = getFileAbsolutePath(filepath);
    struct stat stat_buf;
    int rc = stat(absolutepath.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int main(int argc, char *argv[]) {
    unsigned short udp_port = 0;
    const char* server_host = "127.0.0.1";
    //process input arguments
    if ((argc != 3) && (argc != 5)){
        cout << "Usage: " << argv[0];
        cout << " [-s <server_host>] -p <udp_port>" << endl;
        return 1;
    } else {
        //system("clear");
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-p") == 0)
                udp_port = (unsigned short) atoi(argv[++i]);
            else if (strcmp(argv[i], "-s") == 0) {
                server_host = argv[++i];
                if (argc == 3) {
                    cout << "Usage: " << argv[0];
                    cout << " [-s <server_host>] -p <udp_port>" << endl;
                    return 1;
                }
            }
            else {
                cout << "Usage: " << argv[0];
                cout << " [-s <server_host>] -p <udp_port>" << endl;
                return 1;
            }
        }
    }

    // create socket
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (sk == -1) {
        cout << " - UDP socket creation failed." << endl;
        close(sk);
        return 1;
    }
    // designate the addressing family
    remote.sin_family = AF_INET;
    remote.sin_port = htons(udp_port);
    // remote.sin_addr.s_addr = inet_addr(server_host);

    // get the address of the remote host and store
    hp = gethostbyname(server_host);
    if(hp == NULL){
        cout << " - unknow host " << argv[1] << endl;
        return 1;
    }
    memcpy(&remote.sin_addr, hp->h_addr, hp->h_length);

    Client_State_T client_state = WAITING;
    string input, args[3];
    while(true) {
        usleep(100);

        switch(client_state) {
            case WAITING:
            {
                input = "";
                memset(args,0,sizeof(args));
                cout << "$ ";
                getline(cin, input);
                int count = 0;
                for(auto x : input){
                    if(x == ' ') count++;
                    else args[count]+= x;
                }
                if(args[0] == "ls") {
                    Cmd_Msg_T msg = {.cmd = CMD_LS};
                    sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, sizeof(remote));
                    client_state = PROCESS_LS;
                    break;
                }
                else if(args[0] == "send") {
                    client_state = PROCESS_SEND;
                    break;
                }
                else if(args[0] == "remove") {
                    //ok
                    if(args[1] != "" ){
                        Cmd_Msg_T msg = {.cmd = CMD_REMOVE};
                        memcpy(msg.filename, args[1].c_str(), FILE_NAME_LEN);
                        sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, sizeof(remote));
                        client_state = PROCESS_REMOVE;
                        break;
                    }
                }
                else if(args[0] == "rename") {
                    //ok
                    if(args[1] != ""  && args[2] != "" ){
                        Cmd_Msg_T msg = {.cmd = CMD_RENAME};
                        memcpy(msg.filename, args[1].c_str(), FILE_NAME_LEN);
                        memcpy(msg.expected_filename, args[2].c_str(), FILE_NAME_LEN);
                        sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, sizeof(remote));
                        client_state = PROCESS_RENAME;
                        break;
                    }
                }
                else if(args[0] == "shutdown") {
                    //ok
                    Cmd_Msg_T msg = {.cmd = CMD_SHUTDOWN};
                    sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, sizeof(remote));
                    client_state = SHUTDOWN;
                    break;
                }
                else if(args[0] == "quit") {
                    //ok
                    client_state = QUIT;
                    break;
                }

                cout<<" - wrong command."<<endl;
                client_state = WAITING;
                break;
            }
            case PROCESS_LS:
            {
                // wait for a response and print it
                Cmd_Msg_T response;
                msglen = recvfrom(sk, &response, sizeof(response), 0, (struct sockaddr *)&remote, &rlen);
                int size = ntohl(response.size);
                if (int(response.cmd) != CMD_LS){
                    cout << " - command response error." << endl;
                }else{
                    if(size > 0){
                        Data_Msg_T data_msg;
                        for (int i = 0; i < size; i++) {
                            do {
                                msglen = recvfrom(sk, &data_msg, sizeof(data_msg), 0, (struct sockaddr *)&remote, &rlen);
                                cout << data_msg.data;
                            } while (msglen < sizeof(*buf));
                        }
                        cout << endl;
                    }else{
                        cout << " - server backup folder is empty." << endl;
                    }
                }
                client_state = WAITING;
                break;
            }
            case PROCESS_SEND:
            {
                Cmd_Msg_T msg;
                msg.cmd = CMD_SEND;
                msg.error = 0;
                memcpy(msg.filename, args[1].c_str(), FILE_NAME_LEN);
                unsigned long filesize = getFileSize(args[1]);
                if (filesize != -1){
                    msg.size = htonl(filesize);
                }
                sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, rlen);
                cout << " - filesize: " << filesize << endl;

                Cmd_Msg_T response;
                int msglen;
                msglen = recvfrom(sk, &response, sizeof(response), 0, (struct sockaddr *)&remote, &rlen);
                if(response.error == 2){
                    cout << " - file exists. overwrite? (y/n): ";
                    char choice;
                    scanf("%c", &choice);
                    Cmd_Msg_T send = {.cmd = CMD_SEND};
                    if (choice == 'y'){
                        send.error = 0;
                    }else{
                        send.error = 2;
                    }
                    sendto(sk, &send, sizeof(send), 0, (struct sockaddr *)&remote, rlen);
                    if(send.error ==2){
                        // return to WAITING state
                        client_state = WAITING;
                        break;
                    }
                 }else if(response.error == 0){
                    if (int(response.cmd) != CMD_SEND) {
                        cout << " - command response error." << endl;
                    } else {
                        uint16_t tcp_port = ntohs(response.port);
                        cout << " - TCP port: " << tcp_port << endl;

                        // create tcp socket and config
                        tcpsk = socket(AF_INET, SOCK_STREAM, 0);
                        tcp.sin_family = AF_INET;
                        tcp.sin_port = htons(tcp_port);
                        memcpy(&tcp.sin_addr, hp->h_addr, hp->h_length);

                        //ebtablish tcp connection
                        int establish = connect(tcpsk, (struct sockaddr *)&tcp, tlen);
                        if(establish != -1){
                            // connected to server with tcp

                            // read file into buffer
                            string path = getFileAbsolutePath(args[1]);
                            FILE *file = fopen(path.c_str(), "rb");
                            char *file_bytes = (char*)malloc(filesize);
                            fread(file_bytes, 1, filesize, file);
                            fclose(file);
                            file_bytes[filesize] = '\0';

                            // split buffer into chunks to send
                            long done = 0;
                            while(done < filesize ){
                                long available = min(DATA_BUF_LEN, (int)(filesize - done));
                                char buff[BUFLEN];
                                memset(buff, 0, sizeof(buff));
                                memcpy(buff, file_bytes + done, available);
                                write(tcpsk, buff, sizeof(buff));
                                done+=available;
                            }

                            // wait for ack
                            Cmd_Msg_T ack;
                            msglen = recvfrom(sk, &ack, sizeof(ack), 0, (struct sockaddr *)&remote, &rlen);
                            if (ack.error == 0) {
                                cout << " - file transmission is completed." << endl;
                            }else{
                                cout << " - file transmission is failed." << endl;
                            }

                        }else{
                            cout << " - failed to connect server with TCP."<<endl;
                        }
                    }
                    client_state = WAITING;
                    break;
                 }
            }
            case PROCESS_REMOVE:
            {
                Cmd_Msg_T response;
                msglen = recvfrom(sk, &response, sizeof(response), 0, (struct sockaddr *)&remote, &rlen);
                if (msglen == -1) {
                    cout << "error: " << errno << endl;
                } else {
                    if (int(response.cmd) == CMD_ACK && response.error == 1) {
                        cout << " - file doesn't exist." << endl;
                        client_state = WAITING;
                    }else{
                        cout << " - file is removed." << endl;
                    }
                }
                client_state = WAITING;
                break;
            }
            case PROCESS_RENAME:
            {
                Cmd_Msg_T response;
                msglen = recvfrom(sk, &response, sizeof(response), 0, (struct sockaddr *)&remote, &rlen);
                if (msglen == -1) {
                    cout << "error: " << errno << endl;
                } else {
                    if (int(response.cmd) == CMD_ACK && response.error == 1) {
                        cout << " - file doesn't exist." << endl;
                        client_state = WAITING;
                    }else{
                        cout << " - file has been renamed." << endl;
                    }
                }
                client_state = WAITING;
                break;
            }
            case SHUTDOWN:
            {
                Cmd_Msg_T response;
                msglen = recvfrom(sk, &response, sizeof(response), 0, (struct sockaddr *)&remote, &rlen);
                if (msglen == -1) {
                    cout << "error: " << errno << endl;
                } else {
                    if (int(response.cmd) == CMD_ACK) {
                        cout << " - server is shutdown." << endl;
                        client_state = WAITING;
                    }
                }
                break;
            }
            case QUIT:
            {
                close(tcpsk);
                close(sk);
                return 0;
            }
            default:
            {
                client_state = WAITING;
                break;
            }
       }
    }
    return 0;
}
