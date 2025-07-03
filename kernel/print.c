#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include "print.h"
#include "char_bitmap.h"
#include "pixel.h"
#include "color.h"
#include "vbe.h"

/* please don't read this horrible code for your own good.
 *
 * I would like to hereby humbly apologize for this attrocity.
 * This catastrophe was a continous lapse of judgement which I never believed
 * could befall me. Alas, I was foolish. Please, bury this code as far away
 * from civilization as possible, lest calamity befall all of us.
 */

uint32_t PRINT_cursor_x = 0;
uint32_t PRINT_cursor_y = 0;

static uint32_t round_up(uint32_t num, uint32_t multiple)
{
	uint32_t remainder;

	if (multiple == 0)
		return num;

	remainder = num % multiple;
	if (remainder == 0)
		return num;

	return num + multiple - remainder;
}

static uint32_t cursor_x_max(void)
{
	return VBE_mode_info.width / (CHAR_BITMAP_bitmap_width + 1) - 1;
}

static uint32_t cursor_y_max(void)
{
	return VBE_mode_info.height / (CHAR_BITMAP_bitmap_height + 1) - 1;
}

static uint32_t cursor_x_to_scr_x(void)
{
	//One pixel of empty space between characters
	return (CHAR_BITMAP_bitmap_width + 1) * PRINT_cursor_x;
}

static uint32_t cursor_y_to_scr_y(void)
{
	//One pixel of empty space between characters
	return (CHAR_BITMAP_bitmap_height + 1) * PRINT_cursor_y;
}

static void move_cursor_down(void)
{
	if (PRINT_cursor_y < cursor_y_max())
		++PRINT_cursor_y;
	else
		PRINT_cursor_y = 0;
}

static void move_cursor_up(void)
{
	if (PRINT_cursor_y > 0)
		--PRINT_cursor_y;
	else
		PRINT_cursor_y = cursor_y_max();
}

static size_t get_str_len(const char *str)
{
	size_t len = 0;
	while (*str++ != '\0')
		++len;
	return len;
}

//Flips the frame buffer on one line.
//cursor_y is the y position the text cursor has to have to be on that line
static void flip_line_scr_buffer(unsigned cursor_y)
{
	unsigned start_y = (CHAR_BITMAP_bitmap_height + 1) * cursor_y;
	unsigned end_y = (CHAR_BITMAP_bitmap_height + 1) * (cursor_y + 1) - 1;

	PIXEL_partially_flip_buffer(0, start_y, VBE_mode_info.width, end_y);
}

/* renders directly to the front buffer if the back buffer isn't available. */
static void plot_char_pixel(char c, uint32_t char_x, uint32_t char_y, size_t x,
			    size_t y, const struct COLOR_rgb *foreground,
			    const struct COLOR_rgb *background)
{
	size_t bitmap_idx = c - 32;
	uint8_t row = CHAR_BITMAP_bitmaps[bitmap_idx][y];

	//the character bitmaps are horizontally mirrored
	unsigned pixel = (row >> (CHAR_BITMAP_bitmap_width - x - 1)) & 1;

	if (pixel == 0) {
		if (!PIXEL_back_buffer)
			PIXEL_plot_norm_rgb(x + char_x, y + char_y,
					    *background);
		else
			PIXEL_back_buffer[VBE_mode_info.width * (y + char_y) +
					  (x + char_x)] = *background;
	} else {
		if (!PIXEL_back_buffer)
			PIXEL_plot_norm_rgb(x + char_x, y + char_y,
					    *foreground);
		else
			PIXEL_back_buffer[VBE_mode_info.width * (y + char_y) +
					  (x + char_x)] = *foreground;
	}
}

void PRINT_char(char c, unsigned char_x, unsigned char_y)
{
	//If c is not a character, return
	if (c > 126 || c < 32)
		return;

	struct COLOR_rgb foreground_color = { 1.f, 1.f, 1.f };
	struct COLOR_rgb background_color = { 0.f, 0.f, 0.f };

	for (uint32_t y = 0; y < CHAR_BITMAP_bitmap_height; y++) {
		for (uint32_t x = 0; x < CHAR_BITMAP_bitmap_width; x++) {
			plot_char_pixel(c, char_x, char_y, x, y,
					&foreground_color, &background_color);
		}
	}
}

void PRINT_reset_cursor_pos()
{
	PRINT_cursor_x = 0;
	PRINT_cursor_y = 0;
}

