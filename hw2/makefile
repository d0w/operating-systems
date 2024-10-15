CC=gcc -Werror -Wall -g 
all: threadlib main
	$(CC) -o main main.o threads.o

main: main.c
	$(CC) -c -o main.o main.c

test-threads-real: test-threads.c
	gcc -Wall -Werror -o test-threads-real test-threads.c -lpthread

test-threads-lib: threadlib test-threadlib.c
	$(CC) -o test-threads-lib test-threadlib.c threads.o

threadlib: threads.c
	$(CC) -c -o threads.o threads.c

push: ec440threads.h threads.c test-threadlib.c README
	scp -P 10001  ./ec440threads.h ./threads.c ./test-threadlib.c ./makefile ./README terrier009@ec440.bu.edu:/home/uebungen/terrier/terrier009/project2-testing
	

clean:
	rm threads.o test-threads
