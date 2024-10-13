CC=gcc -Werror -Wall -g 
all: threadlib main
	$(CC) -o main main.o threads.o

main: main.c
	$(CC) -c -o main.o main.c

test-threads-real: test-threads.c
	gcc -Wall -Werror -o test-threads-real test-threads.c -lpthread

test-threads-lib: threadlib test-threads.c
	$(CC) -o test-threads-lib test-threadlib.c threads.o

threadlib: threads.c
	$(CC) -c -o threads.o threads.c

push: 
	

clean:
	rm threads.o test-threads