//Doesn't flip the frame buffer
void k_putchar(char c)
{
	if (c == '\0') {
		return;
	} else if (c == '\r') {
		PRINT_cursor_x = 0;
	} else if (c == '\n') {
		flip_line_scr_buffer(PRINT_cursor_y);
		PRINT_cursor_x = 0;
		move_cursor_down();
	} else if (c == '\b') {
		if (PRINT_cursor_x > 0)
			--PRINT_cursor_x;
		else {
			PRINT_cursor_x = cursor_x_max();
			move_cursor_up();
		}
	} else if (c == '\t') {
		//the +1 is to make sure the cursor goes to the next tab if it
		//is already on one.
		PRINT_cursor_x = round_up(PRINT_cursor_x + 1, 8);

		if (PRINT_cursor_x > cursor_x_max()) {
			PRINT_cursor_x = 0;
			move_cursor_down();
		}
	} else {
		PRINT_char(c, cursor_x_to_scr_x(), cursor_y_to_scr_y());
		if (PRINT_cursor_x < cursor_x_max())
			++PRINT_cursor_x;
		else {
			PRINT_cursor_x = 0;
			move_cursor_down();
		}
	}
}

static void print_str(const char *str)
{
	char c;
	while ((c = *str++) != '\0') {
		k_putchar(c);
	}
}

void k_puts(const char *str)
{
	print_str(str);
	k_putchar('\n');
}

/* size doesn't count the NULL terminator */
static void flip_str(char *dest, const char *src, size_t size)
{
	size_t i = size - 1;
	size_t j = 0;

	for (; j <= i; j++) {
		dest[i] = src[j];
		dest[j] = src[i];

		--i;
	}

	dest[size] = '\0';
}

//Returns the number of digits printed
static size_t print_uint(unsigned value, unsigned digits_to_print,
			 char digit_to_extend_with)
{
	/* Maximum number of digits an int can have is 10 */
	char str[100];
	char flipped_str[100];
	size_t i = 0;

	if (value < 10) {
		for (int i = 0; i < (int)digits_to_print - 1; i++)
			k_putchar(digit_to_extend_with);
		k_putchar('0' + value);
		return 1;
	}

	while (value >= 10) {
		str[i] = (value % 10) + '0';

		value = value / 10;
		++i;
	}

	/* Need to do it one extra time, to print the most significant digit */
	str[i] = (value % 10) + '0';
	str[i + 1] = '\0';
	flip_str(flipped_str, str, i + 1);

	for (size_t k = get_str_len(flipped_str); k < digits_to_print; k++)
		k_putchar(digit_to_extend_with);
	print_str(flipped_str);

	return i;
}

//Returns the number of digits printed
static size_t print_ulint(unsigned long value, unsigned digits_to_print,
			  char digit_to_extend_with)
{
	/* Maximum number of digits an int can have is 10 */
	char str[100];
	char flipped_str[100];
	size_t i = 0;

	/* Program breaks if I don't do this */
	if (value < 10) {
		/* If value less than 10, just print value and return */
		for (int i = 0; i < (int)digits_to_print - 1; i++)
			k_putchar(digit_to_extend_with);
		k_putchar('0' + value);
		return 1;
	}

	while (value >= 10) {
		str[i] = (value % 10) + '0';

		value = value / 10;
		++i;
	}

	/* Need to do it one extra time, to print the most significant digit */
	str[i] = (value % 10) + '0';
	str[i + 1] = '\0';
	flip_str(flipped_str, str, i + 1);

	for (unsigned k = get_str_len(flipped_str); k < digits_to_print; k++)
		k_putchar(digit_to_extend_with);
	print_str(flipped_str);

	return get_str_len(flipped_str);
}

//Returns the number of digits printed
static size_t print_int(int value, unsigned digits_to_print,
			char digit_to_extend_with)
{
	if (value < 0) {
		k_putchar('-'); /* Print a negative symbol */
		value *= -1; /* Set val to be positive before printing it */
	}

	/* Now val can be printed like an unsigned int */
	return print_uint(value, digits_to_print, digit_to_extend_with);
}

//Returns the number of digits printed
static size_t print_lint(long value, unsigned digits_to_print,
			 char digit_to_extend_with)
{
	if (value < 0) {
		k_putchar('-'); /* Print a negative symbol */
		value *= -1; /* Set val to be positive before printing it */
	}

	/* Now val can be printed like an unsigned int */
	return print_ulint(value, digits_to_print, digit_to_extend_with);
}

