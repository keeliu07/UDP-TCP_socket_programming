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

#include "message.h"
#include "client.h"
#include "socket-client.h"

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
    // cout << remote.sin_port << endl;
    // cout << ntohs(remote.sin_port) << endl;

    // get the address of the remote host and store
    hp = gethostbyname(server_host);
    memcpy(&remote.sin_addr, hp->h_addr, hp->h_length);
    // cout << hp->h_name << endl;

    // send message telling it to shut down
    // sendto(sk, MSG2, strlen(MSG2), 0, (struct sockaddr *)&remote, sizeof(remote));

    Client_State_T client_state = WAITING;
    string in_cmd;
    while(true) {
        usleep(100);

        switch(client_state) {
            case WAITING:
            {
                cout << "$ ";
                cin>>in_cmd;

                if(in_cmd == "ls") {
                    client_state = PROCESS_LS;
                }
                else if(in_cmd == "send") {
                    client_state = PROCESS_SEND;
                }
                else if(in_cmd == "remove") {
                    client_state = PROCESS_REMOVE;
                }
                else if(in_cmd == "rename") {
                    client_state = PROCESS_RENAME;
                }
                else if(in_cmd == "shutdown") {
                    client_state = SHUTDOWN;
                }
                else if(in_cmd == "quit") {
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
                Cmd_Msg_T msg = {.cmd = CMD_LS, .port = remote.sin_port};
                // cout << unsigned(msg.cmd) << "\n";
                // send the message to the other side
                sendto(sk, &msg, strlen((const char *)&msg), 0, (struct sockaddr *)&remote, sizeof(remote));
                // wait for a response and print it
                msglen = read(sk, buf, BUFLEN);
                buf[msglen] = '\0';
                cout << buf << "\n";
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
                client_state = WAITING;
                break;
            }
            case PROCESS_RENAME:
            {
                client_state = WAITING;
                break;
            }
            case SHUTDOWN:
            {
                break;
            }
            case QUIT:
            {
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
