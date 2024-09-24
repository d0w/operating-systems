#include "parser.h"

struct Pipeline* initPipeline() {
	struct Pipeline* pipe = malloc(sizeof(struct Pipeline));

	pipe->commands = NULL;
	pipe->background = FALSE;

	return pipe;
}

struct Command* initCommand() {
	struct Command* command = malloc(sizeof(struct Command));
    command->args = NULL;
	command->next = NULL;
    command->inPath = NULL;
    command->outPath = NULL;

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
int readCommand(char *buffer, int size, struct Pipeline *pipe, int usePrompt) {
    if (usePrompt) {
        printf("my_shell$ ");
    }
    
    // checks for EOF
    if(fgets(buffer, BUFSIZE, stdin) == NULL) {
        return -1;
        // exit(0);
    }
    // print out buffer char by char
    // for (int i = 0; buffer[i] != '\0'; i++) {
    //     printf("%c ", buffer[i]);
    // }
    int background = 0;
    pipe->commands = parseLine(buffer, &background); // calls parser.c function
    pipe->background = background;



    // struct Command *curr = pipe->commands;

    // if (curr == NULL) {
    //     return 5;
    // }

    // while(curr != NULL) {
    //     char **commandPtr = curr->args;
    //     while (*commandPtr != NULL) {
    //         printf("%s ", *commandPtr);
    //         commandPtr++;
    //     }
    //     printf("\n");
    //     curr = curr->next;
    // }


    // print debugging
    
    // do command logic here

    // free(curr);
    return 0;
}

int executeCommand(struct Command *cmd, int inFd, int wait) {
    int filed[2];
    pipe(filed);

    pid_t pid = fork();
    if (pid == 0) {
        // child process
        // might need next line
        // strcpy(command, "/bin/");
        // strcat(command, curr->args[0]);
        // char *arg[] = {"echo", "-e", "a\nb\nc", NULL};
        // int response = execvp(arg[0], arg+1);
        // printf("Command: %s\n", curr->args[0]);
        // for (int i = 0; curr->args[i] != NULL; i++) {
        //     printf("Arg: %s\n", curr->args[i]);
        // }
        // char *arg[] = {"echo", "yay", ">", "tes.txt", NULL};
        // int response = execvp(arg[0], arg);

        // handling input redirection. First create or open files needed for redirection
        if(cmd->inPath) {
            inFd = open(cmd->inPath, O_RDONLY);
            if(inFd < 0 ) {
                exit(2);
            }
        }
        // output redirection
        if(cmd->outPath) {
            // permission bits -> owner (rw) / group (r) / other (r)
            filed[1] = creat(cmd->outPath, 644);
            if (filed[1] < 0) {
                exit(3);
            }
        }

        // redirect stdin to the read end of the pipe
        dup2(inFd, STDIN_FILENO);

        if(cmd->next) {
            // if there is a next command, pipe the output of the current command to the next command
            // by redirecting stdout to the write end of the pipe
            // and redirecting stdin to the read end of the pipe
            dup2(filed[1], STDOUT_FILENO);
        } else if (cmd->outPath) {
            // if output file, redirect stdout to the file
            dup2(filed[1], STDOUT_FILENO);
        }

        // close pipe file descriptors
        close(filed[0]);
        close(filed[1]);

        // execute command
        if(execvp(cmd->args[0], cmd->args) < 0) {
            fprintf(stderr, "ERROR: execvp failed\n");
			exit(1);
        }

    } else if (pid < 0) {
        fprintf(stderr,"ERROR: fork failed");
        return -1;
    } else {
        // parent process
        close(filed[1]);
        if (inFd != STDIN_FILENO) {
            close(inFd);
        }

        if (!wait) {
            return 0;
        }

        int returnStatus;
        if (waitpid(pid, &returnStatus, 0) == -1) {
            fprintf(stderr, "ERROR: waitpid failed");
            return -1;
        }

        // get return status from child process
        if (WIFEXITED(returnStatus)) {
            int exitStatus = WEXITSTATUS(returnStatus);
            if (exitStatus != 0) {
                fprintf(stderr, "ERROR: command failed with exit status: %d\n", exitStatus);
                return -1;
            }
        } else {
            fprintf(stderr, "ERROR: command did not exit normally\n");
            return -1;
        }

        // set the input file descriptor to the read end of the pipe
        inFd = filed[0];
    }
    return inFd;
}

int executePipeline(struct Pipeline *pipe) {
    struct Command *curr = pipe->commands;
    // struct Command *temp = curr;

    int inFd = STDIN_FILENO;

    // while (curr != NULL) {
    //     printf("Command: %s\n", curr->args[0]);
    //     for (int i = 0; curr->args[i] != NULL; i++) {
    //         printf("Arg: %s\n", curr->args[i]);
    //     }
    //     printf("In: %s\n", curr->inPath);
    //     printf("Out: %s\n", curr->outPath);
    //     printf("Background: %d\n", pipe->background);
    //     curr = curr->next;
    // }
    // curr = pipe->commands;

    int status = 0;

    while (curr != NULL) {
        // get the input fd from the previous command and pass to the next
        inFd = executeCommand(curr, inFd, 1);
        curr = curr->next;
        // curr = NULL;
    }

    // if we are not reading from stdin anymore, close the file descriptor
    if (inFd != STDIN_FILENO) {
        close(inFd);
    }

    // wait for all child processes to finish
    // while (wait(&status) > 0);
    // while (wait(&status))

    return status;
}

// main shell
int shell(char *arg) {
    char buf[BUFSIZE];
    struct Pipeline *pipe = initPipeline();

    int usePrompt = 1;
    if (strcmp(arg, "-n") == 0) {
        usePrompt = 0;
    }

    // pid_t pid;
    int status;
    while ((status = readCommand(buf, BUFSIZE, pipe, usePrompt)) >= 0) {

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
            executePipeline(pipe);

            // clean up
            cleanPipeline(pipe);

            // reinitialize pipeline
            pipe = initPipeline();
        }
        memset(buf, '\0', sizeof(buf)); 
        fflush(NULL);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argv[1] == NULL) {
        return shell("");
    }
    int output = shell(argv[1]);
    return output;
}


