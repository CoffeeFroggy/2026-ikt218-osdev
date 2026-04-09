#include "apps/raycaster/raycaster_internal.h"
#include "apps/song/song.h"
#include "common.h"
#include "kernel/pit.h"



// MÅ FIKSES
// Advance one note in the background song when it is time
// This is non-blocking, so the game can keep rendering every frame
static void raycaster_play_background_theme(uint32_t current_ticks, uint32_t *next_note_tick, uint32_t *music_index)
{
    const uint32_t bg_theme_len = (uint32_t)(sizeof(raycaster_bg_theme) / sizeof(raycaster_bg_theme[0]));
    // Nothing to play yet, or song is empty
    if (bg_theme_len == 0 || (int32_t)(current_ticks - *next_note_tick) < 0) {
        return;
    }

    // If rendering lags, fast-forward song time so audio timing stays correct.
    // Only the currently relevant note is played (no burst of old notes).
    Note note = raycaster_bg_theme[*music_index];
    uint32_t steps = 0;
    const uint32_t max_steps = 64;

    while ((int32_t)(current_ticks - *next_note_tick) >= 0 && steps < max_steps) {
        note = raycaster_bg_theme[*music_index];
        *next_note_tick += note.duration;
        (*music_index)++;
        if (*music_index >= bg_theme_len) {
            *music_index = 0;
        }
        steps++;
    }

    // Play the note that should be active now
    if (note.frequency == R) {
        stop_sound();
    } else {
        play_sound(note.frequency);
    }
}

// Main game loop: input -> update -> render -> present
void raycaster_game_loop(void)
{
    // ~60 FPS target frame time
    const uint32_t target_frame_ms = 16;
    uint32_t music_index = 0;
    uint32_t next_note_tick;

    // Enter game mode: capture input + switch to VGA graphics mode
    raycaster_input_set_active(1);
    raycaster_vga13_set_mode_internal();
    raycaster_vga13_clear_internal(0);

    // Create and initialize game state.
    Raycaster raycaster;
    raycaster_init(&raycaster);

    next_note_tick = pit_get_ticks();

    // Current key state used by movement/rotation code
    uint8_t key_down[RC_KEY_COUNT] = {0};
    // Fixed-step physics timer state
    uint32_t last_physics_tick = pit_get_ticks();
    uint32_t physics_accumulator = 0;

    // Minimap starts visible, this tracks M key debounce.
    int minimap_visible = 1;
    uint8_t minimap_m_down = 0;

    while (raycaster.game_running) {
        // Used to limit frame duration at end of loop.
        uint32_t frame_start_ticks = pit_get_ticks();

        // Poll current controls (WASD/QE + instant keys)
        char instant_key = raycaster_poll_controls_internal(&raycaster, key_down);
        // Exit requested by keyboard handler
        if (raycaster_input_consume_exit_request_internal()) {
            raycaster.game_running = false;
        }
        uint32_t current_ticks = pit_get_ticks();
        uint8_t scancode = 0;

        // Consume all pending raw scancodes for one-shot actions
        while (raycaster_input_pop_scancode_internal(&scancode)) {
            uint8_t code = (uint8_t)(scancode & 0x7F);
            uint8_t released = (uint8_t)(scancode & 0x80);

            // ESC exits the game immediately
            if (code == 0x01) {
                raycaster.game_running = false;
                break;
            }

            // M toggles minimap only on key press (not while held down)
            if (code == 0x32) {
                if (!released) {
                    if (!minimap_m_down) {
                        minimap_visible = !minimap_visible;
                        minimap_m_down = 1;
                    }
                } else {
                    minimap_m_down = 0;
                }
            }
        }

        // Add elapsed real time to fixed-step physics accumulator
        uint32_t elapsed_ms = current_ticks - last_physics_tick;
        last_physics_tick = current_ticks;
        if (elapsed_ms > 100) elapsed_ms = 100;
        physics_accumulator += elapsed_ms;

        // Instant keys should trigger at least one physics step
        if (instant_key == 'w' || instant_key == 'W') key_down[RC_KEY_W] = 1;
        else if (instant_key == 's' || instant_key == 'S') key_down[RC_KEY_S] = 1;
        else if (instant_key == 'a' || instant_key == 'A') key_down[RC_KEY_A] = 1;
        else if (instant_key == 'd' || instant_key == 'D') key_down[RC_KEY_D] = 1;
        else if (instant_key == 'q' || instant_key == 'Q') key_down[RC_KEY_Q] = 1;
        else if (instant_key == 'e' || instant_key == 'E') key_down[RC_KEY_E] = 1;

        if (instant_key != 0 && physics_accumulator < RAYCASTER_PHYSICS_STEP_MS) {
            physics_accumulator = RAYCASTER_PHYSICS_STEP_MS;
        }

        // Run physics in fixed-size time slices for stable movement
        while (physics_accumulator >= RAYCASTER_PHYSICS_STEP_MS) {
            raycaster.player.speed = RAYCASTER_MOVE_STEP;

            if (key_down[RC_KEY_W] || key_down[RC_KEY_S] || key_down[RC_KEY_A] ||
                key_down[RC_KEY_D] || key_down[RC_KEY_Q] || key_down[RC_KEY_E]) {
                raycaster_apply_key_state_internal(&raycaster, key_down);
            }

            physics_accumulator -= RAYCASTER_PHYSICS_STEP_MS;
        }

        raycaster.frame_count++;

        // Draw world, optional minimap, then show frame
        raycaster_render_mode13_internal(&raycaster);
        if (minimap_visible) {
            raycaster_draw_minimap_mode13_internal(&raycaster);
        }
        raycaster_vga13_present_internal();

        // Update music after rendering
        current_ticks = pit_get_ticks();
        raycaster_play_background_theme(current_ticks, &next_note_tick, &music_index);

        // Sleep the remaining frame time to keep FPS stable
        {
            uint32_t frame_time = pit_get_ticks() - frame_start_ticks;
            if (frame_time < target_frame_ms) {
                sleep_interrupt(target_frame_ms - frame_time);
            }
        }
    }

    // Leave game mode and restore terminal environment
    stop_sound();
    raycaster_vga_text_set_mode_internal();
    raycaster_input_set_active(0);
}