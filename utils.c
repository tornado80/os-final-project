#include<stdlib.h>
#include<stdio.h>

void error(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}