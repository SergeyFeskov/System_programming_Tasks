//
// Created by serus on 13.04.22.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>


enum VALIDATE_INPUT_ERRS {
    VIE_NOTINT = 1,
    VIE_OVERFLOW,
    VIE_NOTINRANGE
};

void handle_file_error(char* filename, char* filetype);
void output_script_help();
int validate_int_param(char* str, int* result_ptr, int left_range_border, int right_range_border);
bool file_exists(char* filename);
void close_dir(DIR* dir);
void create_copy_proc(char* src_filename, char* dest_filename);
long copy_file(char* src_filename, char* dest_filename);
void sync_dirs(char* dir1_path, char* dir2_path);
void copy_dir(char* dir_from_path, char* dir_to_path);
void close_file(FILE* file);
long get_size_by_name(char* filename);
long get_size(FILE* file);

int N;
int curr_N;

int main(int argc, char* argv[]) {
    /*--------------------------------------------*/
    /*  VALIDATION OF PARAMETERS                  */
    /*--------------------------------------------*/

    if (argc != 4) {
        fprintf(stderr, "ERROR: script expects 3 arguments, but %d were passed\n", argc - 1);
        output_script_help();
        return -2;
    }

    DIR *dir1 = opendir(argv[1]);
    if (dir1 == NULL) {
        handle_file_error(argv[1], "directory");
        output_script_help();
        return -2;
    }
    close_dir(dir1);

    DIR *dir2 = opendir(argv[2]);
    if (dir2 == NULL) {
        handle_file_error(argv[2], "directory");
        output_script_help();
        return -2;
    }
    close_dir(dir2);

    // validate N
    switch (validate_int_param(argv[3], &N, 1, INT_MAX)) {
        case VIE_NOTINT:
            fprintf(stderr, "ERROR: passed N isn't integer\n");
            output_script_help();
            return -2;

        case VIE_OVERFLOW:
            fprintf(stderr, "ERROR: passed N causes overflow\n");
            return -2;

        case VIE_NOTINRANGE:
            fprintf(stderr, "ERROR: passed N is not in range from 1 to INT_MAX\n");
            output_script_help();
            return -2;
    }

    /*--------------------------------------------*/
    /*  SCRIPT MAIN PART                          */
    /*--------------------------------------------*/

    if (strcmp(argv[1], argv[2]) == 0) {
        printf("Passed dir paths are same. No copying needed.");
        return 0;
    }

    curr_N = 0;
    sync_dirs(argv[1], argv[2]);

    while (curr_N != 0) {
        int p_stat;
        int child_pid = wait(&p_stat);
        if (child_pid == -1) {
            perror("ERROR(in waitpid func)");
            return -1;
        }
        printf("\nProcess %d finished with exit code %d\n", child_pid, p_stat);
        curr_N--;
    }

    printf("\nDIRECTORIES SYNCHRONIZED!\n");
}

long copy_file(char* src_filename, char* dest_filename) {
    FILE* src_file = fopen(src_filename, "r");
    if (src_file == NULL) {
        handle_file_error(src_filename, "file");
        exit(-1);
    }

    FILE* dest_file = fopen(dest_filename, "w");
    if (dest_file == NULL) {
        handle_file_error(dest_filename, "file");
        exit(-1);
    }

    int src_char = fgetc(src_file);
    while (src_char != EOF) {
        if (fputc(src_char, dest_file) == EOF) {
            handle_file_error(dest_filename, "file");
            close_file(src_file);
            close_file(dest_file);
            exit(-1);
        }

        src_char = fgetc(src_file);
    }

    if (ferror(src_file) != 0) {
        handle_file_error(src_filename, "file");
        close_file(src_file);
        close_file(dest_file);
        exit(-1);
    }

    struct stat src_stat;
    if (stat(src_filename, &src_stat) == -1) {
        handle_file_error(src_filename, "file");
        close_file(src_file);
        close_file(dest_file);
        exit(-1);
    }

    if (chmod(dest_filename, src_stat.st_mode) == -1) {
        handle_file_error(dest_filename, "file");
        close_file(src_file);
        close_file(dest_file);
        exit(-1);
    }

    close_file(src_file);
    close_file(dest_file);

    return get_size_by_name(dest_filename);
}

