writer:writer.c
	gcc writer.c -o writer
	
clean:
	rm -f *.o writer *.map *.elf *.out
	
CROSS_COMPILE:
	aarch64-none-linux-gnu-gcc -g -Wall   -c -o writer.o writer.c
	
,DELETE_ON_ERROR:
all:*.o *.out *.elf writer
