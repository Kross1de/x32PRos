#ifndef KERNEL_H
#define KERNEL_H

/* Types */

#define NULL ((void *)0UL)

typedef unsigned long uintptr_t;
typedef long size_t;
typedef unsigned int uint32;

/* Unimportant Kernel Strings */
#define KERNEL_UNAME "PRos"
#define KERNEL_VERSION "0.0.1"

/* kernel main */
extern void *memcpy(void * restrict dest, const void * restrict src, size_t count);
extern unsigned short *memcpyw(unsigned short *dest, const unsigned short *src, int count);
extern void *memset(void *dest, int val, size_t count);
extern unsigned short *memsetw(unsigned short *dest, unsigned short val, int count);
extern int strlen(const char *str);
extern unsigned char inportb(unsigned short port);
extern void outportb(unsigned short port, unsigned char data);

/* vga driver */
extern void cls();
extern void putch(unsigned char c);
extern void puts(char *str);
extern void settextcolor(unsigned char forecolor, unsigned char backcolor);
extern void resettextcolor();
extern void init_video();

/* GDT */
extern void gdt_install();
extern void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);

/* IDT */
extern void idt_install();
extern void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);

/* Registers */
struct regs {
          unsigned int gs, fs, es, ds;
          unsigned int edi, esi, ebp, esp,ebx, edx, ecx, eax;
          unsigned int int_no, err_code;
          unsigned int eip, cs, eflags, useresp, ss;
};

typedef void (*irq_handler_t)(struct regs *);

/* ISRS */
extern void isrs_install();

/* Interrupt Handlers */
extern void irq_install();
extern void irq_install_handler(int irq, irq_handler_t);
extern void irq_uninstall_handler(int irq);

/* Timer */
extern void timer_install();
extern long timer_ticks;
extern void timer_wait(int ticks);

/* Keyboard */
extern void keyboardInstall();
extern void keyboard_wait();

/* kprintf */
extern void kprintf(const char *fmt, ...);

#endif
