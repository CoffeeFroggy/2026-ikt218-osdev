#include "kernel/heap.h"
#include "libc/libs.h"
#include "apps/raycaster/raycaster.h"
#include "interrupts/keyboard.h"

struct allocation_probe_t {
    int left;
    int right;
};

void *operator new(size_t size)
{
    return malloc(size);
}

void *operator new[](size_t size)
{
    return malloc(size);
}

void operator delete(void *ptr) noexcept
{
    free(ptr);
}

void operator delete[](void *ptr) noexcept
{
    free(ptr);
}

void operator delete(void *ptr, size_t) noexcept
{
    free(ptr);
}

void operator delete[](void *ptr, size_t) noexcept
{
    free(ptr);
}

extern "C" int kernel_main(void);

int kernel_main()
{
    // Keep kernel alive and execute deferred commands outside IRQ context.
    while (1) {
        if (raycaster_input_consume_launch_request()) {
            raycaster_game_loop();
            terminal_refresh();
            printf("\nExited game. Back to terminal.\n");
            keyboard_print_prompt();
        }

        asm volatile("hlt");
    }

    return 0;
}
