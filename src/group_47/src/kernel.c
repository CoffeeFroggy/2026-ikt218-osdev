#include "libc/stdint.h"
#include "terminal.h"
#include "gdt.h"

void kmain(uint32_t magic, void* mb_info_addr) {
    (void)magic;
    (void)mb_info_addr;

    init_gdt();

    terminal_initialize();
    printf("Hello World\n");

    while (1) {
        __asm__ __volatile__("hlt");
    }
}