#include "kernel/memory.h"
#include "libc/stdio.h"

#define PAGE_DIRECTORY_ADDR 0x400000
#define FIRST_PAGE_TABLE_ADDR 0x401000
#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 1024

static uint32_t *page_directory = 0;
static uint32_t page_dir_loc = 0;
static uint32_t *last_page = 0;

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    uint32_t page_directory_index = virt >> 22;
    uint32_t i;

    for (i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        last_page[i] = (phys + (i * PAGE_SIZE)) | 3;
    }

    page_directory[page_directory_index] = ((uint32_t)last_page) | 3;
    last_page = (uint32_t *)((uint32_t)last_page + PAGE_SIZE);
}

static void paging_enable(void)
{
    uint32_t cr0;

    asm volatile("mov %0, %%cr3" : : "r"(page_dir_loc));
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

void init_paging(void)
{
    uint32_t i;

    printf("Setting up paging\n");

    page_directory = (uint32_t *)PAGE_DIRECTORY_ADDR;
    page_dir_loc = (uint32_t)page_directory;
    last_page = (uint32_t *)FIRST_PAGE_TABLE_ADDR;

    memset(page_directory, 0, PAGE_SIZE);
    for (i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        page_directory[i] = 2;
    }

    paging_map_virtual_to_phys(0, 0);
    paging_map_virtual_to_phys(0x400000, 0x400000);
    paging_enable();

    printf("Paging enabled\n");
}
