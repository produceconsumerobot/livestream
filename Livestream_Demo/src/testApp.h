#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "livestreamDemoSettings.h"


class testApp : public ofBaseApp{
	public:

		LivestreamDemoSettings settings;	// Settings that change from computer to computer
		static const int nSensors = LIVESTREAM_nSENSORS;

		// Arduino variables
		std::vector<ofArduino> arduinos;
		std::vector<bool> bSetupArduinos;	// flag variable for setting up arduino once
		int		sensorAnalogPin;
		ofSerial _serial;
		bool	blink13On;		// heartbeat blinks on digital channel 13 to verify arduino is working
		int		blinkCounter;	// counter for blinking

		// **** Setup sensor conversion variables **** //
		float	maxSensorDist;		// total range of sensor
		float	maxOuputDist;		// target output range of sensor
		float	maxAnalogValue;		// ADC bit resolution
		int		maxOutputValue;		// maximum value of output

		// Filter variables
		float maxSamplesToSmooth;
		float nSamplesToSmooth;
		std::vector<float> smoothData; // smoothed sensordata

		// MIDI variables
		int		midiPort;
		int		midiChannel;
		std::vector<int> midiIds;	// midiID that each sensor controls
		int		midiValue;
		bool	midiMapMode;		// Turns off midi signals in the main loop for mapping
		ofxMidiOut midiout;			// midi ouput

		bool	resendNote;			// for resending notes periodically
		int		noteResendCounter;	// for resending notes periodically
		int		noteResendTime;
		int		soundCheck;			// for debugging sound

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

		void setupArduino(ofArduino & ard);
		void analogPinChanged(const int & pinNum);


};
