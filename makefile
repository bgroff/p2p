CFLAGS=-g 
CC=gcc
LIBS=-lpthread -lresolv -lrt
all: webserver

webserver: main.o receiver.o sender.o rds.o cache.o list.o directory.o
	$(CC) $(CFLAGS) main.o receiver.o sender.o rds.o \
	cache.o list.o directory.o $(LIBS) -o p2p
main.o: main.c
	$(CC) main.c -c $(CFLAGS)
receiver.o: receiver.c
	$(CC) receiver.c -c $(CFLAGS)
sender.o: sender.c
	$(CC) sender.c -c $(CFLAGS)
rds.o: rds.c
	$(CC) rds.c -c $(CFLAGS)
cache.o : cache.c
	$(CC) cache.c -c  $(CFLAGS)
list.o : list.c
	$(CC) list.c -c $(CFLAGS)
directory.o : directory.c
	$(CC) directory.c -c $(CFLAGS)
clean:
	rm *.o p2p
