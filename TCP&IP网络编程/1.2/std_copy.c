#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
    FILE *src_fp, *dst_fp;
    char buf[BUF_SIZE];
    size_t num_read;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        exit(1);
    }

    src_fp = fopen(argv[1],"r");
    if (src_fp == NULL)
        error_handling("fopen() error for source file!");
    
    dst_fp = fopen(argv[2], "w");
    if (dst_fp == NULL)
        error_handling("fopen() error for destination file!");
    
    while ((num_read = fread(buf,1,BUF_SIZE,src_fp)) > 0) {
        if (fwrite(buf,1,num_read,dst_fp) != num_read)
            error_handling("fwrite() error!");
    }

    if (ferror(src_fp))
        error_handling("fread() error!");
    
    fclose(src_fp);
    fclose(dst_fp);

    printf("File copied successfully.\n");
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}