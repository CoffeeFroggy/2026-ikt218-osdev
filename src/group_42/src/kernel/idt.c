#include "idt.h"

#include <libc/stddef.h>

// NOTE: Not implemented
// TODO: Implement ;)
int_handler_t int_handlers[IDT_ENTRIES];
idt_entry_t idt[IDT_ENTRIES];
idt_ptr_t idt_ptr;

void register_int_handler(int num, void (*handler)(void* data), void* data) {
  int_handlers[num].num = num;
  int_handlers[num].handler = handler;
  int_handlers[num].data = data;
}

void default_int_handler(void* data) {
  // handle interrupt
}

void int_handler(int num) {
  if (int_handlers[num].handler != NULL) {
    int_handlers[num].handler(int_handlers[num].data);
  } else {
    default_int_handler(NULL);
  }
}

void init_idt() {
  // set idt limit
  idt_ptr.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
  idt_ptr.base = (uint32_t)&idt;

  for (int i = 0; i < IDT_ENTRIES; i++) {
    idt[i].base_low = 0x0000;
    idt[i].base_high = 0x0000;
    idt[i].selector = 0x08; // check this is correct
    idt[i].zero = 0x00;
    idt[i].flags = 0x8E;

    int_handlers[i].handler = NULL;
  }

  idt_load(&idt_ptr);
}

void idt_load(idt_ptr_t* ptr) {
  asm volatile("lidt %0" : : "m"(*ptr));
}
