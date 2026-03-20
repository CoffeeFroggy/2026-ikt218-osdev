#include "../../include/kernel/pit.h"
#include "../../include/irq.h"
#include "../../include/libc/stdio.h"

// Counts how many PIT ticks have occurred since init_pit() was called.
static volatile uint32_t tick = 0;

// Returns the current tick count.
static uint32_t get_current_tick()
{
    return tick;
}

// IRQ0 handler — called by the IRQ system on every timer interrupt.
static void pit_irq_handler(registers_t* regs)
{
    tick++;
}

// Initialise the PIT to fire at TARGET_FREQUENCY (1000 Hz = 1 tick per ms).
void init_pit()
{
    // Register our handler on IRQ0 (the timer line).
    irq_register_handler(0, pit_irq_handler);

    // Command byte 0x36: channel 0, lobyte/hibyte access, mode 3 (square wave).
    uint32_t divisor = DIVIDER;
    asm volatile("outb %0, %1" : : "a"((uint8_t)0x36),                    "Nd"((uint16_t)PIT_CMD_PORT));
    asm volatile("outb %0, %1" : : "a"((uint8_t)(divisor & 0xFF)),        "Nd"((uint16_t)PIT_CHANNEL0_PORT));
    asm volatile("outb %0, %1" : : "a"((uint8_t)((divisor >> 8) & 0xFF)), "Nd"((uint16_t)PIT_CHANNEL0_PORT));

    printf("PIT initialised at %d Hz\n", TARGET_FREQUENCY);
}

// Busy-wait sleep: spins the CPU until enough ticks have passed.
// High CPU usage — the CPU does nothing useful while waiting.
void sleep_busy(uint32_t milliseconds)
{
    uint32_t start_tick    = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    while (elapsed_ticks < ticks_to_wait)
    {
        while (get_current_tick() == start_tick + elapsed_ticks) {}
        elapsed_ticks++;
    }
}

// Interrupt-driven sleep: halts the CPU between ticks.
// Low CPU usage — the CPU sleeps until the next interrupt wakes it.
void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t current_tick  = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end_ticks     = current_tick + ticks_to_wait;

    while (current_tick < end_ticks)
    {
        // Enable interrupts and halt until the next one fires (the PIT tick).
        asm volatile("sti");
        asm volatile("hlt");
        current_tick = get_current_tick();
    }
}
