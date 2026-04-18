#include "arch/i386/cpu/gdt.h"

#include <kernel/log.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct __attribute__((packed)) {
    uint16_t prev_task_link;
    uint16_t reserved0;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved1;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved2;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved3;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint16_t es, cs, ss, ds, fs, gs, ldtr, reserved4;
    uint16_t iopb_offset;
    uint32_t esp3;
    uint16_t ss3;
    uint16_t reserved5;
} tss_t;

static tss_t tss;
static uint8_t kernel_stack[8192];

extern void tss_flush(void);

void init_tss(void) {
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = 0x10;
    tss.esp0 = (uint32_t)(kernel_stack + sizeof(kernel_stack));
    tss.cs = 0x08;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10;

    gdt_set_entry(5, (uint32_t)&tss, sizeof(tss) - 1, 0x89, 0x00);

    gdt_reload();

    tss_flush();

    log_info("TSS initialized, kernel stack at 0x%x\n", tss.esp0);
}

void tss_update_esp0(uint32_t esp) {
    tss.esp0 = esp;
}
