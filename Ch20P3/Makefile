CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

TARGET = program
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	powershell -NoProfile -Command "Remove-Item -Force -ErrorAction SilentlyContinue $(OBJS), $(TARGET)"
