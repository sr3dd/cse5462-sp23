# Makefile for client and server
# using GCC as compiler
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .

CC = gcc

# source (.c) files
SRCS = $(wildcard $(SRC_DIR)/*.c)
# suffix replacement rule to map .c to .o files
OBJCS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
# path to include/ directory with .h files
INCLUDES = -Isrc/include -lm
# name of executable
TARGET = $(BIN_DIR)/drone6

# C flags
# -std=c99 : set compliance to C99
# -g : add debug symbols to generated executable file
# -pedantic, -Wall, -Wextra : compiler warning options 
# -Iinclude
CFLAGS = -std=c99 -g -pedantic -Wall -Wextra

.PHONY: depend clean docker

all: $(TARGET)
	@echo Compiling drone6 from src/

$(TARGET): $(OBJCS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(TARGET) $(OBJ_DIR)

depend: $(SRCS)
	makedepend $(INCLUDES) $^
