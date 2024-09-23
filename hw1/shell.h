#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/types.h>

#define BUFSIZE 512
#define WORDSIZE 32
#define FALSE 0
#define TRUE 1

struct Pipeline {
    struct Command *commands;
    // int count;
    int background;
};


// struct Process {
//     char *name;
//     char **args;
//     int pid;
//     int status;
// };

struct Command {
    // struct Process *process;
    struct Command *next;
    char **args;

    // these handle input and output redirection
    char *inPath;
    char *outPath;
};

int readCommand(char *buffer, int size, struct Pipeline *pipe, int usePrompt);
int shell(char *arg);
struct Command *initCommand();
struct Process *initProcess();
struct Pipeline *initPipeline();
int cleanPipeline(struct Pipeline *pipe);
int cleanCommand(struct Command *command);


#endif