#include "apps/raycaster/raycaster_internal.h"
#include "interrupts/keyboard.h"

#define GAME_SCANCODE_QUEUE_SIZE 64

static volatile uint8_t g_game_scancode_queue[GAME_SCANCODE_QUEUE_SIZE];
static volatile uint8_t g_game_scancode_head = 0;
static volatile uint8_t g_game_scancode_tail = 0;
static volatile uint8_t g_game_key_state = 0;
static volatile uint8_t g_game_launch_requested = 0;
static volatile uint8_t g_game_exit_requested = 0;
static volatile uint8_t g_game_extended_scancode = 0;
static volatile int g_game_input_active = 0;

static void raycaster_input_enqueue_scancode(uint8_t scancode)
{
    uint8_t next = (uint8_t)((g_game_scancode_head + 1) % GAME_SCANCODE_QUEUE_SIZE);
    if (next == g_game_scancode_tail) {
        g_game_scancode_tail = (uint8_t)((g_game_scancode_tail + 1) % GAME_SCANCODE_QUEUE_SIZE);
    }
    g_game_scancode_queue[g_game_scancode_head] = scancode;
    g_game_scancode_head = next;
}

uint8_t raycaster_input_pop_scancode_internal(uint8_t *scancode)
{
    if (scancode == 0) {
        return 0;
    }

    if (g_game_scancode_tail == g_game_scancode_head) {
        return 0;
    }

    *scancode = g_game_scancode_queue[g_game_scancode_tail];
    g_game_scancode_tail = (uint8_t)((g_game_scancode_tail + 1) % GAME_SCANCODE_QUEUE_SIZE);
    return 1;
}

void raycaster_input_set_active(int enabled)
{
    g_game_input_active = enabled;
    g_game_key_state = 0;
    g_game_exit_requested = 0;
    g_game_extended_scancode = 0;
    g_game_scancode_head = 0;
    g_game_scancode_tail = 0;
    (void)keyboard_get_last_key();
    if (enabled) {
        g_game_launch_requested = 0;
    }
}

int raycaster_input_is_active(void)
{
    return g_game_input_active;
}

void raycaster_input_request_launch(void)
{
    g_game_launch_requested = 1;
}

uint8_t raycaster_input_consume_launch_request(void)
{
    uint8_t requested = g_game_launch_requested;
    g_game_launch_requested = 0;
    return requested;
}

uint8_t raycaster_input_consume_exit_request_internal(void)
{
    uint8_t requested = g_game_exit_requested;
    g_game_exit_requested = 0;
    return requested;
}

void raycaster_input_request_exit(void)
{
    g_game_exit_requested = 1;
}

void raycaster_input_submit_scancode(uint8_t scancode)
{
    if (!g_game_input_active) {
        return;
    }

    if ((scancode & 0x7F) == 0x01) {
        g_game_exit_requested = 1;
    }

    raycaster_input_enqueue_scancode(scancode);

    if (scancode == 0xE0) {
        g_game_extended_scancode = 1;
        return;
    }

    if (g_game_extended_scancode) {
        uint8_t released = (uint8_t)(scancode & 0x80);
        uint8_t code = (uint8_t)(scancode & 0x7F);
        g_game_extended_scancode = 0;

        if (code == 0x48) {
            if (released) g_game_key_state &= (uint8_t)~GAME_KEY_W;
            else g_game_key_state |= GAME_KEY_W;
        } else if (code == 0x50) {
            if (released) g_game_key_state &= (uint8_t)~GAME_KEY_S;
            else g_game_key_state |= GAME_KEY_S;
        } else if (code == 0x4B) {
            if (released) g_game_key_state &= (uint8_t)~GAME_KEY_Q;
            else g_game_key_state |= GAME_KEY_Q;
        } else if (code == 0x4D) {
            if (released) g_game_key_state &= (uint8_t)~GAME_KEY_E;
            else g_game_key_state |= GAME_KEY_E;
        }
        return;
    }

    if (scancode & 0x80) {
        uint8_t released_key = (uint8_t)(scancode & ~0x80);
        if (released_key == 0x11) g_game_key_state &= (uint8_t)~GAME_KEY_W;
        else if (released_key == 0x1F) g_game_key_state &= (uint8_t)~GAME_KEY_S;
        else if (released_key == 0x1E) g_game_key_state &= (uint8_t)~GAME_KEY_A;
        else if (released_key == 0x20) g_game_key_state &= (uint8_t)~GAME_KEY_D;
        else if (released_key == 0x10) g_game_key_state &= (uint8_t)~GAME_KEY_Q;
        else if (released_key == 0x12) g_game_key_state &= (uint8_t)~GAME_KEY_E;
        return;
    }

    if (scancode == 0x11) g_game_key_state |= GAME_KEY_W;
    else if (scancode == 0x1F) g_game_key_state |= GAME_KEY_S;
    else if (scancode == 0x1E) g_game_key_state |= GAME_KEY_A;
    else if (scancode == 0x20) g_game_key_state |= GAME_KEY_D;
    else if (scancode == 0x10) g_game_key_state |= GAME_KEY_Q;
    else if (scancode == 0x12) g_game_key_state |= GAME_KEY_E;
}

char raycaster_poll_controls_internal(Raycaster *rc, uint8_t *key_down)
{
    uint8_t state = g_game_key_state;
    char instant_key = 0;

    key_down[RC_KEY_W] = (state & GAME_KEY_W) ? 1 : 0;
    key_down[RC_KEY_S] = (state & GAME_KEY_S) ? 1 : 0;
    key_down[RC_KEY_A] = (state & GAME_KEY_A) ? 1 : 0;
    key_down[RC_KEY_D] = (state & GAME_KEY_D) ? 1 : 0;
    key_down[RC_KEY_Q] = (state & GAME_KEY_Q) ? 1 : 0;
    key_down[RC_KEY_E] = (state & GAME_KEY_E) ? 1 : 0;

    {
        char key = keyboard_get_last_key();
        if (key == 27) {
            rc->game_running = false;
        } else if (key == 'w' || key == 'W' || key == 's' || key == 'S' ||
                   key == 'a' || key == 'A' || key == 'd' || key == 'D' ||
                   key == 'q' || key == 'Q' || key == 'e' || key == 'E') {
            char lower = key;
            if (lower >= 'A' && lower <= 'Z') {
                lower = (char)(lower - 'A' + 'a');
            }

            if (lower == 'w') key_down[RC_KEY_W] = 1;
            else if (lower == 's') key_down[RC_KEY_S] = 1;
            else if (lower == 'a') key_down[RC_KEY_A] = 1;
            else if (lower == 'd') key_down[RC_KEY_D] = 1;
            else if (lower == 'q') key_down[RC_KEY_Q] = 1;
            else if (lower == 'e') key_down[RC_KEY_E] = 1;

            instant_key = lower;
        }
    }

    return instant_key;
}
