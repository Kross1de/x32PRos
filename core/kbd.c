#include "../include/kernel.h"

char kbd[128] = {
	0, 27,
	'1','2','3','4','5','6','7','8','9','0',
	'-','=','\b',
	'\t', /* tab */
	'q','w','e','r','t','y','u','i','o','p','[',']','\n',
	0, /* control */
	'a','s','d','f','g','h','j','k','l',';','\'', '`',
	0, /* left shift */
	'\\','z','x','c','v','b','n','m',',','.','/',
	0, /* right shift */
	'*',
	0, /* alt */
	' ', /* space */
	0, /* caps lock */
	0, /* F1 [59] */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, /* ... F10 */
	0, /* 69 num lock */
	0, /* scroll lock */
	0, /* home */
	0, /* up */
	0, /* page up */
	'-',
	0, /* left arrow */
	0,
	0, /* right arrow */
	'+',
	0, /* 79 end */
	0, /* down */
	0, /* page down */
	0, /* insert */
	0, /* delete */
	0, 0, 0,
	0, /* F11 */
	0, /* F12 */
	0, /* everything else */
};

void
keyboard_handler() {
        unsigned char scancode;
        scancode = inportb(0x60);
        if (scancode & 0x80) {
                /* key up */
        } else {
                if (kbd[scancode] == 'q') {
                        /* Page Fault Time */
                        uint32 *ptr = (uint32 *)0xA0000000;
                        uint32 do_page_fault = *ptr;
                }
                putch(kbd[scancode]);
        }
}

void
keyboardInstall() {
        /* IRQ installer */
        irq_install_handler(1, keyboard_handler);
}

void
keyboard_wait() {
        while(inportb(0x64) & 2);
}
