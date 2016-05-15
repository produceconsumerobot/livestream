
#pragma once

#include <string>
#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxGui.h"
#include "ofxInputField.h"
#include "Range.h"
#include "LivestreamNetwork.h"

using namespace std;

// ---------------------------------------------------------------------------- //
// LivestreamInteractionUnit
// ---------------------------------------------------------------------------- //
class LivestreamInteractionUnit {

public:
	
	int id;						// ID of this interaction unit
	ofxTextField dataName;			// Name of the water data played by this unit
	ofxTextField sensorLocation;		// Location of the water data played by this unit
	ofxTextField ipAddress;			// IP address of this unit
	ofxFloatField volume;				// Note play volume
	ofxFloatField volumeMin;
	ofxFloatField volumeMax;
	ofxFloatField waterDataMin;
	ofxFloatField waterDataMax;
	ofxIntField noteMin;
	ofxIntField noteMax;
	ofxIntField waterDataReadInterval;
	ofxIntField notePlayInterval;
	ofxIntField distanceReadInterval;
	ofxFloatField distanceMin;
	ofxFloatField distanceMax;
	ofxIntField distanceSignalStrength;	// Signal strength of distance sensor
	ofxFloatField signalStrengthMin;	// Min/Full signal strength used for weighting the distance measure
	ofxFloatField signalStrengthMax;	// Min/Full signal strength used for weighting the distance measure
	ofxFloatField minSignalWeight;		// Weight assigned to the min minimum signal strength
	ofxIntField noiseDistance;			// Distance (cm) below which the measurement is considered noise
	ofxIntField maxDistSamplesToSmooth;	// Number of distance samples to smooth
	//bool heartbeat;				// Hearbeats to track if we're alive
	ofxIntField heartbeatInterval;		// Interval (ms) to pulse heartbeats

	ofxFloatSlider guiSmoothedDistance;		// Smoothed distance weighted by the signal strength
	ofxFloatSlider guiTemperature;			// Most recent temperature measurement;
	ofxFloatSlider guiLowTemperature;			// Lowest recorded temperature
	ofxFloatSlider guiHighTemperature;		// Highest recorded temperature

	bool eyeSafetyOn;			// Tracks whether eye safety is on
	int distSensorStatus;		// Status of the distance sensor
	unsigned long long nPacketsSent;
	int packetProtocolVersion;


	//Range volumeRange;			// Min/Max note play volume		
	bool sustainPedal;			// Enables the audio notes to sustain 
	//Range waterDataRange;		// Range of water data to transpose into notes
	//Range noteRange;			// Range of notes for water data to transpose into
	//int waterDataReadInterval;	// Interval (ms) at which new notes are read from the water data "score"
	//int notePlayInterval;		// Interval (ms) at which the present note is played
	//int distanceReadInterval;	// Interval (ms) that the interaction distance is sampled
	//Range distanceRange;		// Min/Max interactive distance (cm) of this unit

	unsigned long long waterDataReadTime;
	unsigned long long notePlayTime;
	unsigned long long distanceReadTime;
	unsigned long long heartbeatTime;
	
	ofxUDPManager udpSender;

	void setDistance(int distance, int signalStrength);
	float calculateVolume();
	void setTemperature(int temp);

	void parseUdpPacket(char * udpMessage); 

	int getSmoothedDistance();
	int getVolume();
	int getRawDistance();
	int getTemperature();
	int getLowTemperature();
	int getHighTemperature();

	LivestreamInteractionUnit();
	void setup(int _id);

	// GUI
	ofxPanel		ixPanel;

private:
	int distSamplesToSmooth;	// 
	int rawDistance;			// Most recent distance measurement
	int smoothedDistance;		// Smoothed distance weighted by the signal strength
	int temperature;			// Most recent temperature measurement;
	int lowTemperature;			// Lowest recorded temperature
	int highTemperature;		// Highest recorded temperature
};

