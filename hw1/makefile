CC=gcc
CFLAGS=-Wall -Werror
RM = rm -f

default: all

all: shell.c
	$(CC) -o shell shell.c $(CFLAGS)

clean veryclean:
	$(RM) shell