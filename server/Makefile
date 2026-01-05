CFLAGS := -g -Wall -Werror

CC := gcc

TARGET  = aesdsocket
SOURCES = aesdsocket.c
OBJECTS = aesdsocket.o

all: 
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -rf $(TARGET)
	rm -rf *.o

.PHONY: all clean
