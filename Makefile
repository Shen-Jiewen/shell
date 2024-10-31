# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Iinclude

# Directories
SRC_DIR = src
INCLUDE_DIR = include

# Source Files
SRC = $(wildcard $(SRC_DIR)/*.c)

# Target executable
TARGET = shell

# Default rule
all: $(TARGET)

# Compile all source files directly to create the executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Clean up generated files
clean:
	rm -f $(TARGET)

# Phony targets to avoid conflicts with files named "all" or "clean"
.PHONY: all clean
