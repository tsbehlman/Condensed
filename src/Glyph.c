#include <pebble.h>
#include "Glyph.h"

void drawBitmap( Glyph* glyph,
						uint32_t* bitmap,
						uint8_t bitIndex,
						GBitmap* frameBuffer,
						GPoint startPosition,
						uint8_t scale,
						GColor color ) {
	uint32_t data = ( *bitmap ) << bitIndex;
	uint16_t rowSize = glyph->size.w * scale;

	GPoint endPosition = {
		startPosition.x + rowSize,
		startPosition.y + glyph->size.h * scale
	};

	for( uint8_t y = startPosition.y; y < endPosition.y; y += scale ) {
		GBitmapDataRowInfo rowInfo = gbitmap_get_data_row_info( frameBuffer, y );
		
		for( uint8_t x = startPosition.x; x < endPosition.x; x += scale ) {
			if( data & ( 1 << 31 ) ) {
				memset( &rowInfo.data[ x ], color.argb, scale );
			}
		
			if( bitIndex == 31 ) {
				bitIndex = 0;
				bitmap++;
				data = *bitmap;
			}
			else {
				bitIndex++;
				data = data << 1;
			}
		}
		
		for( int i = 1; i < scale; i++ ) {
			GBitmapDataRowInfo nextRowInfo = gbitmap_get_data_row_info( frameBuffer, y + i );
			memcpy( &nextRowInfo.data[ startPosition.x ], &rowInfo.data[ startPosition.x ], rowSize );
		}
	}
}

void drawGlyph( Glyph* glyph, GBitmap* frameBuffer, GPoint startPosition, GColor color ) {
	drawBitmap( glyph, glyph->bitmap, 0, frameBuffer, startPosition, 1, color );
}

Font* Font_create( uint8_t bitmapID, uint8_t numGlyphs, GSize glyphSize ) {
	Font* font = new( Font );
	font->glyph.size = glyphSize;
	ResHandle handle = resource_get_handle( bitmapID );
	size_t size = ( numGlyphs * glyphSize.w * glyphSize.h + 32 ) >> 3;
	uint8_t *data = Array( uint8_t, size );
	resource_load( handle, data, size );
	font->glyph.bitmap = (uint32_t*) data;
	return font;
}

void Font_destroy( Font* font ) {
	free( font->glyph.bitmap );
	free( font );
}

void Font_drawGlyph( Font* font, uint8_t glyphIndex, GBitmap* frameBuffer, GPoint startPosition ) {
	uint16_t bitIndex = glyphIndex * font->glyph.size.w * font->glyph.size.h;
	drawBitmap( &font->glyph, font->glyph.bitmap + ( bitIndex >> 5 ), bitIndex & 0b11111, frameBuffer, startPosition, font->scale, font->color );
}

GlyphLayer* GlyphLayer_create( Font* font, GPoint startPosition, uint8_t stringLength, DrawProc drawBackground ) {
	GlyphLayer* layer = new( GlyphLayer );
	layer->font = font;
	layer->startPosition = startPosition;
	layer->text = String( stringLength );
	layer->drawBackground = drawBackground;
	return layer;
}

void GlyphLayer_destroy( GlyphLayer* layer ) {
	free( layer->text );
	free( layer );
}

void GlyphLayer_draw( GlyphLayer* layer, GBitmap* frameBuffer, String text ) {
	GPoint startPosition = layer->startPosition;
	const GSize glyphSize = { layer->font->glyph.size.w * layer->font->scale, layer->font->glyph.size.h * layer->font->scale };
	const int dx = ( glyphSize.w + layer->font->letterSpacing );
	
	for( int i = 0; text[ i ] != '\0'; i++ ) {
		if( text[ i ] > 32 && layer->text[ i ] != text[ i ] ) {
			layer->drawBackground( frameBuffer, (GRect) { startPosition, glyphSize } );
			layer->text[ i ] = text[ i ];
			Font_drawGlyph( layer->font, text[ i ] - 33, frameBuffer, startPosition );
		}
		startPosition.x += dx;
	}
}