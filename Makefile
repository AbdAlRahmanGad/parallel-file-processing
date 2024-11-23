# Makefile for main.c project

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Source files
SRCS = main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = main

# Default target
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compiling the source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Running the executable
run: $(TARGET)
	./$(TARGET)

# Cleaning up
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean run
