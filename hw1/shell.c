#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BUFSIZE 512
#define DELIMITERS "<>|&;"

struct process {
    char *name;
    char **args;
    int pid;
    int status;
};

// TRY USING FSM
/* 
If normal character, just add to array
if special charadcter, switch state
    - state 0 - add char to array
    - state 1 (|) - take output from previous state and pipe to next command
    - state 2 (<) - take input from file, go back to state 0
    - state 3 (>) - output to file, go back to state 0
    - state 4 (&) - run in background, go back to state 0
*/

// parse command and separate into string array
char **parseCommand(char *buffer) {
    char **args = (char **) malloc(BUFSIZE * sizeof(char *));
    char *tokens;
    int i = 0;
    tokens = strtok(buffer, DELIMITERS);
    while (tokens != NULL) {
        args[i] = tokens;
        i++;
        tokens = strtok(NULL, "\n");
    }
    args[i] = NULL;
    return args;
}


// read command from stdin or file
int readCommand(char *buffer, int size, FILE *stream) {
    printf("my_shell$");
    if (stream == NULL) {
        fgets(buffer, size, stdin);
    }
    else {
        fgets(buffer, size, stdin);
    }
    // char **cmds = parseCommand(buffer);

    // holds current command
    char *curr = (char *)malloc(sizeof(char) * BUFSIZE);

    // loop through buffer and check for special characters
    int i = 0;
    int j = 0;
    while (buffer[i] != '\n') {
        switch(buffer[i]) {
            case '|':
                // do something with previous command
                printf("%s\n", curr);
                curr = memset(curr, 0, strlen(curr));
                j = -1;
                break;
            case '<':
                // 
                break;
            case '>':
                break;
            case '&':
                // end of command
                break;
            default:
                curr[j] = buffer[i];
                break;
        }
        i++;
        j++;
        
    }

    // run command


    printf("%s\n", curr);


    // print debugging
    
    // do command logic here

    free(curr);
    return 0;
}

// main shell
int shell() {
    while (1) {
        char buf[BUFSIZE];
        readCommand(buf, BUFSIZE, NULL);
        if (strcmp(buf, "exit\n") == 0) {
            break;
        }
        if (fork() != 0) {
            // parent process
            // waitpid(-1, &status, 0);
        } else {
            // child code
            execve(buf, NULL, 0);
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    int output = shell();
    return output || 0;
}


