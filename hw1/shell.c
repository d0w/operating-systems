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

int executePipeline(struct Pipeline *pipe) {
    struct Command *curr = pipe->commands;
    // struct Command *temp = curr;
    int status;

    while (curr != NULL) {
        printf("Command: %s\n", curr->args[0]);
        for (int i = 0; curr->args[i] != NULL; i++) {
            printf("Arg: %s\n", curr->args[i]);
        }
        printf("In: %s\n", curr->inPath);
        printf("Out: %s\n", curr->outPath);
        printf("Background: %d\n", pipe->background);
        curr = curr->next;
    }
    curr = pipe->commands;



    while (curr != NULL) {
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
            int response = execvp(curr->args[0], curr->args);
            if (response != 0) {
                fprintf(stderr, "ERROR: Command not found: %s\n", curr->args[0]);
                exit(-1);
            }
        } else if (pid < 0) {
            fprintf(stderr,"ERROR: fork failed");
            return -1;
        } else {
            // parent

            while (waitpid(pid, &status, 0) == -1) {
                fprintf(stderr, "ERROR: waitpid failed");
                return -1;
            }
            // printf("Child %d exited with status %d\n", pid, WEXITSTATUS(status));
        }
        // curr = curr->next;
        curr = NULL;
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


