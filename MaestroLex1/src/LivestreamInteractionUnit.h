
#pragma once

#include <string>
#include "ofMain.h"
#include "Range.h"

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

	void setDistance(int distance, int signalStrength);
	void setTemperature(int temp);

	int parseUdpPacket(char * packet); 

	int getSmoothedDistance();
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

// ---------------------------------------------------------------------------- //
// Constructor
// ---------------------------------------------------------------------------- //
LivestreamInteractionUnit::LivestreamInteractionUnit() {
	
	id = -1;
	dataName = "none";
	sensorLocation = "none";
	ipAddress = "0.0.0.0";
	volume = 0.f;						// 0 to 1
	volumeRange = Range(0.f, 1.f);		// 0 to 1
	sustainPedal = true;
	waterDataRange = Range(0, 1);		// Range of data
	noteRange = Range(1, 2);			// number of note files
	waterDataReadInterval = 3000;		// ms
	notePlayInterval = 1000;			// ms
	distanceReadInterval = 1000/60;		// ms
	rawDistance = 0;					// cm
	smoothedDistance = 0;				// cm
	distanceRange = Range(30, 20*30);	// cm
	distanceSignalStrength = 0;			// 0 to 255
	signalStrengthRange = Range(20, 80);	// 0 to 255
	minSignalWeight = 0.05f;			// 0 to 1
	noiseDistance = 20;					// cm
	distSamplesToSmooth = 0;			// Start at zero and increment to maxDistSamplesToSmooth
	maxDistSamplesToSmooth = 1;		
	temperature = -1;
	lowTemperature = -1;
	highTemperature = -1;
	heartbeat = 1;
	heartbeatInterval = 1000;			// ms
	eyeSafetyOn = false;
	distSensorStatus = 0;	


};

// ---------------------------------------------------------------------------- //
// 
// ---------------------------------------------------------------------------- //
void LivestreamInteractionUnit::setDistance(int distance, int signalStrength) {
	// set the raw distance
	rawDistance = distance;
	// set the signal strength
	distanceSignalStrength = signalStrength;

	// increment up to max smoothing (deals with initial state)
	if (distSamplesToSmooth < maxDistSamplesToSmooth ) distSamplesToSmooth++; 
	// Calculate the weight of new value
	float newWeight = ((float)1)/((float) distSamplesToSmooth); 

	if (rawDistance < noiseDistance) {
		// Handle strange case where lidar reports 1cm when should be infinity
		distance = distanceRange.max*2;

		// We're within the noise distance
		// Ignore signal strength and go toward newVal quickly
		newWeight = newWeight * (1.f - signalStrengthRange.min);
	} else {
		// Weight by the signal strength
		newWeight = newWeight * ofMap(signalStrength, signalStrengthRange.min, signalStrengthRange.max, minSignalWeight, 1.f, true);
	}

	// Calculate the smoothed PWM value
	smoothedDistance = ((float) rawDistance)*newWeight + smoothedDistance*(1-newWeight);
}

// ---------------------------------------------------------------------------- //
// setTemperature
// ---------------------------------------------------------------------------- //
void LivestreamInteractionUnit::setTemperature(int temp) {
	temperature = temp;
	lowTemperature = min(temperature, lowTemperature);
	highTemperature = max(temperature, highTemperature);
}

// ---------------------------------------------------------------------------- //
// getTemperature
// ---------------------------------------------------------------------------- //
int LivestreamInteractionUnit::getTemperature() {
	return temperature;
}

// ---------------------------------------------------------------------------- //
// getLowTemperature
// ---------------------------------------------------------------------------- //
int LivestreamInteractionUnit::getLowTemperature() {
	return lowTemperature;
}

// ---------------------------------------------------------------------------- //
// getHighTemperature
// ---------------------------------------------------------------------------- //
int LivestreamInteractionUnit::getHighTemperature() {
	return highTemperature;
}