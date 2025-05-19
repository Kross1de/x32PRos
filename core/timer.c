#include "../include/kernel.h"

void
timer_phase(
        int hz
) {
        int divisor = 1193180 / hz;
        outportb(0x43, 0x36);
        outportb(0x40, divisor & 0xFF);
        outportb(0x40, (divisor >> 8) & 0xFF);
}

long timer_ticks = 0;
unsigned long ticker = 0;

void
timer_handler() {
        ++timer_ticks;
        if (timer_ticks % 18 == 0) {
                ++ticker;
        }
}

void timer_install() {
        irq_install_handler(0, timer_handler);
        timer_phase(100); /* 100Hz */
}

void
timer_wait(
        int ticks
) {
        long eticks;
        eticks = (long)timer_ticks + (long)ticks;
        while(timer_ticks < eticks);
}
