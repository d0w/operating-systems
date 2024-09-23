#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>

/*
 * Simple program that creates a pipe, forks, and then reads messages from
 * stdin in the parent, sends them to the client via the pipe, and the
 * client prints the received message
*/

#define STDIN 0
#define STDOUT 1

int main(int argc, char **argv) {
    int len;
    char buffer[256];
    char recbuf[256];
    int pipefd[2];
    int res;
    pid_t pid;

    res = pipe(pipefd);
    if (res != 0) {
        perror("Error in pipe(): ");
    }

    pid = fork();
    if (pid > 0) {
      //parent
      //print some book-keeping information
      printf("I'm the parent and my pid is: %d and the child's is %d\n",
          getpid(), pid);

      //read messages from stdin and send them over the pipe
      while(1) {
        //clear buffer so no stale content
        memset(buffer, 0, 256);

        //read message from stdin/terminal
        fgets(buffer, 256, stdin);

        //get message length
        len = strlen(buffer);

        //write message length
        write(pipefd[1], &len, 4);

        //write message content
        write(pipefd[1], buffer, len);

        //if the message was "end" we stop
        if (strncmp(buffer, "end", 3) == 0) {
            break;
        }
      }
    } else if (pid == 0) {
      //child

      //process messages
      while(1) {
        //make sure the buffer is empty and no stale content there
        memset(recbuf, 0, 256);

        //get the length of the message
        read(pipefd[0], recbuf, 4);
        len = (int)(recbuf[0]);

        //read the message content
        read(pipefd[0], recbuf, len);

        //bail if we get the message "end"
        if (strncmp(recbuf, "end", 3) == 0) {
            break;
        }

        //print the message we just got
        printf("From the child: %s\n", recbuf);
      }
    } else {
      //problem
      perror("Error in fork(): ");
    }
    return 0;
}