CC=gcc -Werror -Wall -g 

all: fs.o

main: fs.o disk.o
	$(CC) -o main $^ main.c

disk.o: disk.c disk.h

fs: fs.o disk.o
	gcc -Wall -g -o $@ $^

%.o: %.c
	gcc -o $@ -Wall -Werror -g -c $<

clean:
	rm -f fs *.o *~

submit:
	scp -P 10001 ./disk.h ./fs.c ./fs.h ./makefile ./README terrier009@ec440.bu.edu:/home/uebungen/terrier/terrier009/project5

push:
	scp -P 10001 ./disk.o ./main.c ./disk.h ./disk.c ./fs.c ./fs.h ./makefile ./README terrier009@ec440.bu.edu:/home/uebungen/terrier/terrier009/project5-testing


