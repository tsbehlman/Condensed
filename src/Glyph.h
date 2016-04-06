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

typedef void ( *DrawProc )( GBitmap* frameBuffer, GRect area );

typedef struct {
    Font* font;
    GPoint startPosition;
    String text;
    DrawProc drawBackground;
} GlyphLayer;

/**
 * Draws the given glyph on the given frame buffer at the given position.
 * @param glyph The glyph to draw
 * @param frameBuffer The framebuffer on which to draw the glyph
 * @param startPosition The position (top-left) at which to draw the glyph
 * @param color The color to paint the glyph
 */
void drawGlyph( Glyph* glyph, GBitmap* frameBuffer, GPoint startPosition, GColor color );

void Font_drawGlyph( Font* font, uint8_t glyphIndex, GBitmap* frameBuffer, GPoint startPosition );

Font* Font_create( uint8_t bitmapID, uint8_t numGlyphs, GSize glyphSize );

void Font_destroy( Font* font );

GlyphLayer* GlyphLayer_create( Font* font, GPoint startPosition, uint8_t stringLength, DrawProc drawBackground );

void GlyphLayer_destroy( GlyphLayer* layer );

void GlyphLayer_draw( GlyphLayer* layer, GBitmap* frameBuffer, String text );