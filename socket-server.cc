#include <iostream>
#include <unistd.h>
#include <netinet/in.h>

using namespace std;

#define BUFLEN 356

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

int main()
{
    int sk;
    sockaddr_in remote_addr;
    sockaddr_in local;
    char buf[BUFLEN];
    char retbuf[BUFLEN];
    socklen_t rlen = sizeof(remote_addr);
    socklen_t len = sizeof(local);
    int moredata = 1;
    int mesglen;

    sk = socket(AF_INET, SOCK_DGRAM, 0);

    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = 0;

    bind(sk, (struct sockaddr *)&local, len);

    getsockname(sk, (struct sockaddr *)&local, &len);
    cout << "socket has port " << local.sin_port << "\n";
    cout << "socket has addr " << local.sin_addr.s_addr << "\n" ;

    while(moredata){
        mesglen = recvfrom(sk, buf,BUFLEN,0,(struct sockaddr *)&remote_addr, &rlen);
        buf[mesglen] = '\0';
        cout << buf << "\n";
        moredata = rot13(buf, retbuf);
        if (moredata) {
            // send a reply, using the address given in remote
            sendto(sk, retbuf, strlen(retbuf), 0, (struct sockaddr *)&remote_addr, rlen);
        }
    }

    close(sk);

    return 0;
}
