// main.c
// Main file
/*
 * This file contains the kernel entry point 
 * and also has functions that are used in other files.
 */

#include "include/kernel.h"
#include "include/multiboot.h"

page_directory_t *kernel_directory;
page_directory_t *current_directory;

/*
 * kernel entry point
 */
int
main(struct multiboot *mboot_ptr) {
        if (mboot_ptr->mods_count > 0) {
                kmalloc_startat(((uintptr_t *)mboot_ptr->mods_addr)[1]);
        }
        mboot_ptr = copy_multiboot(mboot_ptr);
        gdt_install();  /* Global descriptor table */
        idt_install();  /* IDT */
        isrs_install(); /* Interrupt service requests */
        irq_install();  /* Hardware interrupt requests */
        init_video();   /* VGA driver */
        timer_install();
        keyboardInstall();
        paging_install(mboot_ptr->mem_upper);
        heap_install();

        settextcolor(12,0);
        kprintf("[%s %s]\n", KERNEL_UNAME, KERNEL_VERSION);
        dump_multiboot(mboot_ptr);

        return 0;
}
