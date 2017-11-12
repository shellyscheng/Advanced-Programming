#include <stdio.h>
#include <assert.h>
#include <arpa/inet.h>

#define HOST_FILE "endian.host"
#define NET_FILE "endian.net"

int main() {
    FILE *f;
    unsigned int num_host;
    unsigned int num_net;

    printf("Enter a hexadecimal integer: ");
    scanf("%x", &num_host);
    printf("The hex number 0x%.8x is %d in decimal\n", num_host, num_host);

    f = fopen(HOST_FILE, "wb");
    assert(f);
    fwrite(&num_host, sizeof(num_host), 1, f);
    printf("Wrote num_host to %s\n", HOST_FILE);
    fclose(f);

    num_net = htonl(num_host);

    f = fopen(NET_FILE, "wb");
    assert(f);
    fwrite(&num_net, sizeof(num_net), 1, f);
    printf("Wrote num_net to %s\n", NET_FILE);
    fclose(f);

    printf("Run \"hd <filename>\" to see the content of a file\n");
}
