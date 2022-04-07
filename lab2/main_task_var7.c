//
// Created by serus on 5.04.22.
//
#include "stdio.h"
#include "sys/stat.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include <unistd.h>
#include <limits.h>
#include "errno.h"
#include "dirent.h"

enum VALIDATE_INPUT_ERRS {
    VIE_NOTINT = 1,
    VIE_OVERFLOW,
    VIE_NOTINRANGE
};

enum COMPARE_FILES_ERRS {
    CFE_F1ERR = 1,
    CFE_F2ERR
};

/*
#define VIE_NOTINT 1
#define VIE_OVERFLOW 2
#define VIE_NOTINRANGE 3
*/


void output_script_help();
void handle_file_error(char* filename, char* filetype);
bool is_integer(char* str);
bool close_files(FILE** files[], int files_am);
bool file_exists(char* filename);
bool is_integer(char* str);
int validate_int_param(char* str, int* result_ptr, int left_range_border, int right_range_border);

int main(int argc, char* argv[]) {
    /*--------------------------------------------*/
    /*  VALIDATION OF PARAMETERS                  */
    /*--------------------------------------------*/

    if (argc != 4) {
        fprintf(stderr, "ERROR: script expects 3 arguments, but %d were passed\n", argc - 1);
        output_script_help();
        return 1;
    }

    // validate N1
    int N1;
    switch (validate_int_param(argv[2], &N1, 1, INT_MAX)) {
        case VIE_NOTINT:
            fprintf(stderr, "ERROR: passed N1 isn't integer\n");
            output_script_help();
            return 1;
            break;

        case VIE_OVERFLOW:
            fprintf(stderr, "ERROR: passed N1 causes overflow\n");
            return 1;
            break;

        case VIE_NOTINRANGE:
            fprintf(stderr, "ERROR: passed N1 isn't positive number\n");
            output_script_help();
            return 1;
            break;
    }

    // validate N2
    int N2;
    switch (validate_int_param(argv[3], &N2, N1 + 1, INT_MAX)) {
        case VIE_NOTINT:
            fprintf(stderr, "ERROR: passed N2 isn't integer\n");
            output_script_help();
            return 1;
            break;

        case VIE_OVERFLOW:
            fprintf(stderr, "ERROR: passed N2 causes overflow\n");
            return 1;
            break;

        case VIE_NOTINRANGE:
            fprintf(stderr, "ERROR: passed N2 lesser than N1\n");
            output_script_help();
            return 1;
            break;
    }

    DIR* main_dir = opendir(argv[1]);
    if (main_dir == NULL) {
        handle_file_error(argv[1], "directory");
        output_script_help();
        return 1;
    }

    /*--------------------------------------------*/
    /*  SCRIPT MAIN PART                          */
    /*--------------------------------------------*/


}

long get_size(FILE* file) {
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return size;
}

int compare_files_content(char* file1name,char* file2name, bool* is_identical) {
    FILE* file1 = fopen(file1name, "r");
    if (file1 == NULL) {
        return CFE_F1ERR;
    }

    FILE* file2 = fopen(file2name, "r");
    if (file2 == NULL) {
        return CFE_F2ERR;
    }

    if (get_size(file1) != get_size(file2)) {
        *is_identical = false;
        fclose(file1);
        fclose(file2);
        return 0;
    }

    int file1_char = fgetc(file1);
    int file2_char = fgetc(file2);
    while (file1_char != EOF && file2_char != EOF) {
        if (file1_char != file2_char) {
            *is_identical = false;
            fclose(file1);
            fclose(file2);
            return 0;
        }

        file1_char = fgetc(file1);
        file2_char = fgetc(file2);
    }

    if (ferror(file1) != 0) {
        fclose(file1);
        fclose(file2);
        return CFE_F1ERR;
    }

    if (ferror(file2) != 0) {
        fclose(file1);
        fclose(file2);
        return CFE_F2ERR;
    }

    *is_identical = false;
    fclose(file1);
    fclose(file2);
    return 0;
}

int validate_int_param(char* str, int* result_ptr, int left_range_border, int right_range_border) {
    if (!is_integer(str)) {
        return VIE_NOTINT;
    }

    errno = 0;
    *result_ptr = strtol(str, NULL, 10);
    if (errno == ERANGE) {
        return VIE_OVERFLOW;
    }

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
    fprintf(stdout, "SYNOPSIS:\n\tmain_task DIR N1 N2\n");
    fprintf(stdout,
            "DESCRIPTION:\n"
            "\tScript outputs groups of files (with sizes in range from N1 to N2 bytes) with identical content from directory DIR.\n"
            "\n"
            "\tDIR - path of directory, in which script finds identical files\n"
            "\tN1, N2 - low and high borders of file's size's range (in bytes)\n");
}

void handle_file_error(char* filename, char* filetype) {
    char* error_header = calloc(strlen(filename) + 100, sizeof(char));
    sprintf(error_header, "ERROR(with %s %s)", filename, filetype);
    perror(error_header);
}