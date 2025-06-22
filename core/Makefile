include ../Makefile.inc
.PHONY: all clean fs

all: kernel.o multiboot.o GDT.o IDT.o irq.o isrs.o kbd.o kprintf.o timer.o vga.o mem.o panic.o alloc.o vfs.o fs

%.o: %.c
	${CC} ${CFLAGS} -I../include -c -o $@ $<

fs:
	cd fs; ${MAKE} ${MFLAGS}

clean:
	rm -f *.o
