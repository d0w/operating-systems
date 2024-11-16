CC=gcc -Werror -Wall -g 
default: tlslib

threadlib: tls.c
	$(CC) -c -o tls.o tls.c

push: threadlib tlslib.c README
	scp -P 10001  ./testSemaphore.c ./test-threads.c ./ec440threads.h ./threads.c ./test-threadlib.c ./makefile ./README terrier009@ec440.bu.edu:/home/uebungen/terrier/terrier009/project3-testing
	
submit:
	scp -P 10001 ./tls.h ./tls.c ./makefile ./README terrier009@ec440.bu.edu:/home/uebungen/terrier/terrier009/project3


clean:
	rm tls.o tls