long get_size_by_name(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        handle_file_error(filename, "file");
        exit(-1);
    }
    long size = get_size(file);
    close_file(file);
    return size;
}

long get_size(FILE* file) {
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return size;
}

void close_file(FILE* file) {
    if (fclose(file) == EOF) {
        perror("ERROR(in fclose)");
        exit(1);
    }
}

void copy_dir(char* dir_from_path, char* dir_to_path) {
    DIR *dir_from = opendir(dir_from_path);
    if (dir_from == NULL) {
        handle_file_error(dir_from_path, "directory");
        exit(-1);
    }

    if (mkdir(dir_to_path, 0777) == -1) {
        handle_file_error(dir_to_path, "directory");
        exit(-1);
    }

    char* dir_from_filename = calloc(strlen(dir_from_path) + 257, sizeof(char));
    strcpy(dir_from_filename, dir_from_path);
    if (dir_from_path[strlen(dir_from_path) - 1] != '/') {
        dir_from_filename[strlen(dir_from_path)] = '/';
        dir_from_filename[strlen(dir_from_path) + 1] = '\0';
    }
    int dir_from_path_len = strlen(dir_from_filename);

    char* dir_to_filename = calloc(strlen(dir_to_path) + 257, sizeof(char));
    strcpy(dir_to_filename, dir_to_path);
    if (dir_to_path[strlen(dir_to_path) - 1] != '/') {
        dir_to_filename[strlen(dir_to_path)] = '/';
        dir_to_filename[strlen(dir_to_path) + 1] = '\0';
    }
    int dir_to_path_len = strlen(dir_to_filename);

    errno = 0;
    struct dirent* dir_from_file_info = readdir(dir_from);
    while (dir_from_file_info != NULL) {
        switch (dir_from_file_info->d_type) {
            case DT_REG:
                strcpy(dir_from_filename + dir_from_path_len, dir_from_file_info->d_name);
                strcpy(dir_to_filename + dir_to_path_len, dir_from_file_info->d_name);

                create_copy_proc(dir_from_filename, dir_to_filename);
                break;

            case DT_DIR:
                if (strcmp(dir_from_file_info->d_name, ".") == 0 || strcmp(dir_from_file_info->d_name, "..") == 0) {
                    break;
                }

                strcpy(dir_from_filename + dir_from_path_len, dir_from_file_info->d_name);
                strcpy(dir_to_filename + dir_to_path_len, dir_from_file_info->d_name);

                copy_dir(dir_from_filename, dir_to_filename);
                break;

            default:
                printf("File %s isn't regular or directory, so it is passed.", dir_from_file_info->d_name);
        }

        errno = 0;
        dir_from_file_info = readdir(dir_from);
    }
    free(dir_from_filename);
    free(dir_to_filename);

    if (errno != 0) {
        handle_file_error(dir_from_path, "directory");
        closedir(dir_from);
        exit(-1);
    }
    closedir(dir_from);
}

void create_copy_proc(char* src_filename, char* dest_filename) {
    if (curr_N == N) {
        int p_stat;
        int child_pid = wait(&p_stat);
        if (child_pid == -1) {
            perror("ERROR(in waitpid func)");
            exit(-1);
        }
        printf("\nProcess %d finished with exit code %d\n", child_pid, p_stat);
        curr_N--;
    }

    int child_p = fork();
    switch (child_p) {
        case -1:
            perror("ERROR(in fork func)");
            exit(-1);
        case 0:
            printf("\nCopying process is started.\n"
                   "PID: %d\n"
                   "SRC_FILE: %s\n"
                   "DEST_FILE: %s\n", getpid(), src_filename, dest_filename);
            long copied_bytes = copy_file(src_filename, dest_filename);
            printf("\nCopying process ended.\n"
                   "PID: %d\n"
                   "SRC_FILE: %s\n"
                   "DEST_FILE: %s\n"
                   "COPIED_BYTES_NUM: %ld\n", getpid(), src_filename, dest_filename, copied_bytes);
            exit(0);
        default:
            curr_N++;
    }
}

