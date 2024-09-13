#include "parser.h"

#define BUFSIZE 512
#define WORDSIZE 32




// read command from stdin or file
// FILE *stream
int readCommand(char *buffer, int size) {

    // checks for EOF
    char *line = fgets(buffer, size, stdin);
    if (line == NULL) {
        return 5;
    }
    // char **cmds = parseCommand(buffer);

    // holds current command
    // char *curr = (char *)malloc(sizeof(char) * BUFSIZE);

    // run command

    struct Command *curr = parseLine(buffer);

    if (curr == NULL) {
        printf("ERROR: Invalid command\n");
        return -1;
    }

    while(curr != NULL) {
        char **commandPtr = curr->args;
        while (*commandPtr != NULL) {
            printf("%s ", *commandPtr);
            commandPtr++;
        }
        printf("\n");
        curr = curr->next;
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
    pid_t pid;
    int status;
    while (1) {
        printf("my_shell$");
        status = readCommand(buf, BUFSIZE);
        if (status == 5) {
            printf("\n");
            break;
        }
        if ((pid = fork()) != 0) {
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


