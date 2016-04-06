#include <pebble.h>
#include "Glyph.h"

static Window *window;
static Layer *windowLayer;
static uint32_t* monacoBitmap;
static Font* monaco;
static GlyphLayer* timeLayer;
static GlyphLayer* batteryLayer;
static String timeString;
static String timeFormat;
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
	
	// Finally, release the framebuffer
	graphics_release_frame_buffer( context, frameBuffer );
	
	#ifdef BENCHMARK
		uint16_t finish = time_ms(NULL, NULL);
		APP_LOG(APP_LOG_LEVEL_INFO, "Frame time: %d", (int) finish - start);
	#endif
}

static void tickHandler( Time* timeValue, TimeUnits units_changed ) {
	strftime( timeString, timeFormatLength, timeFormat, timeValue );
	
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

static void init() {
	// Create a window and get information about the window
	window = window_create();
	windowLayer = window_get_root_layer( window );
	
	// Push the window, setting the window animation to 'true'
	window_stack_push( window, true );
	
	// Initialize time display
	if( clock_is_24h_style() ) {
		timeFormat = "%H:%M";
		timeFormatLength = 6;
	}
	else {
		timeFormat = "%I:%M%p";
		timeFormatLength = 8;
	}
	timeString = String( timeFormatLength );
	
	// Initialize font
	monaco = Font_create( RESOURCE_ID_MONACO_9, 48, (GSize) { 5, 9 } );
	monaco->scale = 3;
	monaco->letterSpacing = 3;
	monaco->color = GColorBlack;
	
	// Initialize GlyphLayer
	const int dx = ( ( monaco->glyph.size.w * monaco->scale ) + monaco->letterSpacing );
	const int dy = ( monaco->glyph.size.h * monaco->scale ) + monaco->letterSpacing;
	timeLayer = GlyphLayer_create( monaco, (GPoint) { ( 180 - dx * ( timeFormatLength - 1 ) ) >> 1, 90 - ( dy / 2 ) }, timeFormatLength, drawBackground );
	
	batteryString = String( 5 );
	batteryLayer = GlyphLayer_create( monaco, (GPoint) { ( 180 - dx * 4 ) >> 1, 90 + ( dy / 2 ) }, 5, drawBackground );
	
	layer_set_update_proc( windowLayer, redrawWindowLayer );
	
	tick_timer_service_subscribe( MINUTE_UNIT, tickHandler );
	battery_state_service_subscribe( batteryHandler );
	
	time_t temp = time( NULL );
	tickHandler( localtime( &temp ), MINUTE_UNIT );
	batteryHandler( battery_state_service_peek() );
}

static void deinit() {
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	
	// Destroy the window
	window_destroy( window );
	
	GlyphLayer_destroy( timeLayer );
	GlyphLayer_destroy( batteryLayer );
	Font_destroy( monaco );
	free( timeString );
}

int main() {
	init();
	app_event_loop();
	deinit();
}
