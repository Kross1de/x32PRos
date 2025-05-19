// main.c
// Main file
/*
 * This file contains the kernel entry point 
 * and also has functions that are used in other files.
 */

#include "include/kernel.h"

/*
 * memcpy
 * Copy from source to destination. Assumes that
 * source and destination are not overlapping.
 */
void *
memcpy(
		void * restrict dest,
		const void * restrict src,
		size_t count
	  ) {
	size_t i;
	unsigned char *a = dest;
	const unsigned char *b = src;
	for ( i = 0; i < count; ++i ) {
		a[i] = b[i];
	}
	return dest;
}

unsigned short *memcpyw(unsigned short *dest, const unsigned short *src, int count) {
    for (int i = 0; i < count; i++) {
        dest[i] = src[i];
    }
    return dest;
}

/*
 * memset
 * Set 'count' bytes to 'val'.
 */
void *
memset(
		void *b,
		int val,
		size_t count
	  ) {
	size_t i;
	unsigned char * dest = b;
	for ( i = 0; i < count; ++i ) {
		dest[i] = (unsigned char)val;
	}
	return b;
}

/*
 * memsetw
 * Set 'count' shorts to 'val'.
 */
unsigned short *
memsetw(
		unsigned short *dest,
		unsigned short val,
		int count
	  ) {
	int i;
	i = 0;
	for ( ; i < count; ++i ) {
		dest[i] = val;
	}
	return dest;
}

/*
 * strlen
 * Returns the length of a given 'str'.
 */
int strlen(const char *str) {
    int len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}
/*
 * inportb
 * Read from an I/O port.
 */
unsigned char inportb(unsigned short port)
{
  unsigned char rv;
  __asm__ __volatile__("inb %1, %0" : "=a"(rv) : "dN"(port));
  return rv;
}

/*
 * outportb
 * Write to an I/O port.
 */
void outportb(unsigned short port, unsigned char data)
{
  __asm__ __volatile__("outb %1, %0" : : "dN"(port), "a"(data));
}

/*
 * Kernel entry point
 */
int main()
{
        gdt_install();
        idt_install();
        isrs_install();
        irq_install();
        __asm__ __volatile__("sti");
        //timer_install();
        keyboardInstall();
        init_video();
        puts("Type something: ");
        for(;;);
        return 0;
}
