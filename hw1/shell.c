#include "parser.h"

// Pipeline and Command Struct initialiazations and clean up functions
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





/**
 * Reads a command from the user and parses it into a pipeline of commands
 * @return 0 if successful, -1 if EOF
 */
int readCommand(char *buffer, int size, struct Pipeline *pipe, int usePrompt) {
    if (usePrompt) {
        printf("my_shell$ ");
    }
    
    // checks for EOF
    if(fgets(buffer, BUFSIZE, stdin) == NULL) {
        return -1;
        // exit(0);
    }

    int background = 0;
    pipe->commands = parseLine(buffer, &background); // calls parser.c function
    pipe->background = background;

    return 0;
}

/**
 * Executes a command and returns the file descriptor for the next command's stdin
 * Forks a child process to execute the command. Uses pipes for inter-process communication
 * @return file descriptor for the next command's stdin if successful, -1 if error
 */
int executeCommand(struct Command *cmd, int inFd, int wait) {
    int filed[2];
    pipe(filed);

    pid_t pid = fork();
    if (pid == 0) {


        // handling input redirection. First create or open files needed for redirection
        if(cmd->inPath) {
            inFd = open(cmd->inPath, O_RDONLY);
            if(inFd < 0 ) {
                exit(2);
            }
        }
        // output redirection
        if(cmd->outPath) {
            // create/overwrite file and set read end of pipe to write to file
            // permission bits -> owner (rw) / group (r) / other (r)
            filed[1] = creat(cmd->outPath, 0644);
            if (filed[1] < 0) {
                exit(3);
            }
        }

        // redirect stdin to the read end of the pipe
        dup2(inFd, STDIN_FILENO);

        if(cmd->next) {
            // if there is a next command, pipe the output of the current command to the next command
            // by redirecting stdout to the write end of the pipe
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
        perror("ERROR: fork");
        return -1;
    } else {
        // parent process
        close(filed[1]);

        // close inFd if it opened a file
        if (inFd != STDIN_FILENO) {
            close(inFd);
        }

        if (!cmd->next) {
            // if there are no more commands, close the pipe
            close (filed[0]);
        }

        // for background processes, return the read end of the pipe
        if (!wait) {
            return filed[0];
        }

        // wait for child process to finish. use while loop in case there are multiple child processes
        int returnStatus;
        while (waitpid(pid, &returnStatus, 0) == -1) {
            perror("ERROR: waitpid");
            return -1;
        }

        // get return status from child process
        if (WIFEXITED(returnStatus)) {  
            int exitStatus = WEXITSTATUS(returnStatus);
            if (exitStatus < 0) {
                perror("ERROR: command exited with errors");
                return -1;
            }
        } else {
            perror("ERROR: command did not exit properly");
            return -1;
        }

        // set the input file descriptor to the read end of the pipe for the next command
        inFd = filed[0];
    }
    return inFd;
}

/**
 * Executes a pipeline of commands
 * @return 0 if successful, -1 if error
 */
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

    // loop through all commands in the pipeline
    while (curr != NULL) {
        // get the input fd from the previous command and pass to the next if stdin needs to be redirected
        inFd = executeCommand(curr, inFd, !pipe->background);
        if (inFd < 0) {
            return -1;
        }
        curr = curr->next;
        // curr = NULL;
    }

    // if we are not reading from stdin anymore, close the file descriptor
    if (inFd != STDIN_FILENO) {
        close(inFd);
    }


    return status;
}

// signal handler
void sigchldHandler(int sig) {
    int status;

    // collect all child processes that have finished with no hang to prevent blocking
    while (waitpid(-1, &status, WNOHANG) > 0);


    // if (WIFEXITED(status)) {
    //     printf("Child exited with status %d\n", WEXITSTATUS(status));
    // }
}

// main shell
int shell(char *arg) {
    char buf[BUFSIZE];
    struct Pipeline *pipe = initPipeline();

    int usePrompt = 1;
    if (strcmp(arg, "-n") == 0) {
        usePrompt = 0;
    }

    // create a signal handler for SIGCHLD
    struct sigaction sa;
    sa.sa_handler = sigchldHandler;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("ERROR: sigaction");
        exit(1);
    }

    // pid_t pid;
    int status;
    while ((status = readCommand(buf, BUFSIZE, pipe, usePrompt)) >= 0) {

        if (status > 0) {
            // handle extra status messages here
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


