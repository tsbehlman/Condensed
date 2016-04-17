#include <pebble.h>
#include "Glyph.h"

static Window *window;
static Layer *windowLayer;
static GlyphSet* universGlyphs;
static GlyphFont univers;
static GlyphLayer* timeLayer;
static String timeString;
static uint8_t timeFormatLength;
static int32_t drawnBatteryAngle = 0;
static int32_t batteryAngle;
static bool firstRender = true;

#define screenRect ((GRect) { (GPoint) { 0, 0 }, (GSize) { 180, 180 } })

#define BACKGROUND_COLOR GColorBlack
#define FOREGROUND_COLOR GColorWhite
#define BATTERY_RING_THICKNESS 8

//#define BENCHMARK

static void drawBackground( GBitmap* frameBuffer, GRect area ) {
	uint16_t y = area.origin.y;
	const uint16_t maxY = y + area.size.h;

	for( ; y < maxY; y++ ) {
		GBitmapDataRowInfo nextRowInfo = gbitmap_get_data_row_info( frameBuffer, y );
		uint8_t startX = max( area.origin.x, nextRowInfo.min_x );
		uint8_t endX = min( area.origin.x + area.size.w, nextRowInfo.max_x + 1 );
		memset( &nextRowInfo.data[ startX ], BACKGROUND_COLOR.argb, endX - startX );
	}
}

static void redrawWindowLayer( Layer *layer, GContext *context ) {
	#ifdef BENCHMARK
		uint16_t start = time_ms(NULL, NULL);
	#endif
	
	graphics_context_set_antialiased( context, false );
	
	// Get the framebuffer
	GBitmap* frameBuffer = graphics_capture_frame_buffer( context );
	
	if( firstRender ) {
		drawBackground( frameBuffer, screenRect );
		firstRender = false;
	}
	
	GlyphLayer_draw( timeLayer, frameBuffer, timeString );
	
	// Release the framebuffer
	graphics_release_frame_buffer( context, frameBuffer );
	
	// Draw the battery ring if necessary
	if ( drawnBatteryAngle < batteryAngle ) {
		graphics_context_set_fill_color( context, FOREGROUND_COLOR );
		graphics_fill_radial( context, screenRect, GOvalScaleModeFitCircle, BATTERY_RING_THICKNESS, 0, batteryAngle );
		drawnBatteryAngle = batteryAngle;
	}
	else if( drawnBatteryAngle > batteryAngle ) {
		graphics_context_set_fill_color( context, BACKGROUND_COLOR );
		graphics_fill_radial( context, screenRect, GOvalScaleModeFitCircle, BATTERY_RING_THICKNESS, batteryAngle, drawnBatteryAngle );
		drawnBatteryAngle = batteryAngle;
	}
	
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
	if( unitsChanged & HOUR_UNIT ) {
		uint8_t hour = timeValue->tm_hour;
		if( !clock_is_24h_style() ) {
			if( hour < 12 ) {
				if( hour == 0 ) {
					hour = 12;
				}
			}
			else {
				hour -= 12;
			}
		}
		
		setTwoDigitNumber( hour, timeString );
	}
	if( unitsChanged & MINUTE_UNIT ) {
		setTwoDigitNumber( timeValue->tm_min, timeString + 3 );
	}
		
	// Trigger layer_update_proc
	layer_mark_dirty( windowLayer );
}

static void batteryHandler( BatteryChargeState charge_state ) {
	batteryAngle = TRIG_MAX_ANGLE * 100 / 100;
	
	// Trigger layer_update_proc
	layer_mark_dirty( windowLayer );
}

static uint8_t asciiNonWhitespaceToGlyphIndex( char codePoint ) {
	if( codePoint < 48 || codePoint > 58 ) {
		return 255;
	}
	else {
		return codePoint - 48;
	}
}

static void init() {
	// Create a window and get information about the window
	window = window_create();
	windowLayer = window_get_root_layer( window );
	
	// Push the window, setting the window animation to 'true'
	window_stack_push( window, true );
		
	// Initialize time display format
	timeString = "00:00";
	timeFormatLength = 5;
	
	// Initialize font
	universGlyphs = GlyphSet_create( RESOURCE_ID_UNIVERS, 11, (GSize) { 18, 80 } );
	univers = (GlyphFont) {
		.glyphSet = universGlyphs,
		.scale = 1,
		.letterSpacing = 3,
		.color = FOREGROUND_COLOR,
		.glyphForCharacter = asciiNonWhitespaceToGlyphIndex
	};
	
	// Initialize GlyphLayers
	const int dx = ( ( univers.glyphSet->size.w * univers.scale ) + univers.letterSpacing );
	const int dy = ( univers.glyphSet->size.h * univers.scale ) + univers.letterSpacing;
	
	timeLayer = GlyphLayer_create( &univers, (GPoint) { ( 180 - ( dx * ( timeFormatLength ) - univers.letterSpacing ) ) >> 1, 90 - ( dy / 2 ) }, timeFormatLength, drawBackground );
	
	// Define draw procedure
	layer_set_update_proc( windowLayer, redrawWindowLayer );
	
	// Subscribe to services
	tick_timer_service_subscribe( MINUTE_UNIT, tickHandler );
	battery_state_service_subscribe( batteryHandler );
	
	// Draw initial display
	time_t temp = time( NULL );
	tickHandler( localtime( &temp ), HOUR_UNIT | MINUTE_UNIT );
	batteryHandler( battery_state_service_peek() );
}

static void deinit() {
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	
	// Destroy the window
	window_destroy( window );
	
	GlyphLayer_destroy( timeLayer );
	GlyphSet_destroy( universGlyphs );
}

int main() {
	init();
	app_event_loop();
	deinit();
}
