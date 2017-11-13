/*
 * Server-Client.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MYPORT "13531" //port for connection
#define BACKLOG 10 //pending connections quene will hold

static void die(const char *s) {
    perror(s);
    exit(1);
}


int main(void)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    getaddrinfo(NULL, MYPORT, &hints, &res);

    // make a socket, bind it. and listen on it.
    
    if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        die("socket failed");

    if(bind(sockfd, res->ai_addr, res->ai_addrlen) < 0)
        die("bind failed");

    if(listen(sockfd, BACKLOG) < 0)
        die("listen failed");

    // Now accept an incoming connection:

    addr_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    //ready to communicate
    //....
}
