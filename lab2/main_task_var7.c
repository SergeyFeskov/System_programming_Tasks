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

typedef struct FileNode {
    char* filename;
    struct FileNode* next;
} TFileNode;

typedef struct GroupNode {
    TFileNode* same_files;
    struct GroupNode* next;
} TGroupNode;



void output_script_help();
void handle_file_error(char* filename, char* filetype);
bool close_files(FILE** files[], int files_am);
bool file_exists(char* filename);
int validate_int_param(char* str, int* result_ptr, int left_range_border, int right_range_border);
int compare_files_content(char* file1name,char* file2name, bool* is_identical);
long get_size(FILE* file);
void close_file(FILE* file);
int parse_dir(DIR* dir, char* dir_path);
long get_size_by_name(char* filename);
void close_dir(DIR* dir);

TGroupNode* groups_list = NULL;
int N1;
int N2;

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
    switch (validate_int_param(argv[2], &N1, 1, INT_MAX)) {
        case VIE_NOTINT:
            fprintf(stderr, "ERROR: passed N1 isn't integer\n");
            output_script_help();
            return 1;

        case VIE_OVERFLOW:
            fprintf(stderr, "ERROR: passed N1 causes overflow\n");
            return 1;

        case VIE_NOTINRANGE:
            fprintf(stderr, "ERROR: passed N1 isn't positive number\n");
            output_script_help();
            return 1;
    }

    // validate N2
    switch (validate_int_param(argv[3], &N2, N1 + 1, INT_MAX)) {
        case VIE_NOTINT:
            fprintf(stderr, "ERROR: passed N2 isn't integer\n");
            output_script_help();
            return 1;

        case VIE_OVERFLOW:
            fprintf(stderr, "ERROR: passed N2 causes overflow\n");
            return 1;

        case VIE_NOTINRANGE:
            fprintf(stderr, "ERROR: passed N2 lesser than N1\n");
            output_script_help();
            return 1;
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

    int ret_code = 0;
    if (parse_dir(main_dir, argv[1]) != 0) {
        handle_file_error(argv[1], "directory");
        ret_code = 1;
    }
    close_dir(main_dir);
    printf("\n");
    printf("Parsing of %s directory completed completed.\n\n", argv[1]);

    printf("Output of groups of files with same content.\n");
    TGroupNode* curr_group = groups_list;
    int groups_count = 0;
    while (curr_group != NULL) {
        if (curr_group->same_files->next != NULL) {
            groups_count++;
            printf("Group %d:\n", groups_count);
            TFileNode* curr_file = curr_group->same_files;
            while (curr_file != NULL) {
                printf("\t%s\n", curr_file->filename);
                curr_file = curr_file->next;
            }
        }
        curr_group = curr_group->next;
    }
    if (groups_count == 0) {
        printf("There's no such groups.\n");
    }

    while (groups_list != NULL) {
        curr_group = groups_list;
        groups_list = groups_list->next;
        while (curr_group->same_files != NULL) {
            TFileNode* curr_file = curr_group->same_files;
            curr_group->same_files = curr_group->same_files->next;
            free(curr_file->filename);
            free(curr_file);
        }
        free(curr_group);
    }

    return ret_code;
}

long get_size_by_name(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        handle_file_error(filename, "file");
        return -1;
    }
    long size = get_size(file);
    close_file(file);
    return size;
}

void insert_in_group(char* filename, TGroupNode* group) {
    TFileNode* new_node = malloc(sizeof(TFileNode));
    new_node->filename = calloc(strlen(filename) + 1, sizeof(char));
    strcpy(new_node->filename, filename);
    new_node->next = group->same_files;
    group->same_files = new_node;
}

int insert_file(char* filename) {
    TGroupNode* prev_group = NULL;
    TGroupNode* curr_group = groups_list;
    while (curr_group != NULL) {
        bool is_same;
        int cfc_exit_code = compare_files_content(curr_group->same_files->filename, filename, &is_same);
        if (cfc_exit_code != 0) {
            if (cfc_exit_code == CFE_F1ERR) {
                handle_file_error(curr_group->same_files->filename, "file");
            } else {
                handle_file_error(filename, "file");
            }
            printf("File %s wasn't inserted in list.", filename);
            return 1;
        }
        if (is_same) {
            insert_in_group(filename, curr_group);
            return 0;
        }
        prev_group = curr_group;
        curr_group = curr_group->next;
    }

    TGroupNode* new_group = malloc(sizeof(TGroupNode));
    new_group->same_files = NULL;
    insert_in_group(filename, new_group);
    new_group->next = groups_list;
    groups_list = new_group;
    return 0;
}

void close_dir(DIR* dir) {
    if (closedir(dir) == -1) {
        perror("ERROR(in dirclose)");
        exit(1);
    }
}

int parse_dir(DIR* dir, char* dir_path) {
    errno = 0;
    struct dirent* curr_dir_el = readdir(dir);
    DIR* inner_dir;
    long file_size;

    char* filename = calloc(strlen(dir_path)+257, sizeof(char));
    strcpy(filename, dir_path);

    int shift = 0;
    if (dir_path[strlen(dir_path) - 1] != '/') {
        shift = 1;
        filename[strlen(dir_path)] = '/';
        filename[strlen(dir_path) + 1] = '\0';
    }

    while (curr_dir_el != NULL) {
        switch (curr_dir_el->d_type) {
            case DT_REG:
                strcpy(filename + strlen(dir_path) + shift, curr_dir_el->d_name);
                file_size = get_size_by_name(filename);
                printf("%d", (int) file_size);
                if (file_size >= N1 && file_size <= N2) {
                    insert_file(filename);
                }
                break;

            case DT_DIR:
                if (strcmp(curr_dir_el->d_name, ".") == 0 || strcmp(curr_dir_el->d_name, "..") == 0) {
                    break;
                }

                strcpy(filename + strlen(dir_path) + shift, curr_dir_el->d_name);
                inner_dir = opendir(filename);
                if (inner_dir == NULL) {
                    handle_file_error(filename, "directory");
                } else {
                    if (parse_dir(inner_dir, filename) != 0) {
                        handle_file_error(filename, "directory");
                    }
                    close_dir(inner_dir);
                }
                break;

            default:
                printf("File %s isn't regular, so it is passed.", curr_dir_el->d_name);
                break;
        }

        errno = 0;
        curr_dir_el = readdir(dir);
    }
    free(filename);

    if (errno != 0) {
        return 1;
    }
    return 0;
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

    *is_identical = true;
    fclose(file1);
    fclose(file2);
    return 0;
}

void close_file(FILE* file) {
    if (fclose(file) == EOF) {
        perror("ERROR(in fclose)");
        exit(1);
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