#include <unistd.h> 
#include <string.h>

int main() {
    char *args[] = {"echo", "Hello, World!", NULL};
    char command[100];
    strcpy(command, "/bin/wefwef");
    strcat(command, args[0]);
    return execvp(command, args);

}


