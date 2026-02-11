#include "drivers/video/vga_terminal.h"

#include "libc/stdint.h"
#include "libc/string.h"

/// Packs colour and char together for printing
/// @param uc character
/// @param color colour
/// @return packed unsigned 16 bit int, ready to be pritned
static uint16_t vga_entry(char uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

static void terminal_scroll(void);


size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;


void vga_terminal_initialise(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

  // Fill entire VGA with blank entries with correct colours
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }
}


void vga_terminal_setcolor(uint8_t color) {
  terminal_color = color;
}


void vga_terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = vga_entry(c, color);
}


void vga_terminal_putchar(char c) {
  if (c == '\n') {
    terminal_column = 0;
    terminal_row++;

    if (terminal_row == VGA_HEIGHT) {
      terminal_scroll();
      terminal_row = VGA_HEIGHT - 1;
    }
    return;
  }

  const size_t index = terminal_row * VGA_WIDTH + terminal_column;
  terminal_buffer[index] = vga_entry(c, terminal_color);

  terminal_column++;

  if (terminal_column == VGA_WIDTH) {
    terminal_column = 0;
    terminal_row++;

    if (terminal_row == VGA_HEIGHT) {
      terminal_scroll();
      terminal_row = VGA_HEIGHT - 1;
    }
  }
}
/// Writes the supplied data interpreted as chars
/// @param data
/// @param size
void vga_terminal_write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++)
    vga_terminal_putchar(data[i]);
}
/// Writes a string to terminal
/// @param data
void vga_terminal_writestring(const char* data) {
  vga_terminal_write(data, strlen(data));
}

/// Scrolls terminal
static void terminal_scroll(void) {
  for (size_t y = 1; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      size_t dest_index = (y - 1) * VGA_WIDTH + x;
      size_t src_index = y * VGA_WIDTH + x;
      terminal_buffer[dest_index] = terminal_buffer[src_index];
    }
  }

  size_t last_row_start = (VGA_HEIGHT - 1) * VGA_WIDTH;
  for (size_t x = 0; x < VGA_WIDTH; x++) {
    terminal_buffer[last_row_start + x] = vga_entry(' ', terminal_color);
  }
}
