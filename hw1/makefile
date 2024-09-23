CC=gcc
CFLAGS=-Wall -Werror
RM = rm -f

default: all

all: shell.c parser.c
	$(CC) parser.c shell.c -o myshell $(CFLAGS)

clean veryclean:
	$(RM) shell

zip: shell.c parser.c shell.h makefile parser.h README
	zip shell.zip shell.c parser.c shell.h makefile parser.h README

push: shell.zip
	scp -P 10001  ./shell.zip terrier009@ec440.bu.edu:/home/uebungen/terrier/terrier009/project1