void sync_dirs(char* dir1_path, char* dir2_path) {
    DIR *dir1 = opendir(dir1_path);
    if (dir1 == NULL) {
        handle_file_error(dir1_path, "directory");
        exit(-1);
    }

    DIR *dir2 = opendir(dir2_path);
    if (dir2 == NULL) {
        handle_file_error(dir2_path, "directory");
        exit(-1);
    }

    char* dir1_filename = calloc(strlen(dir1_path)+257, sizeof(char));
    strcpy(dir1_filename, dir1_path);
    if (dir1_path[strlen(dir1_path) - 1] != '/') {
        dir1_filename[strlen(dir1_path)] = '/';
        dir1_filename[strlen(dir1_path) + 1] = '\0';
    }
    int dir1_path_len = strlen(dir1_filename);

    char* dir2_filename = calloc(strlen(dir2_path)+257, sizeof(char));
    strcpy(dir2_filename, dir2_path);
    if (dir2_path[strlen(dir2_path) - 1] != '/') {
        dir2_filename[strlen(dir2_path)] = '/';
        dir2_filename[strlen(dir2_path) + 1] = '\0';
    }
    int dir2_path_len = strlen(dir2_filename);

    errno = 0;
    struct dirent* dir1_file_info = readdir(dir1);
    while (dir1_file_info != NULL) {
        switch (dir1_file_info->d_type) {
            case DT_REG:
                strcpy(dir1_filename + dir1_path_len, dir1_file_info->d_name);
                strcpy(dir2_filename + dir2_path_len, dir1_file_info->d_name);
                if (!file_exists(dir2_filename)) {
                    create_copy_proc(dir1_filename, dir2_filename);
                }
                break;

            case DT_DIR:
                if (strcmp(dir1_file_info->d_name, ".") == 0 || strcmp(dir1_file_info->d_name, "..") == 0) {
                    break;
                }

                strcpy(dir1_filename + dir1_path_len, dir1_file_info->d_name);
                strcpy(dir2_filename + dir2_path_len, dir1_file_info->d_name);

                errno = 0;
                DIR *tmp_dir = opendir(dir2_filename);
                switch (errno) {
                    case ENOENT:
                        copy_dir(dir1_filename, dir2_filename);
                        break;
                    case ENOTDIR:
                        handle_file_error(dir2_filename, "directory");
                        printf("\n%s isn't directory. You should rename this file, "
                               "if you want to copy dir with such name from %s to %s.\n",
                               dir2_filename, dir1_path, dir2_path);
                        break;
                    default:
                        if (errno == 0) {
                            close_dir(tmp_dir);
                        }
                        sync_dirs(dir1_filename, dir2_filename);
                }
                break;

            default:
                printf("File %s isn't regular or directory, so it is passed.", dir1_file_info->d_name);
        }

        errno = 0;
        dir1_file_info = readdir(dir1);
    }
    free(dir1_filename);
    free(dir2_filename);

    if (errno != 0) {
        handle_file_error(dir1_path, "directory");
        closedir(dir1);
        closedir(dir2);
        exit(-1);
    }
    closedir(dir1);
    closedir(dir2);
}

void close_dir(DIR* dir) {
    if (closedir(dir) == -1) {
        perror("ERROR(in dirclose)");
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
    fprintf(stdout, "SYNOPSIS:\n\tmain_task_var2 DIR1 DIR2 N\n");
    fprintf(stdout,
            "DESCRIPTION:\n"
            "\tScript copies non-existent in DIR2 files from DIR1 to DIR2 (comparing of files by names).\n"
            "\tCopying of each file executes in new process. Max number of active copying processes is defined by N.\n"
            "\n"
            "\tDIR1 / DIR2 - path of directory, from / to which script copies files\n"
            "\tN - max number of active copying processes (from 1 to INT_MAX)\n");
}

void handle_file_error(char* filename, char* filetype) {
    char* error_header = calloc(strlen(filename) + 100, sizeof(char));
    sprintf(error_header, "ERROR(with %s %s)", filename, filetype);
    perror(error_header);
    free(error_header);
}

bool file_exists(char* filename) {
    return access(filename, F_OK) == 0;
}
