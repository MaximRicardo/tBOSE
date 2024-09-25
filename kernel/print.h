#pragma once

extern unsigned PRINT_cursor_x;
extern unsigned PRINT_cursor_y;

void PRINT_char(char c, unsigned char_x, unsigned char_y);

void k_putchar(char c);

void k_puts(const char *str);

//Returns 0 if no error occured. Else returns a non-zero integer
__attribute__((format (printf, 1, 2)))
int k_printf(const char *restrict fmt, ...);
