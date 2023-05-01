# Makefile for client and server

CC = gcc
OBJCS = client0.o
OBJCSS = server0.o
COMMON = common.o

# Setting C99 compliance and extra compiler warning options
CFLAGS = -std=c99 -g -pedantic -Wall -Wextra -I .
# setup for system
DEPS = common.h

all: client0 server0


client0: $(OBJCS) $(COMMON)
	$(CC) -o $@ $(OBJCS) $(COMMON)

server0: $(OBJCSS) $(COMMON)
	$(CC) -o $@ $(OBJCSS) $(COMMON)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

docker: Dockerfile
	docker build --progress plain --platform linux/x86_64 . -t lab0

clean:
	rm client0 server0 *.o
