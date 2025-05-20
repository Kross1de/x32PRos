include Makefile.inc

DIRS = core

.PHONY: all clean core run iso

kernel: start.o link.ld main.o core
	${LD} -T link.ld -o kernel *.o core/*.o

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

core:
	cd core; ${MAKE} ${MFLAGS}

iso: kernel
	mkdir -p iso/boot/grub/
	cp kernel iso/boot/kernel
	cp boot/grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o pros.iso iso

run:
	qemu-system-i386 -cdrom pros.iso

start.o: start.asm
	nasm -f elf -o start.o start.asm

clean:
	-rm -f *.o kernel pros.iso
	-rm -rf iso
	-for d in ${DIRS}; do (cd $$d; ${MAKE} clean); done
