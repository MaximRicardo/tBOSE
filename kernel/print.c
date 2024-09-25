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

unsigned PRINT_cursor_x = 0;
unsigned PRINT_cursor_y = 0;

static unsigned cursor_x_max(void) {
    return VBE_mode_info.width / (CHAR_BITMAP_bitmap_width+1) - 1;
}

static unsigned cursor_y_max(void) {
    return VBE_mode_info.height / (CHAR_BITMAP_bitmap_height+1) - 1;
}

static unsigned cursor_x_to_scr_x(void) {
    //One pixel of empty space between characters
    return (CHAR_BITMAP_bitmap_width+1)*PRINT_cursor_x;
}

static unsigned cursor_y_to_scr_y(void) {
    //One pixel of empty space between characters
    return (CHAR_BITMAP_bitmap_height+1)*PRINT_cursor_y;
}

static void move_cursor_down(void) {
    if (PRINT_cursor_y < cursor_y_max()) ++PRINT_cursor_y;
    else PRINT_cursor_y = 0;
}

static void move_cursor_up(void) {
    if (PRINT_cursor_y > 0) --PRINT_cursor_y;
    else PRINT_cursor_y = cursor_y_max();
}

void PRINT_char(char c, unsigned char_x, unsigned char_y) {

    //If c is not a character, return
    if (c > 126 || c < 32) return;

    size_t bitmap_idx = c-32;

    struct COLOR_rgb foreground_color = {1.f, 1.f, 1.f};
    struct COLOR_rgb background_color = {0.f, 0.f, 0.f};

    for (unsigned y = 0; y < CHAR_BITMAP_bitmap_height; y++) {
        uint8_t row = CHAR_BITMAP_bitmaps[bitmap_idx][y];

        for (unsigned x = 0; x < CHAR_BITMAP_bitmap_width; x++) {
            unsigned pixel = (row >> (CHAR_BITMAP_bitmap_width-x-1)) & 1; //Get the bit opposite of the xth bit of the row as the pixel data
            if (pixel == 0) PIXEL_plot_norm_rgb(x+char_x, y+char_y, background_color);
            else PIXEL_plot_norm_rgb(x+char_x, y+char_y, foreground_color);
        }
    }

}

void k_putchar(char c) {

    if (c == '\r') {
        move_cursor_down();
    }
    else if (c == '\n' || c == '\r') {
        PRINT_cursor_x = 0;
        move_cursor_down();
    }
    else if (c == '\b') {
        if (PRINT_cursor_x != 0) --PRINT_cursor_x;
        else {
            PRINT_cursor_x = cursor_x_max();
            move_cursor_up();
        }
    }
    else if (c == '\t') {
        //This rounds cursor x to the closest greater mutliple of 8. No idea how
        PRINT_cursor_x = (PRINT_cursor_x+7) & -8;
        if (PRINT_cursor_x > cursor_x_max()) {
            PRINT_cursor_x %= cursor_x_max();
            move_cursor_down();
        }
    }
    else {
        PRINT_char(c, cursor_x_to_scr_x(), cursor_y_to_scr_y());
        if (PRINT_cursor_x < cursor_x_max()) ++PRINT_cursor_x;
        else {
            PRINT_cursor_x = 0;
            move_cursor_down();
        }
    }

}

static void print_str(const char *str) {
    char c;
    while ((c = *str++) != '\0') {
        k_putchar(c);
    }
}

void k_puts(const char *str) {
    print_str(str);
    k_putchar('\n');
}

static void print_uint(unsigned value) {

    /* Maximum number of digits an int can have is 10 */
    char str[100];
    char flipped_str[100];
    size_t i = 0;
    size_t j = 0;

    /* Program breaks if I don't do this */
    if (value < 10) {
        /* If value less than 10, just print value and return */
        k_putchar('0'+value);
        return;
    }

    while (value >= 10) {
        str[i] = (value % 10) + '0';
        
        value = value / 10;
        ++i;
    }

    /* Need to do it one extra time, to print the most significant digit */
    str[i] = (value % 10) + '0';

    /* Flip around the string, so it is the right way around */
    flipped_str[i+1] = '\0'; /* Null terminate the string before the index of the end of the number is lost */
    for (; j <= i; j++) {
        flipped_str[i] = str[j];
        flipped_str[j] = str[i];        

        --i;
    }

    print_str(flipped_str);

}

static void print_ulint(unsigned long value) {

    /* Maximum number of digits an int can have is 10 */
    char str[100];
    char flipped_str[100];
    size_t i = 0;
    size_t j = 0;

    /* Program breaks if I don't do this */
    if (value < 10) {
        /* If value less than 10, just print value and return */
        k_putchar('0'+value);
        return;
    }

    while (value >= 10) {
        str[i] = (value % 10) + '0';
        
        value = value / 10;
        ++i;
    }

    /* Need to do it one extra time, to print the most significant digit */
    str[i] = (value % 10) + '0';

    /* Flip around the string, so it is the right way around */
    flipped_str[i+1] = '\0'; /* Null terminate the string before the index of the end of the number is lost */
    for (; j <= i; j++) {
        flipped_str[i] = str[j];
        flipped_str[j] = str[i];        

        --i;
    }

    print_str(flipped_str);

}

