#pragma once

#include "ofMain.h"
#include "ofxGPIO.h"
#include "lidarLite.h"
#include "DS18B20.h"


class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void exit();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		string getDateTimeString();
		
		int counter;
		unsigned long long blinkTimer;
		unsigned long long logTimer;
		bool gpioOutLedState;
		bool gpioBlinkLedState;
		string gpioPwmInState;
		
		//GPIO* gpioOutLed;
		//GPIO* gpioBlinkLed;
		//GPIO* gpioPwmIn;
		
		// PWM smoothing
		int nSamplesToSmooth;
		float maxSamplesToSmooth;
		float smoothPwm;
		int newPwmVal;
		
		// Sound output
		ofSoundPlayer  pitchSound;
		ofSoundPlayer  volSound;
		
		bool pitchBend; 
		bool volBend;
		
		// LIDAR Lite variables
		int fd; 		// initialization result
		int minDist; 	// cm
		int maxDist; 	// cm
		
		string hostname;
		
		DS18B20 tempSensor;
		int nSensors;
		
		// Blink LED
		bool gpio15outState;
		bool gpio21outState;
		
		GPIO* gpio15;
		GPIO* gpio21;
};
