#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "repl.h"
#include "../vm/vm.h"

#define MAX_INPUT 1024

error_t repl() {
    char input[MAX_INPUT] = "";
    error_t err = ERR_NO_ERROR;

    char *program = malloc(2);
    program[0] = 0;

    while (1) {
        printf("> ");

        if (!fgets(input, sizeof(input), stdin)) {
            printf("Bye!\n");
            goto done;
        }
    }

    done:
    return err;
}

void run(const char *filename) {

}

int main(int argc, const char **argv) {
    if (argc == 1) {
        exit(repl());
    } else if (argc == 2) {
        run(argv[1]);
    } else {
        fprintf(stderr, "Usage: er [file]\n");
        exit(1);
    }

    return 0;
}