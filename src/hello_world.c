#include <pebble.h>
#include "Glyph.h"

static Window *window;
static Layer *windowLayer;
static uint32_t* monacoBitmap;
static Font monaco;
static String timeString;

#define BENCHMARK

static void redrawWindowLayer( Layer *layer, GContext *context ) {
	#ifdef BENCHMARK
		uint16_t start = time_ms(NULL, NULL);
	#endif
	
	// Get the framebuffer
	GBitmap* frameBuffer = graphics_capture_frame_buffer( context );
	
	const int dx = ( ( monaco.glyph.size.w * monaco.scale ) + monaco.letterSpacing ) * 7;
	const int dy = ( monaco.glyph.size.h * monaco.scale );
	const int x = ( 180 - dx ) >> 1;
	
	clear( frameBuffer, (GRect) { (GPoint) { x, 90 - ( dy / 2 ) }, (GSize) { x + dx, dy } }, GColorWhite );
	drawText( timeString, &monaco, frameBuffer, (GPoint) { x, 90 - ( dy / 2 ) } );
	
	// Finally, release the framebuffer
	graphics_release_frame_buffer( context, frameBuffer );
	
	#ifdef BENCHMARK
		uint16_t finish = time_ms(NULL, NULL);
		APP_LOG(APP_LOG_LEVEL_INFO, "Frame time: %d", (int) finish - start);
	#endif
}

static void tickHandler( Time* timeValue, TimeUnits units_changed ) {
	strftime( timeString, 8, "%I:%M%p", timeValue );
	
	// Trigger layer_update_proc
	layer_mark_dirty( windowLayer );
}

static void init() {
	// Create a window and get information about the window
	window = window_create();
	windowLayer = window_get_root_layer( window );
	
	// Push the window, setting the window animation to 'true'
	window_stack_push( window, true );
	
	// Initialize font
	monaco = (Font) {
		.glyph = (Glyph) { NULL, (GSize) { 5, 9 } },
		.scale = 3,
		.letterSpacing = 3,
		.color = GColorVividCerulean
	};
	loadFont( &monaco, RESOURCE_ID_MONACO_9, 48 );
	
	// Initialize time display
	timeString = array( char, 8 );
	
	layer_set_update_proc( windowLayer, redrawWindowLayer );
	
	tick_timer_service_subscribe( MINUTE_UNIT, tickHandler );
	
	time_t temp = time( NULL );
	tickHandler( localtime( &temp ), MINUTE_UNIT );
}

static void deinit() {
	// Destroy the window
	window_destroy( window );
	
	destroyFont( &monaco );
	free( timeString );
}

int main() {
	init();
	app_event_loop();
	deinit();
}
