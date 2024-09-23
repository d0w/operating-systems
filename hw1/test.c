#include <unistd.h> 
#include <string.h>

int main() {
    char *args[] = {"echo", "a\nb\nc", NULL};
    char command[100];
    // strcpy(command, args[0]);
    strcat(command, args[0]);
    return execvp("echo", args);

}


