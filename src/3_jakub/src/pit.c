#include "kernel/pit.h"
#include "common.h"
#include "interrupts/isr.h"

static volatile uint32_t pit_ticks = 0;

static void pit_callback(registers_t *regs)
{
    (void)regs;
    pit_ticks++;
}

void init_pit(void)
{
    uint16_t divisor = (uint16_t)DIVIDER;

    register_interrupt_handler(IRQ0, pit_callback);

    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t pit_get_ticks(void)
{
    return pit_ticks;
}

void sleep_busy(uint32_t milliseconds)
{
    uint32_t start_ticks;
    uint32_t ticks_to_wait;

    if (milliseconds == 0) {
        return;
    }

    start_ticks = pit_get_ticks();
    ticks_to_wait = milliseconds * TICKS_PER_MS;

    while ((pit_get_ticks() - start_ticks) < ticks_to_wait) {
    }
}

void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t start_ticks;
    uint32_t ticks_to_wait;

    if (milliseconds == 0) {
        return;
    }

    start_ticks = pit_get_ticks();
    ticks_to_wait = milliseconds * TICKS_PER_MS;

    while ((pit_get_ticks() - start_ticks) < ticks_to_wait) {
        asm volatile("sti\nhlt");
    }
}
