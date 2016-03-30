#include <pebble.h>
#include "./Glyph.h"

static Window *s_window;
//static TextLayer *s_text_layer;

static void layer_update_proc( Layer *layer, GContext *context ) {
	// Get the framebuffer
	GBitmap* frameBuffer = graphics_capture_frame_buffer( context );
	
	uint8_t starData[5] = {
		0b00010000,
		0b00111000,
		0b11111110,
		0b01101100,
		0b10000010
	};
	
	Glyph starGlyph = { starData, (GPoint) { 8, 5 } };
	
	drawGlyph( &starGlyph, frameBuffer, (GPoint) { 90, 90 } );
	
	// Finally, release the framebuffer
	graphics_release_frame_buffer( context, frameBuffer );
}

static void init() {
	// Create a window and get information about the window
	s_window = window_create();
	Layer *window_layer = window_get_root_layer(s_window);
	/*GRect bounds = layer_get_bounds(window_layer);
	
	// Create a text layer and set the text
	s_text_layer = text_layer_create(bounds);
	text_layer_set_text(s_text_layer, "Hi, I'm a Pebble!");
  
	// Set the font and text alignment
	text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);*/
	
	layer_set_update_proc(window_layer, layer_update_proc);
	
	/*// Add the text layer to the window
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_text_layer));
	
	// Enable text flow and paging on the text layer, with a slight inset of 10, for round screens
	text_layer_enable_screen_text_flow_and_paging(s_text_layer, 10);*/
	
	// Push the window, setting the window animation to 'true'
	window_stack_push(s_window, true);
	
	// App Logging!
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

static void deinit() {
	// Destroy the text layer
	//text_layer_destroy(s_text_layer);
	
	// Destroy the window
	window_destroy(s_window);
}

int main() {
	init();
	app_event_loop();
	deinit();
}
