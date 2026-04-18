#pragma once
#include <stdint.h>

#define PAGE_PRESENT   0x001
#define PAGE_RW        0x002
#define PAGE_USER      0x004
#define PAGE_ACCESSED  0x020
#define PAGE_DIRTY     0x040

#define PAGE_KERNEL_RW  (PAGE_PRESENT | PAGE_RW)
#define PAGE_KERNEL_RO  (PAGE_PRESENT)
#define PAGE_USER_RW    (PAGE_PRESENT | PAGE_RW | PAGE_USER)
#define PAGE_USER_RO    (PAGE_PRESENT | PAGE_USER)

#define KERNEL_VIRT_BASE  0xC0000000
#define USER_VIRT_BASE    0x00000000
#define USER_VIRT_TOP     0xBFFFFFFF
#define KERNEL_HEAP_VIRT  0xC0100000

#define PGD_INDEX(addr)  (((addr) >> 22) & 0x3FF)
#define PTE_INDEX(addr)  (((addr) >> 12) & 0x3FF)

void init_paging(void* mb_info);
void paging_enable(void);
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);
uint32_t paging_get_phys(uint32_t virt);
int vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags);
int vmm_unmap_page(uint32_t virt);
int vmm_map_user_page(uint32_t virt, uint32_t phys, uint32_t flags);

void load_cr3(uint32_t addr);
void enable_paging(void);
void invalidate_page(uint32_t addr);
