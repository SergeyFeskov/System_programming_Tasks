//
// Created by serus on 4.04.22.
//
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

void output_script_help();
void handle_file_error(char* filename);
bool close_files(FILE** files[], int files_am);
bool file_exists(char* filename);

int main(int argc, char* argv[]) {
    /*--------------------------------------------*/
    /*  VALIDATION OF PARAMETERS                  */
    /*--------------------------------------------*/

    if (argc != 3) {
        fprintf(stderr, "ERROR: script expects 2 arguments, but %d were passed\n", argc - 1);
        output_script_help();
        return 1;
    }

    char *src_name = argv[1];
    FILE* src_file = fopen(src_name, "r");

    // check if fopen executed with errors
    // if so, outputs error in stderr
    if (src_file == NULL) {
        handle_file_error(src_name);
        output_script_help();
        return 1;
    }

    char *dest_name = argv[2];
    if (file_exists(dest_name)) {
        fprintf(stderr, "ERROR(with %s file): "
                        "no file existence expected, but it exists\n", dest_name);
        return 1;
    }

    FILE* dest_file = fopen(dest_name, "w");

    // check if fopen executed with errors
    // if so, outputs error in stderr
    if (dest_file == NULL) {
        handle_file_error(dest_name);
        output_script_help();
        return 1;
    }


    /*--------------------------------------------*/
    /*  SCRIPT MAIN PART                          */
    /*--------------------------------------------*/

    FILE** files[2];
    files[0] = &src_file;
    files[1] = &dest_file;

    int src_char = fgetc(src_file);
    while (src_char != EOF) {
        if (fputc(src_char, dest_file) == EOF) {
            handle_file_error(dest_name);
            close_files(files, 2);
            return 1;
        }

        src_char = fgetc(src_file);
    }

    if (ferror(src_file) != 0) {
        handle_file_error(src_name);
        close_files(files, 2);
        return 1;
    }

    struct stat src_stat;
    if (stat(src_name, &src_stat) == -1) {
        handle_file_error(src_name);
        close_files(files, 2);
        return 1;
    }

    if (chmod(dest_name, src_stat.st_mode) == -1) {
        handle_file_error(src_name);
        close_files(files, 2);
        return 1;
    }

    if (close_files(files, 2)) {
        return 0;
    } else {
        return 1;
    }
}

bool file_exists(char* filename) {
    return access(filename, F_OK) == 0;
}

bool close_files(FILE** files[], int files_am) {
    for (int i = 0; i < files_am; i++) {
        if (*(files[i]) != NULL) {
            if (fclose(*(files[i])) == EOF) {
                perror("ERROR(in fclose)");
                return false;
            }
        }
    }
    return true;
}

void output_script_help() {
    fprintf(stdout, "SYNOPSIS:\n\ttask4 SRC_FILE DEST_FILE\n");
    fprintf(stdout,
            "DESCRIPTION:\n"
            "\tSRC_FILE - path of file, which script copies as DEST_FILE\n"
            "\tDEST_FILE - path of file, to which script copies SRC_FILE\n");
}

void handle_file_error(char* filename) {
    char* error_header = calloc(strlen(filename) + 100, sizeof(char));
    sprintf(error_header, "ERROR(with %s file)", filename);
    perror(error_header);
    free(error_header);
}