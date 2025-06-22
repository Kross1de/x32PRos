include ../../Makefile.inc
.PHONY: all clean

all: ext2_initrd.o

%.o: %.c
	${CC} ${CFLAGS} -I../../include -c -o $@ $<

clean:
	-rm -f *.o
