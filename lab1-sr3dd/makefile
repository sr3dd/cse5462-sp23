# Makefile for client and server

CC = gcc
OBJCS = client1.o
OBJCSS = server1.o
COMMON = common.o

# Setting C99 compliance and extra compiler warning options
CFLAGS = -std=c99 -g -pedantic -Wall -Wextra -I .
# setup for system
DEPS = common.h

all: client1 server1

client1: $(OBJCS) $(COMMON)
	$(CC) -o $@ $(OBJCS) $(COMMON)

server1: $(OBJCSS) $(COMMON)
	$(CC) -o $@ $(OBJCSS) $(COMMON)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

docker: Dockerfile
	docker build --progress plain --platform linux/x86_64 . -t lab1

clean:
	rm client1 server1 *.o
