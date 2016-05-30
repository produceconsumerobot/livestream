#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxGui.h"
#include "ofxInputField.h"
#include "LivestreamInteractionUnit.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		
		void loggingOnChanged(bool & _loggingOn);
		void setTemperature(int temp);

		ofxUDPManager udpBroadcaster;
		ofxUDPManager udpReceiver;
		unsigned long long nPacketsSent;
		int packetProtocolVersion;

		//string maestroIpAddress;

		vector<LivestreamInteractionUnit> interXUnit;
		int nInterXUnits;

		ofTrueTypeFont  mono;
		ofTrueTypeFont  monosm;
		vector<ofPoint> stroke;

		bool testLED;

		// Settings
		//string waterDataAddress;
		//string waterDataFilesLocation;
		//string soundFilesLocation;
		//Range volumeRange;
		//int waterDataReadInterval;
		//int distanceReadInterval;
		//int notePlayInterval;
		//Range signalStrengthRange;
		//float minSignalWeight;
		//int noiseDistance;
		//int maxDistSamplesToSmooth;

		// GUI
		ofxPanel defaultSettingsPanel;

		ofxTextField waterDataFilesLocation;
		ofxTextField soundFilesLocation;
		ofxFloatField volumeMin;
		ofxFloatField volumeMax;
		ofxIntField waterDataReadInterval;
		ofxIntField distanceReadInterval;
		ofxIntField notePlayInterval;
		ofxFloatField signalStrengthMin;
		ofxFloatField signalStrengthMax;
		ofxFloatField minSignalWeight;
		ofxIntField noiseDistance;
		ofxIntField maxDistSamplesToSmooth;

		ofxPanel maestroPanel;
		ofxTextField maestroIpAddress;
		ofxLabel softwareVersion;
		ofxLabel currentDateTime;
		ofxLabel lastStartupDateTime;
		ofxFloatSlider guiTemperature;		// Most recent temperature measurement;
		ofxFloatSlider guiLowTemperature;	// Lowest recorded temperature
		ofxFloatSlider guiHighTemperature;	// Highest recorded temperature
		ofxIntField frameRate;
		ofxToggle soundOn;
		ofxToggle loggingOn;

		int temperature;			// Most recent temperature measurement;
		int lowTemperature;			// Lowest recorded temperature
		int highTemperature;		// Highest recorded temperature

		int panelWidth;
		int panelRowHeight;

		bool udpSendTimersOn;

		LoggerThread logger;
		string logDir;
};

