//
// Created by serus on 30.03.22.
//
#include "stdio.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

void output_script_help() {
    fprintf(stdout, "SYNOPSIS:\n\ttask2 FILE\n");
    fprintf(stdout, "DESCRIPTION:\n\tFILE - path of file, in which script writes input\n");
}

void handle_file_error(char* filename, int fd) {
    char* error_header = calloc(strlen(filename) + 100, sizeof(char));
    sprintf(error_header, "ERROR(with %s file)", filename);
    perror(error_header);
    free(error_header);
    close(fd);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "ERROR: script expects 1 argument, but %d were passed\n", argc - 1);
        output_script_help();
        return 1;
    }

    char* filename = argv[1];
    int fd = open(filename, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

    // check if open executed with errors
    // if so, outputs error in stderr
    if (fd == -1) {
        handle_file_error(filename,fd);
        output_script_help();
        return 1;
    }

    printf("Enter message you want to write in %s.\n", filename);
    printf("(to stop input press $)\n");
    int input_char = getchar();
    while (input_char != '$') {
        if (write(fd, &input_char, 1) == -1) {
            handle_file_error(filename,fd);
            return 1;
        }
        input_char = getchar();
        if (input_char == EOF) {
            handle_file_error(filename,fd);
            return 1;
        }
    }

    close(fd);
}
