#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

#define MSG1 "Have you heard about the new corduroy pillows?\nThey are making headlines!"
#define MSG2 "..."
#define BUFLEN 356

int main(int argc, char *argv[]) {
    int sk;
    sockaddr_in remote;
    char buf[BUFLEN];
    hostent *hp;
    int msglen;

    sk = socket(AF_INET, SOCK_DGRAM, 0);

    // designate the addressing family
    remote.sin_family = AF_INET;

    // get the address of the remote host and store
    hp = gethostbyname(argv[1]);
    memcpy(&remote.sin_addr, hp->h_addr, hp->h_length);
    remote.sin_port = atoi(argv[2]);

    // send the message to the other side
    // sendto(sk, MSG1, strlen(MSG1), 0, (struct sockaddr *)&remote, sizeof(remote));
    
    // // wait for a response and print it
    // msglen = read(sk, buf, BUFLEN);
    // buf[msglen] = '\0';
    // cout << buf << "\n";

    // // send message telling it to shut down 
    // sendto(sk,MSG2,strlen(MSG2),0,(struct sockaddr *)&remote, sizeof(remote));

    // close(sk);

    return 0;
}