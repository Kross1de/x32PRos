include Makefile.inc

DIRS = core

.PHONY: all clean core run 

kernel: start.o link.ld main.o core
	${LD} -T link.ld -o kernel *.o core/*.o

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

core:
	cd core; ${MAKE} ${MFLAGS}
	
run:
	qemu-system-i386 -kernel kernel

start.o: start.asm
	nasm -f elf -o start.o start.asm

clean:
	-rm -f *.o kernel
	-for d in ${DIRS}; do (cd $$d; ${MAKE} clean); done
