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

		ofxUDPManager udpSender;
		ofxUDPManager udpReceiver;
		unsigned long long nPacketsSent;
		int packetProtocolVersion;

		string maestroIpAddress;

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
		ofxPanel globalSettingsPanel;

		ofxTextField waterDataFilesLocation;
		ofxTextField soundFilesLocation;
		ofxFloatField volumeMin;
		ofxFloatField volumeMax;
		ofxIntField waterDataReadInterval;
		ofxIntField distanceReadInterval;
		ofxIntField notePlayInterval;
		ofxIntField signalStrengthMin;
		ofxIntField signalStrengthMax;
		ofxFloatField minSignalWeight;
		ofxIntField noiseDistance;
		ofxIntField maxDistSamplesToSmooth;

		ofxPanel maestroPanel;
		ofxLabel datetimeString;
		ofxLabel lastStartupTime;
		ofxLabel currentTemp;
		ofxLabel lowTemp;
		ofxLabel highTemp;

		int panelWidth;
		int panelRowHeight;

};

