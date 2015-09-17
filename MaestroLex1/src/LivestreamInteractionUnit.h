
#pragma once

#include <string>
#include "ofMain.h"
#include "ofxNetwork.h"
#include "Range.h"
#include "LivestreamNetwork.h"

using namespace std;

// ---------------------------------------------------------------------------- //
// LivestreamInteractionUnit
// ---------------------------------------------------------------------------- //
class LivestreamInteractionUnit {

public:

	int id;						// ID of this interaction unit
	string dataName;			// Name of the water data played by this unit
	string sensorLocation;		// Location of the water data played by this unit
	string ipAddress;			// IP address of this unit
	float volume;				// Note play volume
	Range volumeRange;			// Min/Max note play volume		
	bool sustainPedal;			// Enables the audio notes to sustain 
	Range waterDataRange;		// Range of water data to transpose into notes
	Range noteRange;			// Range of notes for water data to transpose into
	int waterDataReadInterval;	// Interval (ms) at which new notes are read from the water data "score"
	int notePlayInterval;		// Interval (ms) at which the present note is played
	int distanceReadInterval;	// Interval (ms) that the interaction distance is sampled
	Range distanceRange;		// Min/Max interactive distance (cm) of this unit
	int distanceSignalStrength;	// Signal strength of distance sensor
	Range signalStrengthRange;	// Min/Full signal strength used for weighting the distance measure
	float minSignalWeight;		// Weight assigned to the min minimum signal strength
	int noiseDistance;			// Distance (cm) below which the measurement is considered noise
	int maxDistSamplesToSmooth;	// Number of distance samples to smooth
	bool heartbeat;				// Hearbeats to track if we're alive
	int heartbeatInterval;		// Interval (ms) to pulse heartbeats
	bool eyeSafetyOn;			// Tracks whether eye safety is on
	int distSensorStatus;		// Status of the distance sensor
	unsigned long long nPacketsSent;
	int packetProtocolVersion;

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

private:
	int distSamplesToSmooth;	// 
	int rawDistance;			// Most recent distance measurement
	int smoothedDistance;		// Smoothed distance weighted by the signal strength
	int temperature;			// Most recent temperature measurement;
	int lowTemperature;			// Lowest recorded temperature
	int highTemperature;		// Highest recorded temperature
};

