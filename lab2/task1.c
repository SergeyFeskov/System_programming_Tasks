//
// Created by serus on 30.03.22.
//
#include "stdio.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "ERROR: script expects 1 argument, but %d were passed\n", argc - 1);
        fprintf(stdout, "SYNOPSIS:\n\ttask1 MESSAGE\n");
        fprintf(stdout, "DESCRIPTION:\n\tMESSAGE - string, which script outputs in stdout\n");
        return 1;
    }

    fprintf(stdout, "%s\n", argv[1]);
}
