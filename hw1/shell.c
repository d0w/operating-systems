
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 

#include "parser.h"

#define BUFSIZE 512
#define WORDSIZE 32

struct process {
    char *name;
    char **args;
    int pid;
    int status;
};

struct command {
    struct process *processes;
    int num_processes;
    int status;
};


// read command from stdin or file
// FILE *stream
int readCommand(char *buffer, int size) {
    printf("w;elkfj");

    // checks for EOF
    char *line = fgets(buffer, size, stdin);
    if (line == NULL) {
        return 5;
    }
    // char **cmds = parseCommand(buffer);

    // holds current command
    // char *curr = (char *)malloc(sizeof(char) * BUFSIZE);

    // run command

    char **curr = parseLine(buffer);

    int i = 0;
    while(curr[i] != NULL) {
        printf("%s\n", curr[i]);
        i++;
    }
    // printf("%s\n", curr);


    // print debugging
    
    // do command logic here

    free(curr);
    return 0;
}

// main shell
int shell() {
    char buf[BUFSIZE];
    int status;
    while (1) {
        printf("my_shell$");
        status = readCommand(buf, BUFSIZE);
        if (status == 5) {
            printf("\n");
            break;
        }
        if (fork() != 0) {
            // parent process
            // waitpid(-1, &status, 0);
        } else {
            // child code
            // execve(buf, NULL, 0);
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    int output = shell();
    return output || 0;
}


