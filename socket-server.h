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
sockaddr_in remote;
sockaddr_in local;
char buf[BUFLEN];
char remotebuf[BUFLEN];
socklen_t rlen = sizeof(remote);
socklen_t len = sizeof(local);
int msglen;

int rot13(char *inbuf, char *outbuf)
{
    int idx;
    if (inbuf[0] == '.')
        return 0;
    idx = 0;
    while (inbuf[idx] != '\0')
    {
        if (isalpha(inbuf[idx]))
        {
            if ((inbuf[idx] & 31) <= 13)
                outbuf[idx] = inbuf[idx] + 13;
            else
                outbuf[idx] = inbuf[idx] - 13;
        }
        else
            outbuf[idx] = inbuf[idx];
        idx++;
    }
    outbuf[idx] = '\0';
    return 1;
}

#endif
