UNAME=$(shell "uname -s")

CC = gcc

CFLAGS=-std=c2x -I/usr/include/SDL2 -D_REENTRANT -DHWY_SHARED_DEFINE -I/usr/include/webp 
CFLAGS += -Wall -Wextra -g -Og -fsanitize=address
LDFLAGS=-lm -lSDL2 -lSDL2_image -lSDL2_mixer -fsanitize=address

BIN=bin
EXEC=chess.x86_64

SRC = $(wildcard src/*.c) $(wildcard src/**/*.c)
OBJ = $(SRC:%.c=$(BIN)/%.o)

.PHONY=dirs clean

all: dirs main
	./$(EXEC)

dirs:
	mkdir -p ./$(BIN)/src/render

clean:
	rm -rf ./bin

main: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $^ $(LDFLAGS)

$(BIN)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)