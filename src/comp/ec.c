#include <stdio.h>
#include <stdlib.h>
#include "sys/errno.h"

#include "comp.h"

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fputs("Usage: ec <filename.e>\n", stderr);
        exit(1);
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fputs("Failed to open input file.\n", stderr);
        exit(errno);
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        fputs("Error determining length of input file.\n", stderr);
        exit(errno);
    }

    size_t len = ftell(fp);
    if (len == -1) {
        fputs("File error.\n", stderr);
        exit(errno);
    }
    rewind(fp);

    char* buf = (char*) malloc(len + 1);
    if (buf == NULL) {
        fputs("Could not allocate memory for input.\n", stderr);
        exit(errno);
    }
    size_t bytes_read = fread(buf, sizeof(char), len, fp);
    buf[bytes_read] = '\0';

    fclose(fp);

    // Buffer must not be modified or deallocated until compilation is complete.
    cg_t cg;
    cg_init(&cg);
    exit(codegen(buf, &cg));
}
