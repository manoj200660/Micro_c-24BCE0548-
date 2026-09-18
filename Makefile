CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = microc.exe

SOURCES = src/main.c src/lexer.c src/token.c
OBJECTS = src/main.o src/lexer.o src/token.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q $(OBJECTS) $(TARGET) 2>nul

.PHONY: all clean