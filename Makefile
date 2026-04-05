CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -pedantic -Iinclude

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))

TARGET := bin/prk
ifeq ($(OS),Windows_NT)
TARGET := bin/prk.exe
endif

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ) | bin
	$(CC) $(OBJ) -o $(TARGET)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

bin:
	mkdir -p bin

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)
	rm -rf build