/*
 * tcp-sender.c
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
    if (argc != 4) {
        fprintf(stderr, "usage: %s <server-ip> <server-port> <filename>\n",
                argv[0]);
        exit(1);
    }

    const char *ip = argv[1];
    unsigned short port = atoi(argv[2]);
    const char *filename = argv[3];

    // Open the file to send; if "-" is passed, read from stdin

    FILE *fp;
    if (strcmp(filename, "-") == 0) {
        fp = stdin;
    } else {
        if ((fp = fopen(filename, "rb")) == NULL)
            die(filename);
    }

    // Create a socket for TCP connection

    int sock; // socket descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("socket failed");

    // Construct a server address structure

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr)); // must zero out the structure
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port        = htons(port); // must be in network byte order

    // Establish a TCP connection to the server

    if (connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        die("connect failed");

    // First, send file size as 4-byte unsigned int in network byte order

    struct stat st;
    stat(filename, &st);
    uint32_t size = st.st_size;
    fprintf(stderr, "file size:  %u\n", size);
    uint32_t size_net = htonl(size);

    /*
     * send(int socket, const void *buffer, size_t length, int flags)
     *
     *   - normally, send() blocks until it sends all bytes requested
     *   - returns num bytes sent or -1 for error
     *   - send(sock,buf,len,0) is equivalent to write(sock,buf,len)
     */
    if (send(sock, &size_net, sizeof(size_net), 0) != sizeof(size_net))
        die("send size failed");

    // Second, send the file content

    char buf[4096];
    unsigned int n;
    unsigned int total = 0;

    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        if (send(sock, buf, n, 0) != n)
            die("send content failed");
        else
            total += n;
    }
    if (ferror(fp)) {
        // fread() returns 0 on EOF or error, so we check if error occurred
	die("fread failed");
    }
    fclose(fp);
    fprintf(stderr, "bytes sent: %u\n", total);

    // Third, receive file size back from the server as acknowledgement

    uint32_t ack, ack_net;

    /*
     * recv(int socket, void *buffer, size_t length, int flags)
     *
     *   - normally, recv() blocks until it has received at least 1 byte
     *   - returns num bytes received, 0 if connection closed, -1 if error
     *   - recv(sock,buf,len,0) is equivalent to read(sock,buf,len)
     *   
     *   - With TCP sockets, we can receive less data than we requested;
     *     MSG_WAITALL flag changes this behavior -- it requests that the 
     *     operation block until the full request is satisfied.
     */
    int r = recv(sock, &ack_net, sizeof(ack_net), MSG_WAITALL);
    if (r != sizeof(ack_net)) {
        if (r < 0)
            die("recv failed");
        else if (r == 0)
            die("connection closed prematurely");
        else
            die("didn't receive uint32");
    }
    ack = ntohl(ack_net); // convert it back to host byte order
    if (ack != size)
        die("ack!=size");

    // recv() will return 0 when the server closes the TCP connection
    char x;
    r = recv(sock, &x, 1, 0);
    assert(r == 0);

    // Clean-up

    close(sock);
    return 0;
}
