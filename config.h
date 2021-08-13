#ifndef CONFIG_H
#define CONFIG_H

#include <SDL2/SDL.h>

//------------------------------- Compatibility --------------------------------

/*
 * The `8xy6` opcode is right-shift, the `8xyE` is left-shift.
 * Register 0xF is set to the shifted-out bit of register X.
 *
 * If this is set,
 *      register X is set to register Y shifted,
 * if not set,
 *      register X is set to register X shifted.
 *
 * The old implementations used the Y register,
 */
#define SHIFT_Y_REG_INSTEAD_OF_X 1

/*
 * The `Fx55` and `Fx66` opcodes loop through the registers and write them to / read from the memory.
 * This variable marks if the index register needs to be incremented while doing the operations.
 * In the original implementation this happened.
 */
#define INC_I_AFTER_MEM_OP 1

//-------------------------------- Logging -------------------------------------

/*
 * If 1, the currently executed opcode, the current PC value and the pressed
 * key is logged to the terminal.
 */
#define VERBOSE_LOG 0

//--------------------------------- Look ---------------------------------------

// Background color (inactive pixels)
#define BG_COLOR_R (uint8_t)25
#define BG_COLOR_G (uint8_t)60
#define BG_COLOR_B (uint8_t)15

// Foreground color (active pixels)
#define FG_COLOR_R (uint8_t)86
#define FG_COLOR_G (uint8_t)185
#define FG_COLOR_B (uint8_t)34

// Panic screen background
#define PANIC_BG_COLOR_R (uint8_t)8
#define PANIC_BG_COLOR_G (uint8_t)39
#define PANIC_BG_COLOR_B (uint8_t)245

// Panic screen foreground
#define PANIC_FG_COLOR_R (uint8_t)255
#define PANIC_FG_COLOR_G (uint8_t)255
#define PANIC_FG_COLOR_B (uint8_t)255

// Font color of messages
#define MESSAGE_COLOR_R (uint8_t)255
#define MESSAGE_COLOR_G (uint8_t)255
#define MESSAGE_COLOR_B (uint8_t)0


//-------------------------------- Shortcuts -----------------------------------

// See: https://wiki.libsdl.org/SDL_Keycode

#define SHORTCUT_KEYCODE_PAUSE           SDLK_p
#define SHORTCUT_KEYCODE_FULLSCREEN      SDLK_F11
#define SHORTCUT_KEYCODE_STEPPING_MODE   SDLK_F5
#define SHORTCUT_KEYCODE_STEP_INST       SDLK_F6
#define SHORTCUT_KEYCODE_TOGGLE_CURSOR   SDLK_F9
#define SHORTCUT_KEYCODE_DEBUG_MODE      SDLK_F10
#define SHORTCUT_KEYCODE_QUIT            SDLK_ESCAPE
#define SHORTCUT_KEYCODE_DUMP_STATE      SDLK_BACKSPACE
#define SHORTCUT_KEYCODE_INC_SPEED       SDLK_F8
#define SHORTCUT_KEYCODE_DEC_SPEED       SDLK_F7
#define SHORTCUT_KEYCODE_RESET           SDLK_F4
#define SHORTCUT_KEYCODE_SCREENSHOT      SDLK_F2
#define SHORTCUT_KEYCODE_TOGGLE_HELP     SDLK_F1

//--------------------------------- Misc. --------------------------------------

/*
 * How long the messages in the left upper corner should be shown.
 */
#define MESSAGE_SHOW_TIME_S 3.0

/*
 * How long the beep sound should be. Specified in frames
 */
#define BEEP_DURATION 50 // frames

#endif // CONFIG_H
