CC=gcc
CFLAGS=-Wall -Werror
OBJ = main.o parser.o
TARGET = db

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

main.o: main.c main.h
	$(CC) $(CFLAGS) -c main.c

parser.o: parser.c main.h
	$(CC) $(CFLAGS) -c parser.c
	
clean:
	rm -f $(OBJ) $(TARGET)
