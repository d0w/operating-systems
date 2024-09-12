
# Submissions Guidelines
Your shell must be written in C and run on Linux. It must compile without any
warning/errors and run on ec440.bu.edu. (i.e., gcc -Werror will be in use)

In your home directory create a folder (e.g., project1) and place README, makefile,
myshell.c files (potentially, myshell.h) there. Switch to the project1 directory and
execute submit1.

A confirmation mail of your submission is sent to your account on ec440.bu.edu. You
can read this mail by executing mail.

To automatically test your solution, we execute make (no arguments will be provided to
make). After make terminates, we execute the binary ./myshell for extensive testing.
(Please make sure that executing make compiles your source code and generates a
binary named myshell)

In the README file explain what you did. If you had problems, tell us why and what.
You are allowed to resubmit your files. The last submission before the deadline will be
graded

Do not forget that you must support the '-n' argument to suppress the output of the
shell prompt for automated testing.

# Specifications
Design basic shell with commands
1. `my_shell$` as prompt
2. Be able to execute commands
   1. Access binaries by searching directories determined by PATH environment var passed to shell
   2. Commands have arguments
3. Interpret/Execute '<' '>' '|' and '&'
   1. '<'- Takes input from file
   2. '>' - Output writes to file
   3. '|' - Output from previous commands pipes (inputs) to next command
   4. '&' Allows user to execute in background
      1. Next command allowed immediately
4. If error, display error message
   1. ERROR: your_error_message
      1. -n when starting shell program, shell doesn't output any command prompt (no"my_shell$")
5. CTRL-D to exit shell
6. Max length of commands/filenames is 32 chars, max length of input line is 512
7. Collect exit codes of all processes it spawns
8. Use fork(2) system call and execvp(2) to execute commands
9. Use waitpid(2) or wait(2) to wait for program to complete execution
10. Signals (SIGCHLD) for statuses of processes in background


# Hints
1. A simple shell such as this needs at least a simple command-line parser to figure out
what the user is trying to do. To read a line from the user, you may use fgets(3).
2. If a valid command has been entered, the shell should fork(2) to create a new (child)
process, and the child process should exec the command.
3. Before calling exec to begin execution, the child process may have to close stdin (file
descriptor 0) or stdout (file descriptor 1), open the corresponding file or pipe (with
open(2) for files, and pipe(2) for pipes), and use dup2(2) or dup(2) to make it the
appropriate file descriptor. After calling dup2(2), close the old file descriptor.
4. The main challenge of calling execvp(2) is to build the argument list correctly. If you
use execvp(2), remember that the first argument in the array is the name of the
command itself, and the last argument must be a null pointer.
5. The easiest way to redirect input and output is to follow these steps in order:
(a) open (or create) the input or output file (or pipe).
(b) close the corresponding standard file descriptor (stdin or stdout).
(c) use dup2 to make file descriptor 0 or 1 correspond to your newly opened file.
(d) close the newly opened file (without closing the standard file descriptor).
6. When executing a command line that requires a pipe, the pipe must be created before
forking the child processes. Also, if there are multiple pipes, the command(s) in the
middle may have both input and output redirected to pipes. Finally, be sure the pipe is
closed in the parent process, so that termination of the process writing to the pipe will
automatically close the pipe and send an EOF (end of file) to the process reading the
pipe.
7. Any pipe or file opened in the parent process may be closed as soon as the child is
forked – this will not affect the open file descriptor in the child.
8. While the project assignment talks about system calls, feel free to use the libc wrapper
functions, documented in their corresponding beautiful man pages instead.

# Resources Used
[https://www.gnu.org/software/libc/manual/html_mono/libc.html#Implementing-a-Shell](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Implementing-a-Shell)