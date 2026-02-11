#include "kernel/gdt.h"

#include "libc/stdio.h"


// NOTE: These don't need to be public so we just declare them here

// from https://pdos.csail.mit.edu/6.828/2004/readings/i386/s05_01.htm
typedef struct __attribute__((packed)) {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} gdt_entry_t;

// Struct pointing to Global Descriptor Table, basically just the start of it, and the size.
typedef struct __attribute__((packed)) {
  uint16_t limit;
  uint32_t base;
} gdt_ptr_t;

///
/// @param num descriptor number
/// @param base Descriptor base. Location of segment
/// @param limit Descriptor limit. Size of the segment.
/// @param access Access bit, a bit that is set when the segment is accessed. For permissions
/// @param flags Which units should LIMIT be interpreted. For instance for 0xCF sets units to
/// 4 KB.
void gdt_set_entry(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

// defined in gdt_routines.asm
extern void gdt_set_gdt(gdt_ptr_t* ptr);
extern void gdt_reload_segments();


// NOTE: https://wiki.osdev.org/GDT_Tutorial

gdt_entry_t gdt[GDT_ENTRIES];
gdt_ptr_t gdt_ptr;


bool init_gdt() {
  // set gdt limit
  printf("Initialising Global Descriptor Table with %d entries...\n", GDT_ENTRIES);
  gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_ENTRIES - 1;
  gdt_ptr.base = (uint32_t)gdt;

  // TODO: maybe just hard code the gdt rather than computing it.
  // Values taken from https://wiki.osdev.org/GDT_Tutorial
  gdt_set_entry(0, 0, 0, 0, 0);             // Null Descriptor
  gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF); // Kernel mode Code Segment
  gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF); // Kernel Mode Data Segment
  gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xCF); // User Mode Code Segment
  gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xCF); // User Mode Data Segment

  // TODO: We probably want this
  // https://wiki.osdev.org/Task_State_Segment Hardware task switching or stack switching during
  // For user mode -> kernel mode
  // privilege transitions gdt_set_entry(5, &TSS, sizeof(TSS) - 1, 0x89, 0x0); // Task State
  // Segment


  // load gdt
  gdt_set_gdt(&gdt_ptr);
  gdt_reload_segments();

  // quick sanity test for gdt
  uint32_t* ptr = (uint32_t*)0x100000;
  *ptr = 0xCAFEBABE;

  if (*ptr != 0xCAFEBABE) {
    fprintf(stdout, "GDT: Something went wrong, got 0x%x but expected 0x%x\n", *ptr, 0xCAFEBABE);
    return false;
  }

  printf("Successfully Initialised Global Descriptor Table.\n");
  return true;
}


void gdt_set_entry(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
  gdt[num].base_low = base & 0xFFFF;
  gdt[num].base_middle = base >> 16 & 0xFF;
  gdt[num].base_high = base >> 24 & 0xFF;

  gdt[num].limit_low = limit & 0xFFFF;

  // Flags (granularity byte) = (Limit High 4 bits) | (Flags High 4 bits)
  gdt[num].granularity = limit >> 16 & 0x0F;
  gdt[num].granularity |= flags & 0xF0;

  gdt[num].access = access;
}
