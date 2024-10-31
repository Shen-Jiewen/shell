CC = gcc
CFLAGS = -Wall -Iinclude

SRC = src/main.c src/shell.c src/command.c src/history.c src/pal.c
all: shell

shell: $(SRC)
	$(CC) $(CFLAGS) -o shell $(SRC)

clean:
	rm -f shell
