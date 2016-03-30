#include <pebble.h>
#include "./Glyph.h"

void drawGlyph( Glyph* glyph, GBitmap* frameBuffer, GPoint startPosition ) {
	uint8_t* bitmap = glyph->bitmap;
	uint8_t byte = *bitmap;
	
	uint32_t bitIndex = 0;
	
	GPoint endPosition = {
		startPosition.x + glyph->size.x,
		startPosition.y + glyph->size.y
	};
	
	for( int y = startPosition.y; y < endPosition.y; y++ ) {
		GBitmapDataRowInfo rowInfo = gbitmap_get_data_row_info( frameBuffer, y );
		
		for( int x = startPosition.x; x < endPosition.x; x++ ) {
			if( byte & 0b10000000 ) {
				rowInfo.data[ x ] = GColorFromRGB(0,0,0).argb;
			}
			
			if( bitIndex == 7 ) {
				bitmap++;
				byte = *bitmap;
				bitIndex = 0;
			}
			else {
				bitIndex++;
				byte = byte << 1;
			}
		}
	}
}