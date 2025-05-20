include ../Makefile.inc
.PHONY: all clean core

all: GDT.o IDT.o irq.o isrs.o kbd.o kprintf.o timer.o vga.o mem.o panic.o alloc.o

%.o: %.c
	${CC} ${CFLAGS} -I../include -c -o $@ $<

clean:
	rm -f *.o
