#include "interrupts/keyboard.h"
#include "interrupts/isr.h"
#include "common.h"
#include "kernel/heap.h"
#include "kernel/pit.h"
#include "apps/song/song.h"
#include "apps/raycaster/raycaster.h"
#include "libc/stdio.h"

// Scancode to ASCII lookup table (US QWERTY layout)
const char kbdUS[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t',     /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0,      /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
 '\'', '`',   0,        /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',      /* 49 */
  'm', ',', '.', '/',   0,              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

const char kbdUS_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
  '(', ')', '_', '+', '\b', /* Backspace */
  '\t',     /* Tab */
  'Q', 'W', 'E', 'R',   /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
    0,      /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
 '"', '~',   0,        /* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',      /* 49 */
  'M', '<', '>', '?',   0,              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

#define KBD_BUFFER_SIZE 256
#define HISTORY_CAPACITY 8

char keyboard_buffer[KBD_BUFFER_SIZE];
int buffer_index = 0;
int shift_pressed = 0;
static int extended_scancode = 0;
static char *history_entries[HISTORY_CAPACITY];
static int history_count = 0;
static int history_start = 0;
static int history_total = 0;
static volatile char last_key_pressed = 0;

static void print_prompt(void)
{
    printf("> ");
}

static size_t keyboard_strlen(const char *text)
{
    size_t length = 0;

    while (text[length] != '\0') {
        length++;
    }

    return length;
}

static int keyboard_streq(const char *left, const char *right)
{
    size_t index = 0;

    while (left[index] != '\0' && right[index] != '\0') {
        if (left[index] != right[index]) {
            return 0;
        }

        index++;
    }

    return left[index] == right[index];
}

static int keyboard_startswith(const char *text, const char *prefix)
{
    size_t index = 0;

    while (prefix[index] != '\0') {
        if (text[index] != prefix[index]) {
            return 0;
        }

        index++;
    }

    return 1;
}

static const char *skip_spaces(const char *text)
{
    while (*text == ' ') {
        text++;
    }

    return text;
}

static void clear_history(void)
{
    int i;

    for (i = 0; i < history_count; i++) {
        int slot = (history_start + i) % HISTORY_CAPACITY;

        free(history_entries[slot]);
        history_entries[slot] = 0;
    }

    history_count = 0;
    history_start = 0;
}

static void print_history(void)
{
    int i;

    if (history_count == 0) {
        printf("History is empty.\n");
        return;
    }

    printf("Saved entries:\n");
    for (i = 0; i < history_count; i++) {
        int slot = (history_start + i) % HISTORY_CAPACITY;

        printf("%d: %s\n", i + 1, history_entries[slot]);
    }
}

static void print_help(void)
{
    printf("Commands:\n");
    printf("help         Show this help screen\n");
    printf("clear        Clear the display\n");
    printf("meminfo      Show heap and page memory info\n");
    printf("history      Show saved command history\n");
    printf("clearhistory Free saved history entries\n");
    printf("ticks        Show current PIT tick count\n");
    printf("uptime       Show uptime in milliseconds\n");
    printf("music <idx>  Play the song with index (0-8)\n");
    printf("game         Play the ASCII raycaster FPS\n");
    printf("echo <text>  Print text back to the screen\n");
    printf("about        Show kernel feature summary\n");
    printf("Keyboard:\n");
    printf("ESC          Stop playing music or exit game\n");
    printf("PgUp/PgDn    Scroll terminal history by pages\n");
    printf("Up/Down      Scroll terminal history line by line\n");
    printf("Home/End     Jump to top or bottom of scrollback\n");
}

static void print_about(void)
{
    terminal_print_logo();
    printf("Interrupts, paging, heap, PIT, keyboard history, scrollback\n");
    printf("History entries are stored on the heap.\n");
}

static void save_history_entry(const char *line)
{
    size_t length = keyboard_strlen(line);
    char *entry;
    int slot;
    size_t i;

    if (length == 0) {
        return;
    }

    entry = (char *)malloc(length + 1);
    if (entry == 0) {
        printf("History allocation failed.\n");
        return;
    }

    for (i = 0; i <= length; i++) {
        entry[i] = line[i];
    }

    if (history_count == HISTORY_CAPACITY) {
        slot = history_start;
        free(history_entries[slot]);
        history_start = (history_start + 1) % HISTORY_CAPACITY;
    } else {
        slot = (history_start + history_count) % HISTORY_CAPACITY;
        history_count++;
    }

    history_entries[slot] = entry;
    history_total++;
}

static void execute_command(const char *command)
{
    const char *argument;

    if (keyboard_streq(command, "help")) {
        print_help();
        return;
    }

    if (keyboard_streq(command, "clear")) {
        terminal_initialize();
        return;
    }

    if (keyboard_streq(command, "meminfo")) {
        print_memory_layout();
        printf("History entries: %d of %d\n", history_count, HISTORY_CAPACITY);
        printf("Ticks: %d\n", (int)pit_get_ticks());
        return;
    }

    if (keyboard_streq(command, "history")) {
        print_history();
        return;
    }

    if (keyboard_streq(command, "clearhistory")) {
        clear_history();
        printf("History cleared.\n");
        return;
    }

    if (keyboard_streq(command, "ticks")) {
        printf("Ticks: %d\n", (int)pit_get_ticks());
        return;
    }

    if (keyboard_streq(command, "uptime")) {
        printf("Uptime: %d ms\n", (int)pit_get_ticks());
        return;
    }

    if (keyboard_streq(command, "about")) {
        print_about();
        return;
    }

    if (keyboard_startswith(command, "music") || keyboard_startswith(command, "music")) {
        const char *arg = keyboard_startswith(command, "music")
            ? skip_spaces(command + 5)
            : skip_spaces(command + 5);
        if (*arg == '\0') {
            printf("Usage: music <song_index>\nAvailable songs: 0-8\n");
            return;
        }
        int song_index = 0;
        while (*arg >= '0' && *arg <= '9') {
            song_index = song_index * 10 + (*arg - '0');
            arg++;
        }
        if (*arg != '\0') {
            printf("Invalid song index: %s\n", skip_spaces(command + 5));
            return;
        }
        play_music(song_index);
        return;
    }

    if (keyboard_startswith(command, "echo")) {
        argument = skip_spaces(command + 4);
        printf("%s\n", argument);
        return;
    }

    if (keyboard_streq(command, "game")) {
        raycaster_input_request_launch();
        printf("Starting game...\n");
        return;
    }

    printf("Unknown command: %s\n", command);
    printf("Type help to see available commands.\n");
}

static void handle_enter_key(void)
{
    printf("\n");

    if (keyboard_buffer[0] == '\0') {
        print_prompt();
        return;
    }

    save_history_entry(keyboard_buffer);

    execute_command(keyboard_buffer);

    buffer_index = 0;
    keyboard_buffer[0] = '\0';
    print_prompt();
}

static void handle_extended_scancode(uint8_t scancode)
{
    uint8_t released = scancode & 0x80;
    uint8_t code = scancode & 0x7F;

    if (released) {
        return;
    }

    if (raycaster_input_is_active()) {
        return;
    }

    if (code == 0x49) {
        terminal_scroll_page_up();
        return;
    }

    if (code == 0x51) {
        terminal_scroll_page_down();
        return;
    }

    if (code == 0x48) {
        terminal_scroll_line_up();
        return;
    }

    if (code == 0x50) {
        terminal_scroll_line_down();
        return;
    }

    if (code == 0x47) {
        terminal_scroll_to_top();
        return;
    }

    if (code == 0x4F) {
        terminal_scroll_to_bottom();
    }
}

static void keyboard_callback(registers_t *regs) {
    (void)regs;
    // The PIC leaves us the scancode in port 0x60
    uint8_t scancode = inb(0x60);

    if (raycaster_input_is_active()) {
        raycaster_input_submit_scancode(scancode);
    }

    if (scancode == 0xE0) {
        extended_scancode = 1;
        return;
    }

    if (extended_scancode) {
        extended_scancode = 0;
        handle_extended_scancode(scancode);
        return;
    }

    // Top bit set means key released
    if (scancode & 0x80) {
        // Key release
        uint8_t released_key = scancode & ~0x80;
        if (released_key == 0x2A || released_key == 0x36) { // Left shift (42) or Right shift (54)
            shift_pressed = 0;
        }

    } else {
        // Key press
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
        } else {
            char ascii = shift_pressed ? kbdUS_shift[scancode] : kbdUS[scancode];
            
            // ESC key - always handle specially
            if (scancode == 0x01) {
                ascii = 27;  // ESC code
                last_key_pressed = ascii;
                raycaster_input_request_exit();
                if (!raycaster_input_is_active()) {
                    stop_music();  // Stop music if not in game
                }
                return;
            }
            
            // Always capture the key for games
            last_key_pressed = ascii;

            // In game mode, only capture keys - don't process commands
            if (raycaster_input_is_active()) {
                return;
            }
            
            // Normal command mode - process as commands
            if (ascii == '\b') {
                terminal_scroll_to_bottom();
                if (buffer_index > 0) {
                    buffer_index--;
                    keyboard_buffer[buffer_index] = '\0';
                    printf("\b \b");
                }
            } else if (ascii == '\n') {
                terminal_scroll_to_bottom();
                handle_enter_key();
            } else if (ascii != 0) {
                terminal_scroll_to_bottom();
                // Store in buffer
                if (buffer_index < KBD_BUFFER_SIZE - 1) {
                    keyboard_buffer[buffer_index++] = ascii;
                    keyboard_buffer[buffer_index] = '\0';
                }
                // Print out character
                printf("%c", ascii);
            }
        }
    }
}

void init_keyboard() {
    keyboard_buffer[0] = '\0';
    register_interrupt_handler(IRQ1, keyboard_callback);
}

void keyboard_print_prompt(void)
{
    print_prompt();
}

char keyboard_get_last_key(void)
{
    char key = last_key_pressed;
    last_key_pressed = 0;  // Clear after reading
    return key;
}
