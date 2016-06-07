#include "ofMain.h"
#include "ofApp.h"

//#define LIVESTREAM_MAESTRO_NO_HEAD

//========================================================================
int main( ){

#ifdef LIVESTREAM_MAESTRO_NO_HEAD
	ofAppNoWindow window;
	ofSetupOpenGL(&window, 900, 700, OF_WINDOW);
	ofRunApp(new ofApp());
#else
	ofSetupOpenGL(900,700, OF_WINDOW);			// <-------- setup the GL context
#endif
	
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new ofApp());

}
