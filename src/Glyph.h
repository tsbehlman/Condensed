#pragma once

#include <pebble.h>

typedef struct {
    uint8_t* bitmap;
    GPoint size;
} Glyph;

/**
 * Draws the given glyph on the given frame buffer at the given position.
 * @param glyph The glyph to draw
 * @param frameBuffer The framebuffer on which to draw the glyph
 * @param startPosition The position (top-left) at which to draw the glyph
 */
void drawGlyph( Glyph* glyph, GBitmap* frameBuffer, GPoint startPosition );