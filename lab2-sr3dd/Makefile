# Makefile for client and server

CC = gcc
OBJCS = client2.o
OBJCSS = server2.o
COMMON = common.o

# Setting C99 compliance and extra compiler warning options
CFLAGS = -std=c99 -g -pedantic -Wall -Wextra -I.
# setup for system
DEPS = common.h

all: client2 server2

client2: $(OBJCS) $(COMMON)
	$(CC) -o $@ $(OBJCS) $(COMMON)

server2: $(OBJCSS) $(COMMON)
	$(CC) -o $@ $(OBJCSS) $(COMMON)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

docker: Dockerfile
	docker build --progress plain --platform linux/x86_64 . -t lab2

clean:
	rm client2 server2 *.o
