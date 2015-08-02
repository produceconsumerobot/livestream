#pragma once

#include "ofMain.h"
#include "ofxGPIO.h"

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		int counter;
		unsigned long long sysMicros;
		bool gpio15outState;
		bool gpio21outState;
		
		GPIO* gpio15;
		GPIO* gpio21;
};
