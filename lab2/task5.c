//
// Created by serus on 4.04.22.
//
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <unistd.h>
#include "errno.h"
#include "dirent.h"

void output_script_help();
void handle_file_error(char* filename, char* filetype);
int output_dir_content(char* dir_path);


int main(int argc, char* argv[]) {
    if (argc != 1) {
        fprintf(stderr, "ERROR: script doesn't expect any arguments, but %d were passed\n", argc - 1);
        output_script_help();
        return 1;
    }

    int buff_size = 100;
    char* cwd_path = calloc(buff_size, sizeof(char));
    while (getcwd(cwd_path, buff_size) == NULL) {
        if (errno != ERANGE) {
            perror("ERROR(in getcwd function)");
            return 1;
        }
        buff_size += 100;
        free(cwd_path);
        cwd_path = calloc(buff_size, sizeof(char));
    }

    if (output_dir_content(cwd_path) == 1) {
        printf("\nOUTPUT_RESULT: Output of %s failed.\n", cwd_path);
    } else {
        printf("\nOUTPUT_RESULT: Output of %s is successful.\n", cwd_path);
    }

    if (output_dir_content("/") == 1) {
        printf("\nOUTPUT_RESULT: Output of root dir ('/') failed.\n");
    } else {
        printf("\nOUTPUT_RESULT: Output of root dir ('/') is successful.\n");
    }

    return 0;
}

int output_dir_content(char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (dir == NULL) {
        handle_file_error(dir_path, "directory");
        return 1;
    }

    printf("\nCONTENT OF %s:\n", dir_path);

    errno = 0;
    struct dirent* curr_dir_el = readdir(dir);
    while (curr_dir_el != NULL) {
        char file_type = 'u';
        switch (curr_dir_el->d_type) {
            case DT_REG:
                file_type = 'f';
                break;

            case DT_DIR:
                file_type = 'd';
                break;
        }
        printf("%c\t%s\n", file_type, curr_dir_el->d_name);
        curr_dir_el = readdir(dir);
    }

    int ret_code = 0;
    if (errno != 0) {
        handle_file_error(dir_path, "directories element");
        ret_code = 1;
    }

    if (closedir(dir) == -1) {
        perror("ERROR(in closedir)");
        ret_code = 1;
    }
    return ret_code;
}

void output_script_help() {
    fprintf(stdout, "SYNOPSIS:\n\ttask5\n");
    fprintf(stdout,
            "DESCRIPTION:\n"
            "\t Script prints in stdout content of current working and root directories.\n");
}

void handle_file_error(char* filename, char* filetype) {
    char* error_header = calloc(strlen(filename) + 100, sizeof(char));
    sprintf(error_header, "ERROR(with %s %s)", filename, filetype);
    perror(error_header);
}
