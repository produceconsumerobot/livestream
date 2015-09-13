
#pragma once

#include <string>
#include "Range.h"
#include "LivestreamInteractionUnit.h"

using namespace std;

// ---------------------------------------------------------------------------- //
// LivestreamSettings
// ---------------------------------------------------------------------------- //
class LivestreamSettings {
public:
	string waterDataAddress;
	string soundFilesLocation;		// Path to the sound files directory
	Range volumeRange;				// Min/Max note play volume	
	bool sustainPedal;				// Enables the audio notes to sustain 
	int waterDataUpdateInterval;	// Interval (minutes) at which water data is downloaded from the interwebs
	int waterDataReadInterval;		// Interval (ms) at which new notes are read from the water data "score"
	int notePlayInterval;			// Interval (ms) at which the present note is played
	Range signalStrengthRange;		// Min/Full signal strength used for weighting the distance measure
	float minSignalWeight;			// Weight assigned to the min minimum signal strength 
	int noiseDistance;				// Distance (cm) below which the measurement is considered noise
	int maxDistSamplesToSmooth;		// Number of distance samples to smooth

	LivestreamSettings();			// Constructor
	void updateInteractionUnit(LivestreamInteractionUnit liu);
private:

};

// ---------------------------------------------------------------------------- //
// LivestreamSettings Constructor
// ---------------------------------------------------------------------------- //
LivestreamSettings::LivestreamSettings() {
	waterDataAddress = "http://google.com/";
	soundFilesLocation = "./";
	volumeRange = Range(0.f, 1.f);		// 0 to 1
	sustainPedal = true;
	waterDataReadInterval = 3000;		// ms
	notePlayInterval = 1000;			// ms
	signalStrengthRange = Range(20, 80);	// 0 to 255
	minSignalWeight = 0.05f;			// 0 to 1
	noiseDistance = 20;					// cm
	maxDistSamplesToSmooth = 1;		
}

// ---------------------------------------------------------------------------- //
// updateInteractionUnit
// ---------------------------------------------------------------------------- //
void LivestreamSettings::updateInteractionUnit(LivestreamInteractionUnit liu) {
	liu.volumeRange = volumeRange;
	liu.sustainPedal = sustainPedal;
	liu.waterDataReadInterval = waterDataReadInterval;		
	liu.notePlayInterval = notePlayInterval;			
	liu.signalStrengthRange = signalStrengthRange;	
	liu.minSignalWeight = minSignalWeight;			
	liu.noiseDistance = noiseDistance;					
	liu.maxDistSamplesToSmooth = maxDistSamplesToSmooth;		
}