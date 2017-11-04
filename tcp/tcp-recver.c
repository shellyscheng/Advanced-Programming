/*
 * tcp-recver.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

static void die(const char *s) { perror(s); exit(1); }

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <server-port> <filebase>\n", argv[0]);
        exit(1);
    }

    unsigned short port = atoi(argv[1]);
    const char *filebase = argv[2];

    // Create a listening socket (also called server socket) 

    int servsock;
    if ((servsock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("socket failed");

    // Construct local address structure

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // any network interface
    servaddr.sin_port = htons(port);

    // Bind to the local address

    if (bind(servsock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        die("bind failed");

    // Start listening for incoming connections

    if (listen(servsock, 5 /* queue size for connection requests */ ) < 0)
        die("listen failed");

    int clntsock;
    socklen_t clntlen;
    struct sockaddr_in clntaddr;

    FILE *fp;
    unsigned int filesuffix = 0;
    char filename[strlen(filebase) + 100];

    int r;
    char buf[4096];
    uint32_t size, size_net, remaining, limit;
    struct stat st;

    while (1) {

        // Accept an incoming connection

        clntlen = sizeof(clntaddr); // initialize the in-out parameter

        if ((clntsock = accept(servsock,
                        (struct sockaddr *) &clntaddr, &clntlen)) < 0)
            die("accept failed");

        // accept() returned a connected socket (also called client socket)
        // and filled in the client's address into clntaddr

        fprintf(stderr, "client ip: %s\n", inet_ntoa(clntaddr.sin_addr));
        sprintf(filename, "%s.%u", filebase, filesuffix++);
        fprintf(stderr, "file name: %s\n", filename);

        if ((fp = fopen(filename, "wb")) == NULL)
            die(filename);

        // First, receive file size

        r = recv(clntsock, &size_net, sizeof(size_net), MSG_WAITALL);
        if (r != sizeof(size_net)) {
            if (r < 0)
                die("recv failed");
            else if (r == 0)
                die("connection closed prematurely");
            else
                die("didn't receive uint32");
        }
        size = ntohl(size_net); // convert it to host byte order
        fprintf(stderr, "file size received: %u\n", size);

        // Second, receive the file content

        remaining = size; 
        while (remaining > 0) {
            limit = remaining > sizeof(buf) ? sizeof(buf) : remaining;
            r = recv(clntsock, buf, limit, 0);
            if (r < 0)
                die("recv failed");
            else if (r == 0)
                die("connection closed prematurely");
            else {
                remaining -= r;
                if (fwrite(buf, 1, r, fp) != r) 
                    die("fwrite failed");
            }
        }
        assert(remaining == 0);
        fclose(fp);

        // Third, send the file size back as acknowledgement

        stat(filename, &st);
        size = st.st_size;
        fprintf(stderr, "file size on disk:  %u\n\n", size);
        size_net = htonl(size);

        if (send(clntsock, &size_net, sizeof(size_net), 0) 
                != sizeof(size_net))
            die("send size failed");

        // Finally, close the client connection and go back to accept()

        close(clntsock);
    }
}
