#include <kernel/log.h>
#include <kernel/panic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arch/i386/cpu/isr.h"
#include "kernel/paging.h"
#include "kernel/pmm.h"

static uint32_t user_heap_start = 0x10000;
static uint32_t user_heap_end = 0x08000000;

uint32_t get_user_heap_end(void) {
  return user_heap_end;
}

void set_user_heap_end(uint32_t addr) {
  if (addr < user_heap_start) {
    addr = user_heap_start;
  }
  if (addr > 0x08000000) {
    addr = 0x08000000;
  }
  user_heap_end = addr;
}

void page_fault_handler(registers_t* regs) {
  uint32_t fault_addr;
  __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));

  uint32_t err = regs->err_code;
  int present = (err >> 0) & 1;
  int write = (err >> 1) & 1;
  int user = (err >> 2) & 1;

  uint32_t eip = regs->eip;
  (void)eip;
  (void)write;

  log_info("PF: addr=0x%x, eip=0x%x, present=%d, write=%d, user=%d\n", fault_addr, eip, present, write, user);

  if (!present) {
    if (fault_addr < 0x1000) {
      log_info("PF: null pointer\n");
      for (;;) { __asm__ volatile("hlt"); }
    }

    if (!user && fault_addr >= 0x100000 && fault_addr < 0x400000) {
      uint32_t page_start = fault_addr & ~0xFFF;
      vmm_map_page(page_start, page_start, PAGE_KERNEL_RW);
      log_info("PF: kernel page mapped\n");
      return;
    }

    if (user && fault_addr < 0x40000000) {
      log_info("PF: user page fault, allocating...\n");
      uint32_t page_start = fault_addr & ~0xFFF;
      uint32_t phys = pmm_alloc_frame();
      log_info("PF: allocated frame 0x%x\n", phys);
      if (phys && phys >= 0x100000) {
        memset((void*)phys, 0, 4096);
        vmm_map_user_page(page_start, phys, PAGE_USER_RW);
        log_info("PF: user page mapped\n");
        return;
      }
    }
  }

  log_info("PF: unhandled fault!\n");
  for (;;) { __asm__ volatile("hlt"); }
}
