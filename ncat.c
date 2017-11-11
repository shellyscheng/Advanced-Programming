/*
 * ncat <file_name>
 *
 *  - read a file line by line
 *    printing them out which line numbers
 *
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf(stderr, "%s\n", "usage: netcat <file_name>");
        exit(1);
    }
    char *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    if(fp == NULL){
        perror(filename);
        exit(1);
    }

    char buf[100];
    int lineno = 1;
    while(fgets(buf, sizeof(buf), fp) != NULL) {
        printf("%4d", lineno++);
        if(fputs(buf, stdout) == EOF) {
            perror("can't write to stdout");
            exit(1);
        }
    }

    if (ferror(fp)) {
        perror(filename);
        exit(1);
    }
    fclose(fp);
    return 0;
}