//Returns the number of digits printed
static size_t print_hex_uint(unsigned value, bool upper_case,
			     unsigned digits_to_print,
			     char digit_to_extend_with)
{
	char str[100];
	char flipped_str[100];
	size_t i = 0;

	/* Program breaks if I don't do this */
	if (value < 10) {
		/* If val is below 10, just print the number and return */
		for (uint32_t i = 0; i < digits_to_print - 1; i++)
			k_putchar(digit_to_extend_with);
		k_putchar('0' + value);
		return 1;
	} else if (value < 16) {
		/* If val is below 16, do the same thing but with hex digits */
		for (uint32_t i = 0; i < digits_to_print - 1; i++)
			k_putchar(digit_to_extend_with);
		k_putchar('0' - 10 + value);
		return 1;
	}

	while (value >= 16) {
		uint8_t c = (value % 16);
		if (c <= 9)
			str[i] = c + '0';
		else {
			if (upper_case)
				str[i] = c + 'A' - 10;
			else
				str[i] = c + 'a' - 10;
		}

		value = value / 16;
		++i;
	}

	/* Need to do it one extra time, to print the most significant digit */
	str[i] = (value % 16);
	if (str[i] <= 9)
		str[i] = str[i] + '0';
	else {
		if (upper_case)
			str[i] = str[i] + 'A' - 10;
		else
			str[i] = str[i] + 'a' - 10;
	}

	str[i + 1] = '\0';
	flip_str(flipped_str, str, i + 1);

	for (unsigned k = get_str_len(flipped_str); k < digits_to_print; k++)
		k_putchar(digit_to_extend_with);

	print_str(flipped_str);

	return get_str_len(flipped_str);
}

static void print_hex_ulint(unsigned long value, bool upper_case,
			    unsigned digits_to_print, char digit_to_extend_with)
{
	char str[100];
	char flipped_str[100];
	size_t i = 0;

	if (value < 10) {
		for (uint32_t i = 0; i < digits_to_print - 1; i++)
			k_putchar(digit_to_extend_with);
		k_putchar('0' + value);
		return;
	} else if (value < 16) {
		for (uint32_t i = 0; i < digits_to_print - 1; i++)
			k_putchar(digit_to_extend_with);
		k_putchar(upper_case ? 'A' : 'a' + value - 10);
		return;
	}

	while (value >= 16) {
		uint8_t c = (value % 16);
		if (c <= 9)
			str[i] = c + '0';
		else {
			if (upper_case)
				str[i] = c + 'A' - 10;
			else
				str[i] = c + 'a' - 10;
		}

		value = value / 16;
		++i;
	}

	/* Need to do it one extra time, to print the most significant digit */
	str[i] = (value % 16);
	if (str[i] <= 9)
		str[i] = str[i] + '0';
	else {
		if (upper_case)
			str[i] = str[i] + 'A' - 10;
		else
			str[i] = str[i] + 'a' - 10;
	}

	str[i + 1] = '\0';
	flip_str(flipped_str, str, i + 1);

	for (unsigned k = get_str_len(flipped_str); k < digits_to_print; k++)
		k_putchar(digit_to_extend_with);

	print_str(flipped_str);
}

/* not my proudest work */
int k_printf(const char *restrict fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char c;
	while ((c = *fmt++) != '\0') {
		if (c == '%') {
			c = *fmt++;

			//minimum number of digits to print if the format is a
			//number
			unsigned num_n_digits = 1;
			char digit_to_extend_with = 0;
			if (c >= '0' && c <= '9') {
				digit_to_extend_with = c;
				num_n_digits = *fmt++ - '0';
				c = *fmt++;
			}

			if (c == '%')
				k_putchar('%');
			else if (c == 's')
				print_str(va_arg(args, const char *));
			else if (c == 'c')
				k_putchar(va_arg(args, int));
			else if (c == 'u')
				print_uint(va_arg(args, unsigned), num_n_digits,
					   digit_to_extend_with);
			else if (c == 'd')
				print_int(va_arg(args, int), num_n_digits,
					  digit_to_extend_with);
			else if (c == 'x')
				print_hex_uint(va_arg(args, unsigned), false,
					       num_n_digits,
					       digit_to_extend_with);
			else if (c == 'X')
				print_hex_uint(va_arg(args, unsigned), true,
					       num_n_digits,
					       digit_to_extend_with);
			else if (c == 'p') {
				const void *ptr = va_arg(args, const void *);
				if (ptr == NULL)
					print_str("(nil)");
				else
					k_printf("0x%08lx", (unsigned long)ptr);
			} else if (c == 'l') {
				c = *fmt++;
				if (c == 'u')
					print_ulint(va_arg(args, unsigned long),
						    num_n_digits,
						    digit_to_extend_with);
				else if (c == 'd')
					print_lint(va_arg(args, long),
						   num_n_digits,
						   digit_to_extend_with);
				else if (c == 'x')
					print_hex_ulint(va_arg(args,
							       unsigned long),
							false, num_n_digits,
							digit_to_extend_with);
				else if (c == 'X')
					print_hex_ulint(va_arg(args,
							       unsigned long),
							true, num_n_digits,
							digit_to_extend_with);
				else {
					//Else there is an error
					va_end(args);
					return 1;
				}
			}
		} else {
			k_putchar(c);
		}
	}

	va_end(args);

	return 0;
}
