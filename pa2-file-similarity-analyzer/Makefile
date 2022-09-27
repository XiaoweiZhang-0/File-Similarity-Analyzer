CC=gcc
CFLAGS=-Wall
main: queue.o cmd.o hash.o jsd.o main.c
	$(CC) $(CFLAGS) -o main queue.o cmd.o hash.o jsd.o main.c -lpthread -lm

queue: queue.c
	$(CC) $(CFLAGS) -c queue.o queue.c 
cmd: queue.o cmd.c
	$(CC) $(CFLAGS) -c cmd.o queue.o cmd.c 
hash: hash.c
	$(CC) $(CFLAGS) -c hash.o hash.c
jsd: hash.o jsd.c
	$(CC) $(CFLAGS) -c jsd.o hash.o jsd.c

.PHONY: clean

clean:
	rm *.o

