//
// Created by serus on 31.03.22.
//
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

void output_script_help();
void handle_file_error(char* filename, FILE* file);
bool is_integer(char* str);

int main(int argc, char* argv[]) {
    /*--------------------------------------------*/
    /*  VALIDATION OF PARAMETERS                  */
    /*--------------------------------------------*/

    if (argc != 3) {
        fprintf(stderr, "ERROR: script expects 2 arguments, but %d were passed\n", argc - 1);
        output_script_help();
        return 1;
    }

    char* filename = argv[1];
    FILE* file = fopen(filename, "r");

    // check if fopen executed with errors
    // if so, outputs error in stderr
    if (file == NULL) {
        handle_file_error(filename, file);
        output_script_help();
        return 1;
    }

    if (!is_integer(argv[2])) {
        fprintf(stderr, "ERROR: second argument isn't integer\n");
        output_script_help();
        return 1;
    }

    int strings_num = atoi(argv[2]);

    if (strings_num < 0) {
        fprintf(stderr, "ERROR: second argument is negative\n");
        output_script_help();
        return 1;
    }


    /*--------------------------------------------*/
    /*  SCRIPT MAIN PART                          */
    /*--------------------------------------------*/

    while (true) {
        int strings_left = strings_num;

        // loop for each pack of STRINGS_NUM strings
        // if STRINGS_NUM == 0, loop will be endless
        do {
            int symbol;
            do {
                symbol = fgetc(file);

                // check if error occurred during fgetc
                if (ferror(file) != 0) {
                    handle_file_error(filename, file);
                    return 1;
                }

                // check if EOF
                // if so, terminate script successfully
                if (symbol == EOF) {
                    fclose(file);
                    printf("\nFile %s was successfully printed.", filename);
                    return 0;
                }

                // output char from file and check for errors
                if (putchar(symbol) == EOF) {
                    handle_file_error(filename, file);
                    return 1;
                }
            } while (symbol != '\n');  // loop for each string in file
        } while ((--strings_left) != 0);

        // waiting for print another STRINGS_NUM strings
        getchar();
    }
}

bool is_integer(char* str) {
    int str_len = strlen(str);
    for (int i = 0; i < str_len; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }
    return true;
}

void output_script_help() {
    fprintf(stdout, "SYNOPSIS:\n\ttask3 FILE STRINGS_NUM\n");
    fprintf(stdout,
            "DESCRIPTION:\n"
            "\tFILE - path of text file, whose content script outputs in stdout\n"
            "\tSTRINGS_NUM - number of strings, which script outputs at once\n");
}

void handle_file_error(char* filename, FILE* file) {
    char* error_header = calloc(strlen(filename) + 100, sizeof(char));
    sprintf(error_header, "ERROR(with %s file)", filename);
    perror(error_header);
    fclose(file);
}
