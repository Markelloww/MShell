CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g -Wno-strict-prototypes -Wno-newline-eof
TARGET = mshell
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SOURCES:.c=.o)

.PHONY: all clean install run test compile

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) command_history.txt
	rm -rf /tmp/vfs

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

run: $(TARGET)
	./$(TARGET)

release: CFLAGS += -O2
release: clean $(TARGET)

help:
	@echo "Available targets:"
	@echo "all      - build MShell"
	@echo "clean    - remove build artifacts"
	@echo "install  - install to /usr/local/bin"
	@echo "run      - build and run MShell"
	@echo "help     - show available commands"
