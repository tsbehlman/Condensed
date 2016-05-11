#include <pebble.h>
#include "Glyph.h"

void drawBitmap( uint32_t* bitmap,
						uint8_t bitIndex,
						GBitmap* frameBuffer,
						GRect area,
						uint8_t scale,
						GColor color ) {
	uint32_t data = ( *bitmap ) << bitIndex;
	uint16_t rowSize = area.size.w * scale;

	GPoint endPosition = {
		area.origin.x + rowSize,
		area.origin.y + area.size.h * scale
	};

	for( uint8_t y = area.origin.y; y < endPosition.y; y += scale ) {
		GBitmapDataRowInfo rowInfo = gbitmap_get_data_row_info( frameBuffer, y );
		
		for( uint8_t x = area.origin.x; x < endPosition.x; x += scale ) {
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
			memcpy( &nextRowInfo.data[ area.origin.x ], &rowInfo.data[ area.origin.x ], rowSize );
		}
	}
}

GlyphSet* GlyphSet_create( uint8_t resourceID, uint8_t numGlyphs, GSize glyphSize ) {
	GlyphSet* glyphSet = new( GlyphSet );
	glyphSet->size = glyphSize;
	ResHandle handle = resource_get_handle( resourceID );
	size_t size = ( numGlyphs * glyphSize.w * glyphSize.h + 32 ) >> 3;
	uint8_t *data = Array( uint8_t, size );
	resource_load( handle, data, size );
	glyphSet->bitmap = (uint32_t*) data;
	return glyphSet;
}

void GlyphSet_destroy( GlyphSet* glyphSet ) {
	free( glyphSet->bitmap );
	free( glyphSet );
}

void GlyphSet_draw( GlyphSet* glyphSet, uint8_t glyphIndex, GBitmap* frameBuffer, GPoint startPosition, uint8_t scale, GColor color ) {
	uint16_t bitIndex = glyphIndex * glyphSet->size.w * glyphSet->size.h;
	drawBitmap( glyphSet->bitmap + ( bitIndex >> 5 ), bitIndex & 0b11111, frameBuffer, (GRect) { startPosition, glyphSet->size }, scale, color );
}

void GlyphFont_drawGlyph( GlyphFont* font, uint8_t glyphIndex, GBitmap* frameBuffer, GPoint startPosition ) {
	GlyphSet_draw( font->glyphSet, glyphIndex, frameBuffer, startPosition, font->scale, font->color );
}

void GlyphFont_drawCharacter( GlyphFont* font, char codePoint, GBitmap* frameBuffer, GPoint startPosition ) {
	GlyphSet_draw( font->glyphSet, font->glyphForCharacter( codePoint ), frameBuffer, startPosition, font->scale, font->color );
}

GlyphLayer* GlyphLayer_create( GlyphFont* font, GPoint startPosition, uint8_t stringLength, DrawProc drawBackground ) {
	GlyphLayer* layer = new( GlyphLayer );
	layer->font = font;
	layer->startPosition = startPosition;
	layer->text = String( stringLength );
	layer->drawBackground = drawBackground;
	return layer;
}

void GlyphLayer_reset( GlyphLayer* layer ) {
	char* textPointer = layer->text;
	
	while( *textPointer != '\0' ) {
		*textPointer = '\0';
		textPointer++;
	}
}

void GlyphLayer_destroy( GlyphLayer* layer ) {
	free( layer->text );
	free( layer );
}

void GlyphLayer_draw( GlyphLayer* layer, GBitmap* frameBuffer, String text ) {
	GPoint startPosition = layer->startPosition;
	const GSize glyphSize = { layer->font->glyphSet->size.w * layer->font->scale, layer->font->glyphSet->size.h * layer->font->scale };
	const int dx = ( glyphSize.w + layer->font->letterSpacing );
	
	for( int i = 0; text[ i ] != '\0'; i++ ) {
		if( layer->text[ i ] != text[ i ] ) {
			uint8_t glyphIndex = layer->font->glyphForCharacter( text[ i ] );
			if( likely( glyphIndex < 255 ) ) {
				layer->drawBackground( frameBuffer, (GRect) { startPosition, glyphSize } );
				layer->text[ i ] = text[ i ];
				GlyphFont_drawGlyph( layer->font, glyphIndex, frameBuffer, startPosition );
			}
		}
		startPosition.x += dx;
	}
}