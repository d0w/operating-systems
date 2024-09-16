CC=gcc
CFLAGS=-Wall -Werror
RM = rm -f

default: all

all: shell.c parser.c
	$(CC) parser.c shell.c -o myshell $(CFLAGS)

clean veryclean:
	$(RM) shell