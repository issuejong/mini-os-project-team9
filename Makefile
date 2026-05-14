CC=gcc
CFLAGS=-Wall -Wextra -Iinclude -pthread
TARGET=mini_os

SRCS=src/main.c \
     src/filesystem.c \
     src/shell.c \
     src/commands.c \
     src/persistence.c \
     src/thread_utils.c

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)