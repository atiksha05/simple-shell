# Simple Shell Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = simple-shell
SRC = main.c
RUNOPTIONS = "Prompt> "

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET) $(RUNOPTIONS)

clean:
	rm -f $(TARGET) *.o

vrun: $(TARGET)
	valgrind ./$(TARGET) $(RUNOPTIONS)