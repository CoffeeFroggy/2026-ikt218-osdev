#include "libc/stdint.h"
#include "libc/stddef.h"
// This file contains the implementation of the terminal output functions, which allow us to print text to the screen using the VGA text mode.
// VGA screen is 80 columns wide and 25 rows high
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
// Defining the variables
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize() { // Initialize the terminal screen by setting the cursor to the top-left corner, setting the default color, and clearing the screen.
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x0F; // White text on black background
    terminal_buffer = (uint16_t*) 0xB8000;
    
    // Clear the screen
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = (uint16_t) ' ' | (uint16_t) terminal_color << 8;
        }
    }
}

void terminal_putchar(char c) { // Handle newlines and carriage returns
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else {
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = (uint16_t) c | (uint16_t) terminal_color << 8;
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) terminal_row = 0;
        }
    }
}

void printf(const char* data) { // Simple implementation of printf that only supports strings (no formatting)
    for (size_t i = 0; data[i] != '\0'; i++)
        terminal_putchar(data[i]);
}