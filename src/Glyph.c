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
	uint16_t rowSize = glyph->size.x * scale;

	GPoint endPosition = {
		startPosition.x + rowSize,
		startPosition.y + glyph->size.y * scale
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

void drawFontGlyph( Font* font, uint8_t glyphIndex, GBitmap* frameBuffer, GPoint startPosition ) {
	uint16_t bitIndex = glyphIndex * font->glyph.size.x * font->glyph.size.y;
	drawBitmap( &font->glyph, font->glyph.bitmap + ( bitIndex >> 5 ), bitIndex & 0b11111, frameBuffer, startPosition, font->scale, font->color );
}

void drawText( String text, Font* font, GBitmap* frameBuffer, GPoint startPosition ) {
	const int dx = ( ( font->glyph.size.x * font->scale ) + font->letterSpacing );
	
	for( int i = 0; text[ i ] != '\0'; i++ ) {
		if( likely( text[ i ] > 32 ) ) {
			drawFontGlyph( font, text[ i ] - 33, frameBuffer, startPosition );
		}
		startPosition.x += dx;
	}
}