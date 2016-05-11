(function( Pebble, window ) {
	var STORAGE_SETTINGS_KEY = "CondensedFaceSettings";
	var CONFIG_URL = "http://tbehlman.com/pebble/condensed/settings-1.0.html";
	
	// Fetch existing settings from local storage
	var settingsString = localStorage.getItem( STORAGE_SETTINGS_KEY );
	
	Pebble.addEventListener( "showConfiguration", function() {
		// Open the configuration page and pass the existing settings as a parameter
		Pebble.openURL( CONFIG_URL + "?" + encodeURIComponent( settingsString ) );
	} );
	
	Pebble.addEventListener( "webviewclosed", function( e ) {
		if( e.response === null || e.response.length === 0 ) {
			return;
		}
		
		// Parse the response
		settingsString = decodeURIComponent( e.response );
		var settings = JSON.parse( settingsString );
		
		// Send to the watchapp
		Pebble.sendAppMessage( settings );
		
		// Persist the settings
		localStorage.setItem( STORAGE_SETTINGS_KEY, settingsString );
	} );
})( Pebble, window );