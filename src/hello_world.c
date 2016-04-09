#include <pebble.h>
#include "Glyph.h"

static Window *window;
static Layer *windowLayer;
static GlyphSet* monacoGlyphs;
static GlyphFont monaco;
static GlyphLayer* timeLayer;
static GlyphLayer* batteryLayer;
static String timeString;
static uint8_t timeFormatLength;
static String batteryString;

#define BACKGROUND_COLOR GColorWhiteARGB8

//#define BENCHMARK

static void redrawWindowLayer( Layer *layer, GContext *context ) {
	#ifdef BENCHMARK
		uint16_t start = time_ms(NULL, NULL);
	#endif
	
	// Get the framebuffer
	GBitmap* frameBuffer = graphics_capture_frame_buffer( context );
	
	GlyphLayer_draw( timeLayer, frameBuffer, timeString );
	GlyphLayer_draw( batteryLayer, frameBuffer, batteryString );
	
	// Release the framebuffer
	graphics_release_frame_buffer( context, frameBuffer );
	
	#ifdef BENCHMARK
		uint16_t finish = time_ms(NULL, NULL);
		APP_LOG(APP_LOG_LEVEL_INFO, "Frame time: %d", (int) finish - start);
	#endif
}

static void setTwoDigitNumber( uint8_t number, String outputString ) {
	if( number >= 10 ) {
		outputString[ 0 ] = 48 + ( number / 10 );
		number = number % 10;
	}
	else {
		outputString[ 0 ] = '0';
	}
	outputString[ 1 ] = 48 + number;
}

static void tickHandler( Time* timeValue, TimeUnits unitsChanged ) {
	switch( unitsChanged ) {
		case HOUR_UNIT: {
			uint8_t hour = timeValue->tm_hour;
			if( !clock_is_24h_style() ) {
				if( hour < 12 ) {
					timeString[ 5 ] = 'A';
					if( hour == 0 ) {
						hour = 12;
					}
				}
				else {
					hour -= 12;
					timeString[ 5 ] = 'P';
				}
			}
			
			setTwoDigitNumber( hour, timeString );
		}
		case MINUTE_UNIT: {
			setTwoDigitNumber( timeValue->tm_min, timeString + 3 );
		}
		default: {
			break;
		}
	}
	
	// Trigger layer_update_proc
	layer_mark_dirty( windowLayer );
}

static void batteryHandler( BatteryChargeState charge_state ) {
	snprintf( batteryString, 5, "% 3d%%", charge_state.charge_percent );

	// Trigger layer_update_proc
	layer_mark_dirty( windowLayer );
}

static void drawBackground( GBitmap* frameBuffer, GRect area ) {
	uint16_t y = area.origin.y;
	
	GBitmapDataRowInfo rowInfo = gbitmap_get_data_row_info( frameBuffer, y );
	
	memset( &rowInfo.data[ area.origin.x ], BACKGROUND_COLOR, area.size.w );
	
	const uint16_t maxY = y + area.size.h;
	
	for( y++; y < maxY; y++ ) {
		GBitmapDataRowInfo nextRowInfo = gbitmap_get_data_row_info( frameBuffer, y );
		memcpy( &nextRowInfo.data[ area.origin.x ], &rowInfo.data[ area.origin.x ], area.size.w );
	}
}

static uint8_t asciiNonWhitespaceToGlyphIndex( char codePoint ) {
	return codePoint - 33;
}

static void init() {
	// Create a window and get information about the window
	window = window_create();
	windowLayer = window_get_root_layer( window );
	
	// Push the window, setting the window animation to 'true'
	window_stack_push( window, true );
	
	// Initialize time display format
	if( clock_is_24h_style() ) {
		timeString = "00:00";
		timeFormatLength = 5;
	}
	else {
		timeString = "00:00AM";
		timeFormatLength = 7;
	}
	
	// Initialize battery display format
	batteryString = "000%";
	
	// Initialize font
	monacoGlyphs = GlyphSet_create( RESOURCE_ID_MONACO_9, 48, (GSize) { 5, 9 } );
	monaco = (GlyphFont) {
		.glyphSet = monacoGlyphs,
		.scale = 3,
		.letterSpacing = 3,
		.color = GColorBlack,
		.glyphForCharacter = asciiNonWhitespaceToGlyphIndex
	};
	
	// Initialize GlyphLayers
	const int dx = ( ( monaco.glyphSet->size.w * monaco.scale ) + monaco.letterSpacing );
	const int dy = ( monaco.glyphSet->size.h * monaco.scale ) + monaco.letterSpacing;
	
	timeLayer = GlyphLayer_create( &monaco, (GPoint) { ( 180 - ( dx * ( timeFormatLength ) - monaco.letterSpacing ) ) >> 1, 90 - ( dy / 2 ) }, timeFormatLength, drawBackground );
	
	batteryLayer = GlyphLayer_create( &monaco, (GPoint) { ( 180 - ( dx * 4 - monaco.letterSpacing ) ) >> 1, 90 + ( dy / 2 ) }, 5, drawBackground );
	
	// Define draw procedure
	layer_set_update_proc( windowLayer, redrawWindowLayer );
	
	// Subscribe to services
	tick_timer_service_subscribe( MINUTE_UNIT, tickHandler );
	battery_state_service_subscribe( batteryHandler );
	
	// Draw initial display
	time_t temp = time( NULL );
	tickHandler( localtime( &temp ), HOUR_UNIT );
	batteryHandler( battery_state_service_peek() );
}

static void deinit() {
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	
	// Destroy the window
	window_destroy( window );
	
	GlyphLayer_destroy( timeLayer );
	GlyphLayer_destroy( batteryLayer );
	GlyphSet_destroy( monacoGlyphs );
}

int main() {
	init();
	app_event_loop();
	deinit();
}
