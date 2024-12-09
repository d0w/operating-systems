CC=gcc -Werror -Wall -g 
default: tls

tls: tls.c tls.h
	$(CC) -c -lpthread -o tls.o tls.c

push:
	scp -P 10001 ./TIME ./tls.h ./tls.c ./makefile ./README terrier009@ec440.bu.edu:/home/uebungen/terrier/terrier009/project4-testing
	
submit:
	scp -P 10001 ./TIME ./tls.h ./tls.c ./makefile ./README terrier009@ec440.bu.edu:/home/uebungen/terrier/terrier009/project4


clean:
	rm tls.o tls
