CC	= $(CROSS_COMPILE)gcc

TARGET 	 = writer
SOURCES = writer.c
OBJECTS = writer.o

all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

%.o : %.c
	$(CC) -c $< -o $@

clean:
	rm -rf $(TARGET)
	rm -rf *.o

.PHONY: all clean





