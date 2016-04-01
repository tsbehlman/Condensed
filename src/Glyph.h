#pragma once

#include <pebble.h>
#include "cextensions.h"

typedef struct {
    uint32_t* bitmap;
    GSize size;
} Glyph;

typedef struct {
    Glyph glyph;
    uint8_t scale;
    uint8_t letterSpacing;
    GColor color;
} Font;

/**
 * Draws the given glyph on the given frame buffer at the given position.
 * @param glyph The glyph to draw
 * @param frameBuffer The framebuffer on which to draw the glyph
 * @param startPosition The position (top-left) at which to draw the glyph
 * @param color The color to paint the glyph
 */
void drawGlyph( Glyph* glyph, GBitmap* frameBuffer, GPoint startPosition, GColor color );

void drawFontGlyph( Font* font, uint8_t glyphIndex, GBitmap* frameBuffer, GPoint startPosition );

void drawText( String text, Font* font, GBitmap* frameBuffer, GPoint startPosition );

void clear( GBitmap* frameBuffer, GRect area, GColor backgroundColor );

void loadFont( Font* font, uint8_t bitmapID, uint8_t numGlyphs );

void destroyFont( Font* font );