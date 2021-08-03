#ifndef _SOCKET_SERVER_H
#define _SOCKET_SERVER_H

#include <netdb.h>
#include <string>

#define BUFLEN 3000

int sk;
int tcpsk;
int msglen;
char buf[BUFLEN];
char remotebuf[BUFLEN];

sockaddr_in remote;
sockaddr_in local;
sockaddr_in tcp;
socklen_t rlen = sizeof(remote);
socklen_t len = sizeof(local);
socklen_t tlen = sizeof(tcp);

int create_tcp_socket();
void _prepare_and_send_data_packet(std::string data);
void _prepare_and_send_msg_packet(uint8_t cmd, uint32_t size);
void resetState();

#endif
