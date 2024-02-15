# Makefile Game of Life - Felix Niemann

CC := gcc 
CFLAGS := -Wall -Wextra -pedantic -Werror
LIBS := -lraylib -lm
SRC := gameoflife.c 
TARGET := gameoflife

.PHONY: all build run clean

all: build run

build: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

run:
	./$(TARGET)
