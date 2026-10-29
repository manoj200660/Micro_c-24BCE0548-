
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = microc.exe

SOURCES = src/main.c \
          src/lexer.c \
          src/token.c \
          src/parser.c \
          src/semantic.c \
          src/tac.c \
          src/codegen.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) tests/sample.mc

clean:
	$(RM) $(TARGET) output.s
