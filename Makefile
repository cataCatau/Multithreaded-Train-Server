
CC = gcc

CFLAGS = -Wall -g -I/usr/include/libxml2


LIBS = -lxml2 -lpthread

all: server client

server: server.c commands.c xml_handler.c shared.h
	$(CC) $(CFLAGS) server.c commands.c xml_handler.c -o server $(LIBS)

client: client.c
	$(CC) -Wall -g client.c -o client

clean:
	rm -f server client