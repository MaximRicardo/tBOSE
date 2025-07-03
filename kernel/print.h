#pragma once

#include <stdint.h>

extern uint32_t PRINT_cursor_x;
extern uint32_t PRINT_cursor_y;

//Prints a character exactly at pixel coordinates: char_x, char_y. Ignores the text cursor's position
void PRINT_char(char c, unsigned char_x, unsigned char_y);

void PRINT_reset_cursor_pos();

//Acts like the C function
void k_putchar(char c);

//Acts like the C function
void k_puts(const char *str);

//Returns 0 if no error occured. Else returns a non-zero integer.
//Otherwise, just acts like the C function but with some formats missing for now.
__attribute__((format(printf, 1, 2))) int k_printf(const char *restrict fmt,
						   ...);