static void print_int(int value) {

    if (value < 0) {
        k_putchar('-'); /* Print a negative symbol */
        value *= -1;    /* Set val to be positive before printing it */
    }

    /* Now val can be printed like an unsigned int */
    print_uint(value);

}

static void print_lint(long value) {

    if (value < 0) {
        k_putchar('-'); /* Print a negative symbol */
        value *= -1;    /* Set val to be positive before printing it */
    }

    /* Now val can be printed like an unsigned int */
    print_ulint(value);

}

static void print_hex_uint(unsigned value, bool upper_case) {

    char str[100];
    char flipped_str[100];
    size_t i = 0;
    size_t j = 0;

    /* Program breaks if I don't do this */
    if (value < 10) {
        /* If val is below 10, just print the number and return */
        k_putchar('0'+value);
        return;
    }
    else if (value < 16) {
        /* If val is below 16, do the same thing but with hex digits */
        k_putchar('0'-10+value);
        return;
    }

    while (value >= 16) {
        uint8_t c = (value % 16);
        if (c <= 9) str[i] = c + '0';
        else {
            if (upper_case) str[i] = c + 'A' - 10;
            else str[i] = c + 'a' - 10;
        }
        
        value = value / 16;
        ++i;
    }

    /* Need to do it one extra time, to print the most significant digit */
    str[i] = (value % 16);
    if (str[i] <= 9) str[i] = str[i] + '0';
    else {
        if (upper_case) str[i] = str[i] + 'A' - 10;
        else str[i] = str[i] + 'a' - 10;
    }

    /* Flip around the string, so it is the right way around */
    flipped_str[i+1] = '\0'; /* Null terminate the string before the index of the end of the number is lost */
    for (; j <= i; j++) {
        flipped_str[i] = str[j];
        flipped_str[j] = str[i];        

        --i;
    }

    print_str(flipped_str);

}

static void print_hex_ulint(unsigned long value, bool upper_case) {

    char str[100];
    char flipped_str[100];
    size_t i = 0;
    size_t j = 0;

    /* Program breaks if I don't do this */
    if (value < 10) {
        /* If val is below 10, just print the number and return */
        k_putchar('0'+value);
        return;
    }
    else if (value < 16) {
        /* If val is below 16, do the same thing but with hex digits */
        k_putchar('0'-10+value);
        return;
    }

    while (value >= 16) {
        uint8_t c = (value % 16);
        if (c <= 9) str[i] = c + '0';
        else {
            if (upper_case) str[i] = c + 'A' - 10;
            else str[i] = c + 'a' - 10;
        }
        
        value = value / 16;
        ++i;
    }

    /* Need to do it one extra time, to print the most significant digit */
    str[i] = (value % 16);
    if (str[i] <= 9) str[i] = str[i] + '0';
    else {
        if (upper_case) str[i] = str[i] + 'A' - 10;
        else str[i] = str[i] + 'a' - 10;
    }

    /* Flip around the string, so it is the right way around */
    flipped_str[i+1] = '\0'; /* Null terminate the string before the index of the end of the number is lost */
    for (; j <= i; j++) {
        flipped_str[i] = str[j];
        flipped_str[j] = str[i];        

        --i;
    }

    print_str(flipped_str);

}

int k_printf(const char *restrict fmt, ...) {

    va_list args;
    va_start(args, fmt);

    char c;
    while ((c = *fmt++) != '\0') {
        if (c == '%') {
            c = *fmt++; //Get the next character
            if (c == '%')
                k_putchar('%');
            else if (c == 's')
                print_str(va_arg(args, const char*));
            else if (c == 'c')
                k_putchar(va_arg(args, int));
            else if (c == 'u')
                print_uint(va_arg(args, unsigned));
            else if (c == 'd')
                print_int(va_arg(args, int));
            else if (c == 'x')
                print_hex_uint(va_arg(args, unsigned), false);
            else if (c == 'X')
                print_hex_uint(va_arg(args, unsigned), true);
            else if (c == 'p') {
                const void *ptr = va_arg(args, const void*);
                if (ptr == NULL) print_str("(nil)");
                else k_printf("0x%lx", (unsigned long)ptr);
            }
            else if (c == 'l') {
                c = *fmt++;
                if (c == 'u') print_ulint(va_arg(args, unsigned long));
                else if (c == 'd') print_lint(va_arg(args, long));
                else if (c == 'x') print_hex_ulint(va_arg(args, unsigned long), false);
                else if (c == 'X') print_hex_ulint(va_arg(args, unsigned long), true);
                else {
                    //Else there is an error
                    va_end(args);
                    return 1;
                }
            }
        }
        else {
            k_putchar(c);
        }
    }

    va_end(args);

    return 0;

}
