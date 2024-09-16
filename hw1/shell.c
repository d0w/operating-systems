#include "parser.h"

struct Pipeline* initPipeline() {
	struct Pipeline* pipe = malloc(sizeof(struct Pipeline));

	pipe->commands = NULL;
	pipe->background = FALSE;

	return pipe;
}

struct Command* initCommand() {
	struct Command* command = malloc(sizeof(struct Command));
	for(int i = 0; i < BUFSIZE; i++) {
		command->args[i] = NULL;
	}
	command->next = NULL;

	return command;
}

int cleanPipeline(struct Pipeline *pipe) {
    struct Command *curr = pipe->commands;
    struct Command *next = NULL;

    while(curr != NULL) {
        next = curr->next;
        cleanCommand(curr);
        curr = next;
    }

    free(pipe);
    return 0;
}

int cleanCommand(struct Command *command) {
    free(command->args);
    free(command);
    return 0;
}





// read command from stdin or file
// FILE *stream
int readCommand(char *buffer, int size, struct Pipeline *pipe) {
    printf("my_shell$");
    fflush(NULL);
    
    // checks for EOF
    if(fgets(buffer, BUFSIZE, stdin) == NULL) {
        return -1;
    }

    pipe->commands = parseLine(buffer); // calls parser.c function



    struct Command *curr = pipe->commands;

    if (curr == NULL) {
        return 5;
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

    // free(curr);
    return 0;
}

// int executePipeline(struct Pipeline *pipe) {
//     struct Command *curr = pipe->commands;
//     int status;

//     while (curr != NULL) {
//         // fork and execute each command in the pipeline
//         pid_t pid = fork();
//         if (pid == 0) {
//             // child process
//             execvp(curr->args[0], curr->args);
//             perror("execvp failed");
//             exit(-1);
//         } else if (pid < 0) {
//             perror("fork failed");
//             return -1;
//         } else {
//             // parent
//             if (waitpid(pid, &status, 0) == -1) {
//                 perror("waitpid failed");
//                 return -1;
//             }
//         }
//         // curr = curr->next;
//         curr = NULL;
//     }

//     // wait for all child processes to finish
//     // while (wait(&status) > 0);
//     // while (wait(&status))

//     return 0;
// }

// main shell
int shell() {
    char buf[BUFSIZE];
    struct Pipeline *pipe = initPipeline();

    // pid_t pid;
    int status;
    while ((status = readCommand(buf, BUFSIZE, pipe)) >= 0) {

        if (status > 0) {
            switch(status) {
                case 5:
                    break;
                default:
                    break;
            }
        }
        else {
            // execute command logic here


        }
        // if ((pid = fork()) != 0) {
        //     // parent process
        //     // waitpid(-1, &status, 0);
        // } else {
        //     // child code
        //     // execve(buf, NULL, 0);
        // }
        memset(buf, '\0', sizeof(buf)); 
    }
    return 0;
}

int main(int argc, char **argv) {
    int output = shell();
    return output || 0;
}


