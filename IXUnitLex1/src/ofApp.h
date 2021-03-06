#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
//#include "ofxGPIO.h"
#include "ThreadedLidarLite.h"
//#include "DS18B20.h"


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
		
		ofxUDPManager udpReceiver;
		ofxUDPManager udpSender;
        ofxUDPManager udpBroadcaster;
		string maestroIpAddress;
        string udpBroadcastAddress;
		
		int counter;
		unsigned long long blinkTimer;
		unsigned long long logTimer;
        unsigned long long blinkInterval;

		
		// PWM smoothing
		int nSamplesToSmooth;
		float maxSamplesToSmooth;
		float smoothPwm;
		int newPwmVal;
		float minSigStrength;		// For creating weighted distance average
		float fullSigStrength;	// For creating weighted distance average
		float minSigWeight;
		
		// Sound output
		ofSoundPlayer  pitchSound;
		//ofSoundPlayer  volSound;
        
        vector<ofSoundPlayer> soundPlayers;     // Multiple sound players used to support sustain of previously played notes
        int nSoundPlayers;
        int currentSoundPlayer;
		
		bool pitchBend; 
		bool volBend;
		
		// LIDAR Lite variables
		int fd; 		// initialization result
		int minDist; 		// cm
		int maxDist; 		// cm
		int noiseDist;	// cm
		
		string hostname;
		
		//DS18B20 tempSensor;
		//int nSensors;
		
		// Blink LED
		bool blinkLEDoutState;
		bool netLEDoutState;
		
		//GPIO* blinkLED;
		//GPIO* netLED;
        string blinkLED;
        string netLED;
        
		ThreadedLidarLite myLidarLite;
		
		int loopInterval;
		
		bool slaveMode; // Slave mode follows UDP commands to run
		unsigned long long nPacketsSent;
		int packetProtocolVersion;
		
		float volume;
		
		int debugTest;
        string softwareVersion;
		
		static const int NO_LIDAR = 1;
		static const int NO_TEMP = 2;
		static const int NO_GPIO = 3;
		static const int NO_SOUND = 4;
};
