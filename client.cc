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

#include <inttypes.h>
#include "message.h"
#include "client.h"
#include "socket-client.h"
#include "helper.h"

using namespace std;

Cmd_Msg_T prepare_data_packet(uint8_t cmd, uint16_t port) {
    Cmd_Msg_T *msg = (Cmd_Msg_T *)malloc(sizeof(Cmd_Msg_T));
    msg->cmd = cmd;
    msg->port = port;
    return *(msg);
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
    // designate the addressing family
    remote.sin_family = AF_INET;
    remote.sin_port = htons(udp_port);

    // get the address of the remote host and store
    hp = gethostbyname(server_host);
    memcpy(&remote.sin_addr, hp->h_addr, hp->h_length);

    Client_State_T client_state = WAITING;
    string input, args[3];
    while(true) {
        usleep(100);

        switch(client_state) {
            case WAITING:
            {
                cout << "$ ";
                getline(cin, input);
                int count = 0;
                for(auto x : input){
                    if(x == ' '){
                        cout << args[count] << endl;
                        count++;
                    }else{
                        args[count]+= x;
                    }
                }
                if(args[0] == "ls") {
                    Cmd_Msg_T msg = {.cmd = CMD_LS};
                    sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, sizeof(remote));
                    client_state = PROCESS_LS;
                }
                else if(args[0] == "send") {
                    client_state = PROCESS_SEND;
                }
                else if(args[0] == "remove") {
                    Cmd_Msg_T msg = {.cmd = CMD_REMOVE};
                    memcpy(msg.filename, args[1].c_str(), FILE_NAME_LEN);
                    sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, sizeof(remote));
                    client_state = PROCESS_REMOVE;
                }
                else if(args[0] == "rename") {
                    Cmd_Msg_T msg = {.cmd = CMD_RENAME};
                    memcpy(msg.filename, args[1].c_str(), FILE_NAME_LEN);
                    memcpy(msg.expected_filename, args[2].c_str(), FILE_NAME_LEN);
                    sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, sizeof(remote));
                    client_state = PROCESS_RENAME;
                }
                else if(args[0] == "shutdown") {
                    Cmd_Msg_T msg = {.cmd = CMD_SHUTDOWN};
                    sendto(sk, &msg, sizeof(msg), 0, (struct sockaddr *)&remote, sizeof(remote));
                    client_state = SHUTDOWN;
                }
                else if(args[0] == "quit") {
                    client_state = QUIT;
                }
                else{
                    cout<<" - wrong command."<<endl;
                    client_state = WAITING;
                }
                break;
            }
            case PROCESS_LS:
            {
                // wait for a response and print it
                Cmd_Msg_T response;
                msglen = recvfrom(sk, &response, sizeof(response), 0, (struct sockaddr *)&remote, &rlen);
                int size = ntohl(response.size);
                if (int(response.cmd) != CMD_LS){
                    cout << " - command response error.";
                }else{
                    Data_Msg_T data_msg;
                    for (int i = 0; i < size; i++) {
                        do {
                            msglen = recvfrom(sk, &data_msg, sizeof(data_msg), 0, (struct sockaddr *)&remote, &rlen);
                            cout << data_msg.data;
                        } while (msglen < sizeof(*buf));
                    }
                }
                cout << endl;
                client_state = WAITING;
                break;
            }
            case PROCESS_SEND:
            {
                
                client_state = WAITING;
                break;
            }
            case PROCESS_REMOVE:
            {
                Cmd_Msg_T response;
                if (read(sk, &response, sizeof(response)) == -1) {
                    cout << "error!" <<endl;
                } else {
                    if (int(response.cmd) == CMD_ACK && response.error == 1) {
                        cout << " - file doesn't exist." << endl;
                        client_state = WAITING;
                    }
                }
                client_state = WAITING;
                break;
            }
            case PROCESS_RENAME:
            {
                Cmd_Msg_T response;
                if (read(sk, &response, sizeof(response)) == -1) {
                    cout << "error!" <<endl;
                } else {
                    if (int(response.cmd) == CMD_ACK && response.error == 1) {
                        cout << " - file doesn't exist." << endl;
                        client_state = WAITING;
                    }
                }
                client_state = WAITING;
                break;
            }
            case SHUTDOWN:
            {
                Cmd_Msg_T response;
                if (read(sk, &response, sizeof(response)) == -1) {
                    cout << "error!" <<endl;
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
