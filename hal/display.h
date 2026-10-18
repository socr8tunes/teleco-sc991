#ifndef TELECO_HAL_DISPLAY_H
#define TELECO_HAL_DISPLAY_H

#include <stdint.h>

/*
 * Monochrome dot-matrix display. The renderer draws into a framebuffer owned
 * by the caller; the HAL only pushes it to the panel.
 *
 * Framebuffer format: 1 bit per pixel, row-major, most significant bit = leftmost
 * pixel, each row padded to a whole byte. stride = (width + 7) / 8, size = stride * height.
 * A set bit means a dark (on) pixel.
 */
typedef struct hal_display_info {
    uint16_t width;
    uint16_t height;
} hal_display_info;

const hal_display_info *hal_display_get_info(void);

/* Send the whole framebuffer to the panel. */
void hal_display_present(const uint8_t *framebuffer);

/* Contrast level, 0 (lightest) .. 63 (darkest). Platforms clamp as needed. */
void hal_display_set_contrast(uint8_t level);

/* Switch the panel and its controller on or off (off = lowest power state). */
void hal_display_power(int on);

#endif /* TELECO_HAL_DISPLAY_H */
