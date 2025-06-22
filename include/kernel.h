#ifndef KERNEL_H
#define KERNEL_H

/* Types */

#define NULL ((void *)0UL)

typedef unsigned long uintptr_t;
typedef long size_t;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef unsigned long long uint64_t;

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

/* Panic */
#define HALT_AND_CATCH_FIRE(mesg) halt_and_catch_fire(mesg, __FILE__, __LINE__)
#define ASSERT(statement) ((statement) ? (void)0 : assert_failed(__FILE__, __LINE__, #statement))
void halt_and_catch_fire(char * error_message, const char * file, int line);
void assert_failed(const char *file, uint32 line, const char *desc);

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
extern void isrs_install_handler(int isrs, irq_handler_t);
extern void isrs_uninstall_handler(int isrs);

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

/* Memory Management */
extern uintptr_t placement_pointer;
extern void kmalloc_startat(uintptr_t address);
extern uintptr_t kmalloc_real(size_t size, int align, uintptr_t * phys);
extern uintptr_t kmalloc(size_t size);
extern uintptr_t kvmalloc(size_t size);
extern uintptr_t kmalloc_p(size_t size, uintptr_t * phys);
extern uintptr_t kvmalloc_p(size_t size, uintptr_t * phys);
extern void heap_install();
extern void *sbrk(uintptr_t increment);
extern void free(void *ptr);

typedef struct page {
	uint32 present : 1;
	uint32 rw      : 1;
	uint32 user    : 1;
	uint32 accessed: 1;
	uint32 dirty   : 1;
	uint32 unused  : 7;
	uint32 frame   : 20;
} page_t;

typedef struct page_table {
	page_t pages[1024];
} page_table_t;

typedef struct page_directory {
	page_table_t *tables[1024]; /* 1024 pointers to page tables... */
	uintptr_t physical_tables[1024]; /* Physical addresses of the tables */
	uintptr_t physical_address; /* The physical address of physical_tables */
} page_directory_t;

extern page_directory_t * kernel_directory;
extern page_directory_t * current_directory;

extern void paging_install(uint32 memsize);
extern void switch_page_directory(page_directory_t *new);
extern page_t *get_page(uintptr_t address, int make, page_directory_t *dir);
extern void page_fault(struct regs *r);

#endif
