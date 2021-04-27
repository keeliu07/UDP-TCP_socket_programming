#ifndef _SOCKET_SERVER_H
#define _SOCKET_SERVER_H

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

#define BUFLEN 3000
#define LOCALHOST "127.0.0.1"

int sk;
int tcp_sk;
sockaddr_in remote;
sockaddr_in local;
sockaddr_in tcp;
char buf[BUFLEN];
char remotebuf[BUFLEN];
socklen_t rlen = sizeof(remote);
socklen_t len = sizeof(local);
socklen_t tlen = sizeof(tcp);
int msglen;

#endif
