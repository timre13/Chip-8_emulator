#ifndef CONFIG_H
#define CONFIG_H

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


// Background color (inactive pixels)
#define BG_COLOR_R (uint8_t)25
#define BG_COLOR_G (uint8_t)60
#define BG_COLOR_B (uint8_t)15

// Foreground color (active pixels)
#define FG_COLOR_R (uint8_t)86
#define FG_COLOR_G (uint8_t)185
#define FG_COLOR_B (uint8_t)34


#endif // CONFIG_H
