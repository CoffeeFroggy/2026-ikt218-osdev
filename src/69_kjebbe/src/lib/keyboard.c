#include "../../include/keyboard.h"
#include "../../include/io.h"
#include "../../include/irq.h"
#include "../../include/libc/stdint.h"
#include "../../include/libc/stdio.h"

// Maps scancode to character. keyboard Layout: US QWERTY
static const char scancode_ascii[128] = {
    0,   0,    '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   '7', '8', '9', '-', '4', '5', '6',
    '+', '1',  '2',  '3', '0',  '.', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,   0,
};

// handler for IRQ1 (keyboard)
static void keyboard_handler(registers_t *regs) {
  (void)regs;

  uint8_t scancode =
      inb(KEYBOARD_DATA_PORT); // Read scancode from keyboard data port

  // If Bit 7 is set. That means the key is released.
  // We ignore key releases with an early return.
  if (scancode & 0x80)
    return;

  char c = scancode_ascii[scancode];
  if (c) {
    printf("%c", c);
  }
}

// Maps IRQ 1 to the pointer of the keyboard handler function.
void keyboard_init(void) { irq_register_handler(1, keyboard_handler); }
