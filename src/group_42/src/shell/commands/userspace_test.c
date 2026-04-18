#include <stdint.h>

void _start(void) {
    const char* msg = "Hello from userspace!\n";
    volatile uint8_t* video = (volatile uint8_t*)0xB8000;
    for (int i = 0; msg[i] != '\0'; i++) {
        video[i * 2] = msg[i];
        video[i * 2 + 1] = 0x07;
    }
    for (;;) { __asm__ volatile("hlt"); }
}