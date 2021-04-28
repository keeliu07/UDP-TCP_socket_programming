#ifndef _SOCKET_CLIENT_H
#define _SOCKET_CLIENT_H

#include <netdb.h>
#define BUFLEN 3000

int sk;
int tcpsk;
int msglen;
char buf[BUFLEN];
hostent *hp;
sockaddr_in remote;
sockaddr_in tcp;
socklen_t rlen = sizeof(remote);
socklen_t tlen = sizeof(tcp);

#endif
