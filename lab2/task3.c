//
// Created by serus on 31.03.22.
//
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "errno.h"
#include "limits.h"

enum VALIDATE_INPUT_ERRS {
    VIE_NOTINT = 1,
    VIE_OVERFLOW,
    VIE_NOTINRANGE
};

void output_script_help();
void handle_file_error(char* filename, FILE* file);
int validate_int_param(char* str, int* result_ptr, int left_range_border, int right_range_border);

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

    int strings_num;
    switch (validate_int_param(argv[2], &strings_num, 0, INT_MAX)) {
        case VIE_NOTINT:
            fprintf(stderr, "ERROR: second argument isn't integer\n");
            output_script_help();
            return 1;

        case VIE_OVERFLOW:
            fprintf(stderr, "ERROR: second argument causes overflow\n");
            return 1;

        case VIE_NOTINRANGE:
            fprintf(stderr, "ERROR: second argument is negative\n");
            output_script_help();
            return 1;
    }

    /*--------------------------------------------*/
    /*  SCRIPT MAIN PART                          */
    /*--------------------------------------------*/

    while (true) {
        long strings_left = strings_num;

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

int validate_int_param(char* str, int* result_ptr, int left_range_border, int right_range_border) {
    char* endptr;
    long long_res = strtol(str, &endptr, 10);

    if (*endptr != '\0' || endptr == str) {
        return VIE_NOTINT;
    }

    if (long_res < INT_MIN || long_res > INT_MAX) {
        return VIE_OVERFLOW;
    }
    *result_ptr = (int)long_res;

    if (*result_ptr < left_range_border || *result_ptr > right_range_border) {
        return VIE_NOTINRANGE;
    }

    return 0;
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
