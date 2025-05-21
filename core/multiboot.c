#include "../include/kernel.h"
#include "../include/multiboot.h"

struct multiboot *
copy_multiboot(
        struct multiboot *mboot_ptr
) {
        struct multiboot *new_header = (struct multiboot *)kmalloc(sizeof(struct multiboot));
        memcpy(new_header, mboot_ptr, sizeof(struct multiboot));
        return new_header;
}

void

dump_multiboot(
		struct multiboot *mboot_ptr
		) {
	resettextcolor();
	kprintf("MULTIBOOT header at 0x%x:\n", (uintptr_t)mboot_ptr);
	settextcolor(7,0);
	kprintf("Flags : 0x%x ",  mboot_ptr->flags);
	kprintf("Mem Lo: 0x%x ",  mboot_ptr->mem_lower);
	kprintf("Mem Hi: 0x%x ",  mboot_ptr->mem_upper);
	kprintf("Boot d: 0x%x\n", mboot_ptr->boot_device);
	kprintf("cmdlin: 0x%x ",  mboot_ptr->cmdline);
	kprintf("Mods  : 0x%x ",  mboot_ptr->mods_count);
	kprintf("Addr  : 0x%x ",  mboot_ptr->mods_addr);
	kprintf("Syms  : 0x%x\n", mboot_ptr->num);
	kprintf("Syms  : 0x%x ",  mboot_ptr->size);
	kprintf("Syms  : 0x%x ",  mboot_ptr->addr);
	kprintf("Syms  : 0x%x ",  mboot_ptr->shndx);
	kprintf("MMap  : 0x%x\n", mboot_ptr->mmap_length);
	kprintf("Addr  : 0x%x ",  mboot_ptr->mmap_addr);
	kprintf("Drives: 0x%x ",  mboot_ptr->drives_length);
	kprintf("Addr  : 0x%x ",  mboot_ptr->drives_addr);
	kprintf("Config: 0x%x\n", mboot_ptr->config_table);
	kprintf("Loader: 0x%x ",  mboot_ptr->boot_loader_name);
	kprintf("APM   : 0x%x ",  mboot_ptr->apm_table);
	kprintf("VBE Co: 0x%x ",  mboot_ptr->vbe_control_info);
	kprintf("VBE Mo: 0x%x\n", mboot_ptr->vbe_mode_info);
	kprintf("VBE In: 0x%x ",  mboot_ptr->vbe_mode);
	kprintf("VBE se: 0x%x ",  mboot_ptr->vbe_interface_seg);
	kprintf("VBE of: 0x%x ",  mboot_ptr->vbe_interface_off);
	kprintf("VBE le: 0x%x\n", mboot_ptr->vbe_interface_len);
	resettextcolor();
	kprintf("Started with: %s\n", (char *)mboot_ptr->cmdline);
	kprintf("Booted from: %s\n", (char *)mboot_ptr->boot_loader_name);
	kprintf("%dkB lower memory\n", mboot_ptr->mem_lower);
	kprintf("%dkB higher memory ", mboot_ptr->mem_upper);
	int mem_mb = mboot_ptr->mem_upper / 1024;
	kprintf("(%dMB)\n", mem_mb);
}