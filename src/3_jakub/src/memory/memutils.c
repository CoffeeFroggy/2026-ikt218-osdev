#include "kernel/memory.h"

void *memcpy(void *dest, const void *src, size_t count)
{
    uint8_t *dst8 = (uint8_t *)dest;
    const uint8_t *src8 = (const uint8_t *)src;

    while (count--) {
        *dst8++ = *src8++;
    }

    return dest;
}

void *memset16(void *ptr, uint16_t value, size_t num)
{
    uint16_t *p = (uint16_t *)ptr;

    while (num--) {
        *p++ = value;
    }

    return ptr;
}

void *memset(void *ptr, int value, size_t num)
{
    uint8_t *p = (uint8_t *)ptr;

    while (num--) {
        *p++ = (uint8_t)value;
    }

    return ptr;
}
