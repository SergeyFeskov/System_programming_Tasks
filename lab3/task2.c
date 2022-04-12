//
// Created by serus on 11.04.22.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>

void get_time_str(char* time_str) {
    struct timespec ts;
    errno = 0;
    if (timespec_get(&ts, TIME_UTC) == 0) {
        if (errno != 0) {
            perror("ERROR(in timespec_get func)");
        } else {
            fprintf(stderr, "ERROR(in timespec_get func): Undefined error.");
        }
        exit(-1);
    }
    char buff[20];
    if (strftime(buff, sizeof buff, "%T", gmtime(&ts.tv_sec)) == 0) {
        if (errno != 0) {
            perror("ERROR(in strftime func)");
        } else {
            fprintf(stderr, "ERROR(in strftime func): Undefined error.");
        }
        exit(-1);
    }
    if (sprintf(time_str, "%s:%09ld", buff, ts.tv_nsec) < 0) {
        if (errno != 0) {
            perror("ERROR(in strftime func)");
        } else {
            fprintf(stderr, "ERROR(in strftime func): Undefined error.");
        }
        exit(-1);
    }
}

void task_func() {
    char time_str[100];
    get_time_str(time_str);
    printf("\nPID: %d\n"
           "PPID: %d\n"
           "Time: %s\n",
           getpid(), getppid(), time_str);
}

int main() {
    pid_t child_p1;
    pid_t child_p2;
    child_p1 = fork();
    switch (child_p1) {
        case -1:
            perror("ERROR(in fork func)");
            exit(-1);
        case 0:
            task_func();
            break;
        default:
            child_p2 = fork();
            switch (child_p2) {
                case -1:
                    perror("ERROR(in fork func)");
                    exit(-1);
                case 0:
                    task_func();
                    break;
                default:
                    task_func();

                    int p1_stat;
                    if (waitpid(child_p1, &p1_stat, 0) == -1) {
                        perror("ERROR(in waitpid func)");
                        exit(-1);
                    }
                    printf("\nChild process 1 finished with exit code %d\n", p1_stat);

                    int p2_stat;
                    if (waitpid(child_p2, &p2_stat, 0) == -1) {
                        perror("ERROR(in waitpid func)");
                        exit(-1);
                    }
                    printf("\nChild process 2 finished with exit code %d\n", p2_stat);
                    break;
            }
            break;
    }
}
