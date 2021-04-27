#ifndef _SOCKET_CLIENT_H
#define _SOCKET_CLIENT_H

#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

using namespace std;
#define BUFLEN 3000
#define MSG1 "Have you heard about the new corduroy pillows?\nThey are making headlines!"
#define MSG2 "..."

int sk;
sockaddr_in remote;
int tcp_sk;
sockaddr_in tcp;
char buf[BUFLEN];
hostent *hp;
int msglen;
socklen_t rlen = sizeof(remote);
socklen_t tlen = sizeof(tcp);

#endif
