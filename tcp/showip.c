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
    struct addrinfo hints, *rec, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2){
        fprintf(stderr, "usage: showip hostname\n");
        return 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; //AF_INET or AF_INET6
    hints.ai_socktype = SOCK_STREAM;
}
