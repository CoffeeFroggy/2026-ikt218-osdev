#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "libc/stdint.h"
#include "libc/stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

// Size of the 2D map used for collisions/walls
#define MAP_WIDTH 16
#define MAP_HEIGHT 16

// View settings
#define FOV 60  // Field of view in degrees
#define MAX_DEPTH 20

// Player position and look direction
typedef struct {
    double x, y; // Position in map
    double angle; // Viewing angle n degrees (0-360)
    double speed; // Movement speed
} Player;

// Main game state
typedef struct {
    Player player;
    uint32_t frame_count; // Number of rendered frames
    bool game_running; // False means exit game loop
} Raycaster;

// Core game functions
void raycaster_init(Raycaster *rc);
int raycaster_check_collision(double x, double y);
void raycaster_game_loop(void);

// Input hooks used by keyboard IRQ and kernel main loop
void raycaster_input_set_active(int enabled);
int raycaster_input_is_active(void);
void raycaster_input_submit_scancode(uint8_t scancode);
void raycaster_input_request_launch(void);
uint8_t raycaster_input_consume_launch_request(void);
void raycaster_input_request_exit(void);

#ifdef __cplusplus
}
#endif

#endif
