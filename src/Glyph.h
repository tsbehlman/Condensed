#pragma once

#include <pebble.h>
#include "cextensions.h"

typedef struct {
    uint32_t* bitmap;
    GSize size;
} GlyphSet;

typedef uint8_t ( *GlyphForCharacterProc )( char codePoint );

typedef struct {
    GlyphSet* glyphSet;
    uint8_t scale;
    uint8_t letterSpacing;
    GColor color;
    GlyphForCharacterProc glyphForCharacter;
} GlyphFont;

typedef void ( *DrawProc )( GBitmap* frameBuffer, GRect area );

typedef struct {
    GlyphFont* font;
    GPoint startPosition;
    String text;
    DrawProc drawBackground;
} GlyphLayer;


GlyphSet* GlyphSet_create( uint8_t resourceID, uint8_t numGlyphs, GSize glyphSize );

void GlyphSet_destroy( GlyphSet* glyphSet );

/**
 * Draws the a glyph on the given frame buffer at the given position.
 * @param glyph The glyphset which contains the glyph to draw
 * @param index the index of the glyph in the set to draw
 * @param frameBuffer The framebuffer on which to draw the glyph
 * @param startPosition The position (top-left) at which to draw the glyph
 * @param color The color to paint the glyph
 */
void GlyphSet_draw( GlyphSet* glyphSet, uint8_t glyphIndex, GBitmap* frameBuffer, GPoint startPosition, uint8_t scale, GColor color );

GlyphFont* GlyphFont_create( GlyphSet* glyphSet, uint8_t scale, uint8_t letterSpacing, GColor color, GlyphForCharacterProc glyphForCharacter );

void GlyphFont_destroy( GlyphFont* font );

void GlyphFont_drawGlyph( GlyphFont* font, uint8_t glyphIndex, GBitmap* frameBuffer, GPoint startPosition );

void GlyphFont_drawCharacter( GlyphFont* font, char codePoint, GBitmap* frameBuffer, GPoint startPosition );

GlyphLayer* GlyphLayer_create( GlyphFont* font, GPoint startPosition, uint8_t stringLength, DrawProc drawBackground );

void GlyphLayer_reset( GlyphLayer* layer );

void GlyphLayer_destroy( GlyphLayer* layer );

void GlyphLayer_draw( GlyphLayer* layer, GBitmap* frameBuffer, String text );