#pragma once

#include <libc/stdbool.h>
#include <stdint.h>

#define GDT_ENTRIES 6

bool init_gdt(void);
void init_tss(void);
void tss_update_esp0(uint32_t esp);
void gdt_set_entry(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdt_reload(void);
