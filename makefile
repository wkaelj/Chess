UNAME=$(shell "uname -s")

CC = gcc

CFLAGS=-std=c++11 -I/usr/include/SDL2 -D_REENTRANT -DHWY_SHARED_DEFINE -I/usr/include/webp 
CFLAGS += -Wall -Wextra -g -Og
LDFLAGS=-lm -lSDL2 -lSDL2_image -lstdc++

BIN=bin
EXEC=chess.x86_64

SRC = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)
OBJ = $(SRC:%.cpp=$(BIN)/%.o)

.PHONY=dirs clean

all: dirs main
	./$(EXEC)

dirs:
	mkdir -p ./$(BIN)/src/render

clean:
	rm -rf ./bin

main: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $^ $(LDFLAGS)

$(BIN)/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)