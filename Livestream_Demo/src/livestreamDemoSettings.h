//
//  settings.h
//
//  Created by Sean Montgomery on 06/22/14.
//  http://produceconsumerobot.com/
//
//  This work is licensed under the Creative Commons 
//  Attribution-ShareAlike 3.0 Unported License. 
//  To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/.
//


#ifndef _LIVESTREAM_DEMO_SETTINGS
#define _LIVESTREAM_DEMO_SETTINGS

#include <string>
#include <vector>

#define LIVESTREAM_nSENSORS 4	// number of attached sensors


class LivestreamDemoSettings 
{
public:

	std::vector< string > arduinoPorts;

	int midiPort;
	int soundCheck;
	float maxSensorDist;
	float maxOuputDist;

	LivestreamDemoSettings() 
	{
		// **** Setup arduino **** //
		// Arduino Ports for Sean's Windows computer
		string arduinoPorts_arr[LIVESTREAM_nSENSORS] = 
		{
			"COM8",		// 1
			"COM11",	// 2
			"COM9",		// 3
			"COM10",	// 4
		};
		// Copy array to a vector for safety
		arduinoPorts.resize(LIVESTREAM_nSENSORS);
		for (int i=0; i<LIVESTREAM_nSENSORS; i++) { 
			arduinoPorts.at(i) = arduinoPorts_arr[i]; // Copy array to a vector for safety
		}

		// **** Setup midi **** //
		// Midi Port
		// midiPort = 0; // Mac
		midiPort = 1; // PC

		soundCheck = 0;	// for debugging sound

		// **** Setup sensor conversion variables **** //
		maxSensorDist = 25.0;		// total range of sensor (ft)
		maxOuputDist = 10.0;		// target output range of sensor (ft)
	}
};
#endif