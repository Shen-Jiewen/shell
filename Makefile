CC = gcc
CFLAGS = -Wall -Iinclude
TARGET = shell

SRC = src/main.c src/uart.c src/shell.c src/command.c src/history.c 

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
