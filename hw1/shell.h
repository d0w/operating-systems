#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 

struct Process {
    char *name;
    char **args;
    int pid;
    int status;
};

struct Command {
    struct Process *processes;
    struct Command *next;
    char **args;
    int num_processes;
    int status;
};

int readCommand(char *buffer, int size);
int shell();


#endif