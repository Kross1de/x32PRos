// main.c
// Main file
/*
 * This file contains the kernel entry point 
 * and also has functions that are used in other files.
 */

#include "include/kernel.h"
#include "include/multiboot.h"

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
int main(struct multiboot *mboot_ptr)
{
        gdt_install();
        idt_install();
        isrs_install();
        irq_install();
        __asm__ __volatile__("sti");
        //timer_install();
        keyboardInstall();
        init_video();
        settextcolor(12,0);
        kprintf("[%s %s]\n", KERNEL_UNAME, KERNEL_VERSION);
        settextcolor(1,0);
        /* Multiboot Debug */
        kprintf("Received the following MULTIBOOT data:\n");
        settextcolor(7,0);
        kprintf("Flags: %x\t", mboot_ptr->flags);
        kprintf("Mem Lo: %x\t", mboot_ptr->mem_lower);
        kprintf("Mem Hi: %x\n", mboot_ptr->mem_upper);
        kprintf("Boot dev: %x\t", mboot_ptr->boot_device);
        kprintf("cmdline: %x\t", mboot_ptr->cmdline);
        kprintf("Mods: %x\n", mboot_ptr->mods_count);
        kprintf("Addr: %x\t", mboot_ptr->mods_addr);
        kprintf("Syms: %x\t", mboot_ptr->num);
        kprintf("Syms: %x\n", mboot_ptr->size);
        kprintf("Syms: %x\t", mboot_ptr->addr);
        kprintf("Syms: %x\t", mboot_ptr->shndx);
        kprintf("MMap: %x\n", mboot_ptr->mmap_length);
        kprintf("Addr: %x\t", mboot_ptr->mmap_addr);
        kprintf("Drives: %x\t", mboot_ptr->drives_length);
        kprintf("Addr: %x\n", mboot_ptr->drives_addr);
        kprintf("Config: %x\t", mboot_ptr->config_table);
        kprintf("Loader: %x\t", mboot_ptr->boot_loader_name);
        kprintf("APM: %x\n", mboot_ptr->apm_table);
        kprintf("VBE Control: %x\t", mboot_ptr->vbe_control_info);
        kprintf("VBE Mode Info: %x\t", mboot_ptr->vbe_mode_info);
        kprintf("VBE Mode: %x\n", mboot_ptr->vbe_mode);
        kprintf("VBE seg: %x\t", mboot_ptr->vbe_interface_seg);
        kprintf("VBE off: %x\t", mboot_ptr->vbe_interface_off);
        kprintf("VBE len: %x\n", mboot_ptr->vbe_interface_len);
        resettextcolor();
        kprintf("(End multiboot raw data)\n");
        kprintf("Started with: %s\n", (char *)mboot_ptr->cmdline);
        kprintf("Booted from: %s\n", (char *)mboot_ptr->boot_loader_name);
        settextcolor(7,0);
        kprintf("Testing colors...\n");
        resettextcolor();
        int i;
        for (i = 0; i < 16; ++i) {
        	settextcolor(i,i);
        	putch(' ');
        }
        putch('\n');
        resettextcolor();        
	kprintf("%dkB lower memory\n", mboot_ptr->mem_lower);
        kprintf("%dkB higher memory ", mboot_ptr->mem_upper);
        int mem_mb = mboot_ptr->mem_upper / 1024;
        kprintf("(%dMB)\n", mem_mb);
        
        //for(;;);
        return 0;
}
