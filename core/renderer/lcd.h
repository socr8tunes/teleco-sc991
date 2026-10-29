/*
 * 1-bit framebuffer and bitmap text for the dot-matrix LCD (192 x 64).
 * Format matches hal/display.h: row-major, MSB = leftmost pixel, set bit = dark.
 */
#ifndef TELECO_RENDERER_LCD_H
#define TELECO_RENDERER_LCD_H

#include <stdint.h>

#define LCD_W      192
#define LCD_H      64
#define LCD_STRIDE ((LCD_W + 7) / 8)

#define FONT_W   6   /* advance of the main font (5 px glyph + 1 gap) */
#define FONT_H   7   /* glyph height                                  */
#define SFONT_W  4   /* advance of the small font (3 + 1)             */
#define SFONT_H  5

/* Symbol codes (8-bit, outside ASCII) used in strings. */
#define S_SQRT   "\x80"
#define S_PI     "\x81"
#define S_TIMES  "\x82"
#define S_DIV    "\x83"
#define S_DEG    "\x84"
#define S_ARROW  "\x85"
#define S_INT    "\x86"
#define S_SIGMA  "\x87"
#define S_THETA  "\x88"
#define S_ANGLE  "\x89"
#define S_SD     "\x8A"
#define S_RIGHT  "\x8B"
#define S_LEFT   "\x8C"
#define S_UP     "\x8D"
#define S_DOWN   "\x8E"
#define S_LE     "\x8F"
#define S_GE     "\x90"
#define S_NE     "\x91"
#define S_MINUS  "\x92"
#define S_XBAR   "\x93"
#define S_YBAR   "\x94"
#define S_SIGSM  "\x95"
#define S_BOX    "\x96"
#define S_DOTS   "\x97"
#define S_MU     "\x98"
#define S_OMEGA  "\x99"
#define S_LBL    "\x9A"
#define S_CHECK  "\x9B"

typedef struct lcd {
    uint8_t px[LCD_STRIDE * LCD_H];
} lcd_t;

void lcd_clear(lcd_t *l);
void lcd_pixel(lcd_t *l, int x, int y, int on);      /* clipped */
int  lcd_get(const lcd_t *l, int x, int y);
void lcd_hline(lcd_t *l, int x, int y, int w, int on);
void lcd_vline(lcd_t *l, int x, int y, int h, int on);
void lcd_fill(lcd_t *l, int x, int y, int w, int h, int on);
void lcd_invert(lcd_t *l, int x, int y, int w, int h);
void lcd_frame(lcd_t *l, int x, int y, int w, int h);

/* Draw one glyph with its top-left at (x, y). Returns the advance in pixels. */
int lcd_char(lcd_t *l, int x, int y, unsigned char ch, int small);
/* Draw a string; returns the x position after the last glyph. */
int lcd_text(lcd_t *l, int x, int y, const char *s, int small);
/* Pixel width of a string. */
int lcd_text_w(const char *s, int small);
/* Right-aligned text ending at x_right. */
int lcd_text_right(lcd_t *l, int x_right, int y, const char *s, int small);

#endif
