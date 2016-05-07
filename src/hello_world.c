#include <pebble.h>
#include "Glyph.h"

//#define PROFILE

//#define BENCHMARK
#ifdef BENCHMARK
	#include "benchmark.h"
#endif

static Window *window;
static Layer *windowLayer;
static GlyphSet* universGlyphs;
static GlyphFont univers;
static GlyphLayer* timeLayer;
static String timeString;
static int32_t drawnBatteryAngle = 0;
static int32_t batteryAngle;
static bool firstRender = true;

#define SCREEN_RECT ((GRect) { (GPoint) { 0, 0 }, (GSize) { 180, 180 } })

#define BACKGROUND_COLOR GColorBlack
#define FOREGROUND_COLOR GColorWhite
#define BATTERY_RING_THICKNESS 8

#define TIME_FORMAT "00:00"
#define TIME_FORMAT_HOUR_POSITION 0
#define TIME_FORMAT_MINUTE_POSITION 3
#define TIME_FORMAT_LENGTH 5

static void drawBackground( GBitmap* frameBuffer, GRect area ) {
	#ifdef PROFILE
		APP_LOG(APP_LOG_LEVEL_INFO, "Drawing background of %dx%d at %d,%d", area.size.w, area.size.h, area.origin.x, area.origin.y);
	#endif
	
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
	#ifdef PROFILE
		time_t temp = time( NULL );
		Time* tm = localtime( &temp );
		APP_LOG(APP_LOG_LEVEL_INFO, "Screen redraw at : %02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
	#endif
	
	#ifdef BENCHMARK
		startTimer();
	#endif
	
	// Disable antialiasing in order to draw and erase the battery ring cleanly
	graphics_context_set_antialiased( context, false );
	
	// Get the framebuffer
	GBitmap* frameBuffer = graphics_capture_frame_buffer( context );
	
	// Draw the background if necessary
	if( firstRender ) {
		#ifdef PROFILE
			APP_LOG(APP_LOG_LEVEL_INFO, "Initial render");
		#endif
		drawBackground( frameBuffer, SCREEN_RECT );
		firstRender = false;
	}
	
	// Update the time display
	GlyphLayer_draw( timeLayer, frameBuffer, timeString );
	
	// Release the framebuffer
	graphics_release_frame_buffer( context, frameBuffer );
	
	// Update the battery ring if necessary
	if( batteryAngle < drawnBatteryAngle ) {
		// Battery has drained, erase a segment of the ring
		graphics_context_set_fill_color( context, BACKGROUND_COLOR );
		graphics_fill_radial( context, SCREEN_RECT, GOvalScaleModeFitCircle, BATTERY_RING_THICKNESS, batteryAngle, drawnBatteryAngle );
		drawnBatteryAngle = batteryAngle;
		
		#ifdef PROFILE
			APP_LOG(APP_LOG_LEVEL_INFO, "Clearing battery ring");
		#endif
	}
	else if( batteryAngle > drawnBatteryAngle ) {
		// Battery has charged, add a segment to the ring
		graphics_context_set_fill_color( context, FOREGROUND_COLOR );
		graphics_fill_radial( context, SCREEN_RECT, GOvalScaleModeFitCircle, BATTERY_RING_THICKNESS, drawnBatteryAngle, batteryAngle );
		drawnBatteryAngle = batteryAngle;
		
		#ifdef PROFILE
			APP_LOG(APP_LOG_LEVEL_INFO, "Drawing battery ring");
		#endif
	}
	
	#ifdef BENCHMARK
		APP_LOG(APP_LOG_LEVEL_INFO, "Frame time: %lu", endTimer());
	#endif
}

static void setTwoDigitNumber( uint8_t number, String outputString ) {
	if( number >= 10 ) {
		div_t division = div( number, 10 );
		outputString[ 0 ] = 48 | division.quot;
		number = division.rem;
	}
	else {
		outputString[ 0 ] = '0';
	}
	outputString[ 1 ] = 48 | number;
}

static void setTime( Time* timeValue, TimeUnits unitsChanged ) {
	if( unitsChanged & HOUR_UNIT ) {
		uint8_t hour = timeValue->tm_hour;
		if( !clock_is_24h_style() ) {
			if( hour > 12 ) {
				hour -= 12;
			}
			else if( hour == 0 ) {
				hour = 12;
			}
		}
		
		setTwoDigitNumber( hour, timeString + TIME_FORMAT_HOUR_POSITION );
	}
	if( unitsChanged & MINUTE_UNIT ) {
		setTwoDigitNumber( timeValue->tm_min, timeString + TIME_FORMAT_MINUTE_POSITION );
	}
}

static void tickHandler( Time* timeValue, TimeUnits unitsChanged ) {
	setTime( timeValue, unitsChanged );
	
	// Trigger layer_update_proc
	layer_mark_dirty( windowLayer );
}

static void setBattery( BatteryChargeState chargeState ) {
	batteryAngle = TRIG_MAX_ANGLE * chargeState.charge_percent / 100;
}

static void batteryHandler( BatteryChargeState chargeState ) {
	setBattery( chargeState );
	
	// The battery event seems to fire once per minute regardless of whether there is a battery state change
	if( batteryAngle != drawnBatteryAngle ) {
		// Trigger layer_update_proc
		layer_mark_dirty( windowLayer );
	}
}

static uint8_t asciiToGlyphIndex( char codePoint ) {
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
	
	// Initialize font
	universGlyphs = GlyphSet_create( RESOURCE_ID_UNIVERS, 11, (GSize) { 18, 80 } );
	univers = (GlyphFont) {
		.glyphSet = universGlyphs,
		.scale = 1,
		.letterSpacing = 5,
		.color = FOREGROUND_COLOR,
		.glyphForCharacter = asciiToGlyphIndex
	};
	
	// Initialize GlyphLayers
	const int dx = ( univers.glyphSet->size.w * univers.scale ) + univers.letterSpacing;
	const int dy = ( univers.glyphSet->size.h * univers.scale );
	
	timeLayer = GlyphLayer_create( &univers, (GPoint) { ( 180 - ( dx * TIME_FORMAT_LENGTH - univers.letterSpacing ) ) >> 1, 90 - ( dy / 2 ) }, TIME_FORMAT_LENGTH, drawBackground );
	
	// Define draw procedure
	layer_set_update_proc( windowLayer, redrawWindowLayer );
	
	// Draw initial display
	time_t temp = time( NULL );
	setTime( localtime( &temp ), HOUR_UNIT | MINUTE_UNIT );
	setBattery( battery_state_service_peek() );
	
	// Subscribe to services
	tick_timer_service_subscribe( MINUTE_UNIT, tickHandler );
	battery_state_service_subscribe( batteryHandler );
}

static void deinit() {
	// Unsubscribe from services
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	
	// Free memory
	window_destroy( window );
	GlyphLayer_destroy( timeLayer );
	GlyphSet_destroy( universGlyphs );
}

int main() {
	init();
	app_event_loop();
	deinit();
}
