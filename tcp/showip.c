/*
 * showip.c -- Show ip address for a host given on the command line
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char**argv)
{
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2){
        fprintf(stderr, "usage: showip hostname\n");
        return 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; //AF_INET or AF_INET6
    hints.ai_socktype = SOCK_STREAM;

    if((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0){
            fprintf(stderr, "getaddinfo: %s\n", gai_strerror(status));
            return 2;
    }

    printf("IP address for %s:\n\n", argv[1]);
}
