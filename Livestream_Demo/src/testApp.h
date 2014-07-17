#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "livestreamDemoSettings.h"
#include "ofxGui.h"

#define LIVESTREAM_nData	10	//number of data points

class testApp : public ofBaseApp{
	public:
		//ofArduino ardi;
		//LivestreamPipe pipe;

		//std::vector< LivestreamPipe > pipes;

		// Data variabless
		//int dataReadRate;		//milliseconds
		//int dataMidiSendRate;	//milliseconds
		//int dataReadCounter;
		//int dataMidiSendCounter;
		//int dataSendHelper;

		//std::vector< float > data; 

		LivestreamDemoSettings settings;	// Settings that change from computer to computer
		
		int Fs; // sampling freq (Hz)
		unsigned long long elapsedTime; 

		// Arduino variables
		//std::vector<ofArduino> arduinos;
		//std::vector<bool> bSetupArduinos;	// flag variable for setting up arduino once
		int		sensorAnalogPin;
		//ofSerial _serial;
		bool	blink13On;		// heartbeat blinks on digital channel 13 to verify arduino is working
		int		blinkCounter;	// counter for blinking

		// **** Setup sensor conversion variables **** //
		float	maxSensorDist;			// total range of sensor
		float	maxOuputDist;			// target output range of sensor
		float	maxAnalogValue;			// ADC bit resolution
		int		maxMidiControlValue;	// maximum value of output

		// Filter variables
		float maxSamplesToSmooth;
		float nSamplesToSmooth;
		std::vector<float> smoothData; // smoothed sensordata



		// MIDI variables
		int		midiPort;
		int		midiChannel;
		//std::vector<int> midiIds;	// midiID that each sensor controls
		int		midiValue;
		bool	midiMapMode;		// Turns off midi signals in the main loop for mapping
		ofxMidiOut midiout;			// midi ouput
		ofxIntSlider testMidiIdSlider;
		ofxIntSlider testMidiChannelSlider;
		ofxPanel gui;
		int testMidiId;
		int testMidiChannel;
		int midiDistControl;
		int crossChannelSustainCounter;

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
		void testMidiIdSliderChanged(int & tempMidiId);
		void testMidiChannelSliderChanged(int & tempMidiChannel);


};
