#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <kernel/user_mode.h>
#include <kernel/paging.h>
#include <kernel/pmm.h>

extern const uint8_t user_program[];
extern const uint32_t user_program_size;

int run_userspace_handler(int argc, char** argv) {
    (void)argc;
    (void)argv;

    uint32_t program_start = (uint32_t)user_program;
    uint32_t program_size = (uint32_t)user_program_size;

    printf("Loading user program at 0x%x (size %d bytes)\n", program_start, program_size);

    uint32_t user_vaddr = 0x08000000;
    uint32_t stack_vaddr = 0x08040000;
    uint32_t pages_needed = (program_size + 4095) / 4096;

    printf("Mapping %d pages at 0x%x\n", pages_needed, user_vaddr);

    printf("Allocating user pages...\n");
    for (uint32_t page = user_vaddr; page < user_vaddr + pages_needed * 4096; page += 4096) {
        printf("  Mapping page at 0x%x\n", page);
        uint32_t phys = pmm_alloc_frame();
        if (!phys) {
            printf("Failed to allocate frame\n");
            return -1;
        }
        printf("  Allocated frame at 0x%x\n", phys);
        memset((void*)phys, 0, 4096);
        vmm_map_user_page(page, phys, PAGE_USER_RW);
        printf("  Page mapped\n");
    }
    printf("User pages ready\n");

    memcpy((void*)user_vaddr, (void*)program_start, program_size);

    uint32_t stack_top = stack_vaddr;
    uint32_t stack_pages = 4;
    printf("Setting up stack at 0x%x\n", stack_top);
    for (uint32_t page = stack_top - (stack_pages * 4096); page < stack_top; page += 4096) {
        uint32_t phys = pmm_alloc_frame();
        if (!phys) {
            printf("Failed to allocate stack frame\n");
            return -1;
        }
        memset((void*)phys, 0, 4096);
        vmm_map_user_page(page, phys, PAGE_USER_RW);
    }

    printf("About to call switch_to_user_mode(0x%x, 0x%x)\n", user_vaddr, stack_top);
    __asm__ volatile("cli");  // Disable interrupts before switch
    switch_to_user_mode(user_vaddr, stack_top);
    printf("ERROR: Returned from user mode!\n");

    return 0;
}