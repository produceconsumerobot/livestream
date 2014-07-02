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

//#define LIVESTREAM_nSENSORS 3	// number of attached pipes

class Range {
public:
	float min;
	float max;
	Range() {
	}
	
	Range(float _min, float _max) {
		min = _min;
		max = _max;
	}
};

class LivestreamData {
public:
	string dataName;
	float data;

	void update() {
		if (buffer.size() > 0) {
			if (buffer.isLastLine()) {
				buffer.resetLineReader();
			}
			data = ofToFloat(buffer.getNextLine());
		} 
	}

	LivestreamData() {
		data = -1;
	}

	LivestreamData(string dataName) {
		string loadFile = dataName + ".csv";
		cout << ofToDataPath("./") << endl;
		cout << ofToDataPath(loadFile) << endl;
		cout << "Loading: " << loadFile << endl;
		buffer = ofBufferFromFile(loadFile);
		data = -1;
	}

private:
	ofBuffer buffer;
};

class LivestreamPipe {
private:
	LivestreamData data;

public:
	string dataName;
	int midiChannel;
	Range dataRange;
	Range midiNoteRange;
	string arduinoPort;
	ofArduino arduino;
	bool isArduinoSetup;

	LivestreamPipe() {
	}

	void setup(string _dataName, string _arduinoPort, int _midiChannel, Range _dataRange, Range _midiNoteRange) {
		dataName = _dataName;
		arduinoPort = _arduinoPort;
		midiChannel = _midiChannel;
		dataRange = _dataRange;
		midiNoteRange = _midiNoteRange;
		data = LivestreamData(dataName);
	}

	float getData() {
		return data.data;
	}

	void updateData() {
		data.update();
	}
};


class LivestreamDemoSettings 
{
public:

	std::vector< LivestreamPipe > pipes;

	//std::vector< std::vector < float > > data;
	std::vector< float > data;

	int midiPort;
	int midiNoteAttack;
	int soundCheck;
	float maxSensorDist;
	float maxOuputDist;
	int crossChannelSustain;

	int dataReadRate;		//milliseconds
	int dataMidiSendRate;	//milliseconds

	bool usingDistanceSensors;

	int nSensors;

	LivestreamDemoSettings() 
	{
		usingDistanceSensors = true;

		nSensors = 2;
		pipes.resize(nSensors); 
		pipes.at(0).setup("Field Conductivity", "COM8", 2, Range(0, 3500), Range(0, 9));
		pipes.at(1).setup("Field pH", "COM9", 1, Range(5, 10), Range(0, 4));
		//pipes.at(1) = LivestreamPipe("Field Temperature", "COM9", 3, Range(0, 30), Range(0, 4));
		
		// **** Setup midi **** //
		// Midi Port
		// midiPort = 0; // Mac
		midiPort = 1; // PC
		midiNoteAttack = 127;
		
		dataReadRate = 2500;		//milliseconds
		dataMidiSendRate = 2500;	//milliseconds

		soundCheck = 0;	// for debugging sound

		// **** Setup sensor conversion variables **** //
		maxSensorDist = 25.0;		// total range of sensor (ft)
		maxOuputDist = 20.0;		// target output range of sensor (ft)

		crossChannelSustain = 4; // >0 alternates between channels to create same note sustain
	}


};
#endif