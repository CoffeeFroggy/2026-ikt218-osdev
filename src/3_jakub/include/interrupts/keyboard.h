#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "libc/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_keyboard();
void keyboard_print_prompt(void);
char keyboard_get_last_key(void);

#ifdef __cplusplus
}
#endif

#endif
