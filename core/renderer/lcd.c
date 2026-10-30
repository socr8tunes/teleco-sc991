#include "renderer/lcd.h"
#include <string.h>

#include "renderer/font_data.inc"

void lcd_unclip(lcd_t *l) { l->cx0 = 0; l->cy0 = 0; l->cx1 = LCD_W; l->cy1 = LCD_H; }
void lcd_clip(lcd_t *l, int x, int y, int w, int h)
{
    l->cx0 = (int16_t)x; l->cy0 = (int16_t)y; l->cx1 = (int16_t)(x + w); l->cy1 = (int16_t)(y + h);
}
void lcd_clear(lcd_t *l) { memset(l->px, 0, sizeof l->px); lcd_unclip(l); }

void lcd_line(lcd_t *l, int x0, int y0, int x1, int y1)
{
    int dx = x1 > x0 ? x1 - x0 : x0 - x1, dy = y1 > y0 ? y1 - y0 : y0 - y1;
    int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, err = dx - dy;
    for (;;) {
        int e2;
        lcd_pixel(l, x0, y0, 1);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void lcd_pixel(lcd_t *l, int x, int y, int on)
{
    uint8_t m;
    if (x < l->cx0 || y < l->cy0 || x >= l->cx1 || y >= l->cy1) return;
    if (x < 0 || y < 0 || x >= LCD_W || y >= LCD_H) return;
    m = (uint8_t)(0x80u >> (x & 7));
    if (on) l->px[y * LCD_STRIDE + (x >> 3)] |= m;
    else    l->px[y * LCD_STRIDE + (x >> 3)] &= (uint8_t)~m;
}

int lcd_get(const lcd_t *l, int x, int y)
{
    if (x < 0 || y < 0 || x >= LCD_W || y >= LCD_H) return 0;
    return (l->px[y * LCD_STRIDE + (x >> 3)] >> (7 - (x & 7))) & 1;
}

void lcd_hline(lcd_t *l, int x, int y, int w, int on)
{
    int i;
    for (i = 0; i < w; i++) lcd_pixel(l, x + i, y, on);
}

void lcd_vline(lcd_t *l, int x, int y, int h, int on)
{
    int i;
    for (i = 0; i < h; i++) lcd_pixel(l, x, y + i, on);
}

void lcd_fill(lcd_t *l, int x, int y, int w, int h, int on)
{
    int j;
    for (j = 0; j < h; j++) lcd_hline(l, x, y + j, w, on);
}

void lcd_invert(lcd_t *l, int x, int y, int w, int h)
{
    int i, j;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++) lcd_pixel(l, x + i, y + j, !lcd_get(l, x + i, y + j));
}

void lcd_frame(lcd_t *l, int x, int y, int w, int h)
{
    lcd_hline(l, x, y, w, 1);
    lcd_hline(l, x, y + h - 1, w, 1);
    lcd_vline(l, x, y, h, 1);
    lcd_vline(l, x + w - 1, y, h, 1);
}

static const uint8_t *main_glyph(unsigned char ch)
{
    static const uint8_t blank[5] = {0, 0, 0, 0, 0};
    if (ch >= 32 && ch <= 126) return FONT_ASCII[ch - 32];
    if (ch >= 0x80 && ch < 0x80 + FONT_SYM_COUNT) return FONT_SYM[ch - 0x80];
    return blank;
}

static const uint8_t *small_glyph(unsigned char ch)
{
    static const uint8_t blank[3] = {0, 0, 0};
    size_t i;
    if (ch >= 'a' && ch <= 'z') ch = (unsigned char)(ch - 32);
    for (i = 0; i < FONT_SMALL_COUNT; i++)
        if (FONT_SMALL[i].ch == ch) return FONT_SMALL[i].col;
    if (ch >= 0x80) return blank;
    return blank;
}

int lcd_char(lcd_t *l, int x, int y, unsigned char ch, int small)
{
    int cx, ry;
    if (small) {
        const uint8_t *g = small_glyph(ch);
        for (cx = 0; cx < 3; cx++)
            for (ry = 0; ry < SFONT_H; ry++)
                if ((g[cx] >> ry) & 1) lcd_pixel(l, x + cx, y + ry, 1);
        return SFONT_W;
    } else {
        const uint8_t *g = main_glyph(ch);
        for (cx = 0; cx < 5; cx++)
            for (ry = 0; ry < FONT_H; ry++)
                if ((g[cx] >> ry) & 1) lcd_pixel(l, x + cx, y + ry, 1);
        return FONT_W;
    }
}

int lcd_text_w(const char *s, int small)
{
    return (int)strlen(s) * (small ? SFONT_W : FONT_W);
}

int lcd_text(lcd_t *l, int x, int y, const char *s, int small)
{
    while (*s) x += lcd_char(l, x, y, (unsigned char)*s++, small);
    return x;
}

int lcd_text_right(lcd_t *l, int x_right, int y, const char *s, int small)
{
    return lcd_text(l, x_right - lcd_text_w(s, small), y, s, small);
}
