
#include "LivestreamInteractionUnit.h"
#include "ofxThreadedLogger.h"

// ---------------------------------------------------------------------------- //
// Constructor
// ---------------------------------------------------------------------------- //
LivestreamInteractionUnit::LivestreamInteractionUnit() {
}

// ---------------------------------------------------------------------------- //
// Destructor
// ---------------------------------------------------------------------------- //
LivestreamInteractionUnit::~LivestreamInteractionUnit() {
	closeWaterDataFile();

	//ofRemoveListener(ipAddress, this, &LivestreamInteractionUnit::ipAddressChanged);
}

// ---------------------------------------------------------------------------- //
// setup
// ---------------------------------------------------------------------------- //
//void LivestreamInteractionUnit::setup(int _id, string _ipAddress) {
void LivestreamInteractionUnit::setup(int _id, string _ipAddress, string _dataName, string _sensorLocation, string _waterDataDir) {
	id = _id;
	//ipAddress = _ipAddress;
	ipAddress.addListener(this, &LivestreamInteractionUnit::ipAddressChanged);

	ixPanel.setup("IXUnit" + ofToString(_id),
		"IXUnit" + ofToString(_id) + ".xml", 0, 0);
	//ixPanel.add(id.setup("ID", _id, -1, 255));
	ixPanel.add(dataName.setup("dataName", _dataName));
	ixPanel.add(sensorLocation.setup("sensorLoc", _sensorLocation));
	ixPanel.add(ipAddress.setup("IP", _ipAddress));
	ixPanel.add(lastPong.setup("lastPong", "NA"));
	ixPanel.add(rawDistance.setup("rawDistance", 0, -1, 40 * 30));
	ixPanel.add(guiSmoothedDistance.setup("SmoothedDistance", 0, -1, 40 * 30));
	ixPanel.add(guiTemperature.setup("Temperature", -60, -60, 100));						// Celcius
	ixPanel.add(guiLowTemperature.setup("LowTemperature", -60, -60, 100));					// Celcius
	ixPanel.add(guiHighTemperature.setup("HighTemperature", 0, -60, 100));					// Celcius
	ixPanel.add(waterData.setup("waterData", -10000, -10000, 10000));
	ixPanel.add(note.setup("note", 1, 1, 32));

	ixPanel.add(volume.setup("volume", 0.f, 0.f, 1.f));
	ixPanel.add(distanceMin.setup("distanceMin", 40, 0, 50 * 30));							// cm
	ixPanel.add(distanceMax.setup("distanceMax", 20 * 30, 0, 50 * 30));						// cm
	ixPanel.add(waterDataMin.setup("waterDataMin", 0.f, -10000.f, 10000.f));
	ixPanel.add(waterDataMax.setup("waterDataMax", 0.f, -10000.f, 10000.f));
	ixPanel.add(noteMin.setup("noteMin", 1, 1, 32));
	ixPanel.add(noteMax.setup("noteMax", 13, 1, 32));
	ixPanel.add(volumeMin.setup("volMin", 0.f, 0.f, 1.f));
	ixPanel.add(volumeMax.setup("volMax", 1.f, 0.f, 1.f));
	ixPanel.add(distanceReadInterval.setup("distanceReadInterval", 1000 / 10, 1, 2000));	// ms
	ixPanel.add(waterDataReadInterval.setup("waterDataReadInterval", 1000, 1, 60000));		// ms
	ixPanel.add(notePlayInterval.setup("notePlayInterval", 1000, 1, 5000));					// ms
	ixPanel.add(heartbeatInterval.setup("heartbeatInterval", 1000, 0, 2000));				// ms
	ixPanel.add(guiSignalStrength.setup("signalStrength", 0, 0, 1.f));
	ixPanel.add(signalStrengthMin.setup("signalStrengthMin", 20.f/255.f, 0, 1.f));
	ixPanel.add(signalStrengthMax.setup("signalStrengthMax", 80.f/255.f, 0, 1.f));
	ixPanel.add(minSignalWeight.setup("minSignalWeight", 0.05f, 0.f, 1.f));
	ixPanel.add(noiseDistance.setup("noiseDistance", 20, 0, 100));
	ixPanel.add(maxDistSamplesToSmooth.setup("maxDistSamplesToSmooth", 1, 0, 100));
	
	heartbeatTime = ofGetElapsedTimeMillis();
	eyeSafetyOn = false;
	distSensorStatus = 0;
	nPacketsSent = 0;
	packetProtocolVersion = 1;

	sustainPedal = true;
	waterDataReadTime = ofGetElapsedTimeMillis();
	notePlayTime = ofGetElapsedTimeMillis();
	distanceReadTime = ofGetElapsedTimeMillis();
	rawDistance = 0;					// cm
	smoothedDistance = 0;				// cm
	distSamplesToSmooth = 0;			// Start at zero and increment to maxDistSamplesToSmooth
	temperature = -100;
	lowTemperature = -100;
	highTemperature = -100;
	heartbeatInterval = 1000;			// ms
	heartbeatTime = ofGetElapsedTimeMillis();
	eyeSafetyOn = false;
	distSensorStatus = 0;
	nPacketsSent = 0;
	packetProtocolVersion = 1;
	waterDataDir = _waterDataDir;
	openWaterDataFile();

	ledState = false;
	loggingOn = false;

	// Setup the UDP 
	udpSender.Create();
	udpSender.SetEnableBroadcast(false);
	udpSender.Connect(ipAddress.getParameter().toString().c_str(), 11999);
	udpSender.SetNonBlocking(true);

	if (loggingOn) {
		// Log to file
		ostringstream  logStringStream;
		logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << "IXUnit," << id << ","
			<< dataName.getParameter().toString() << "," 
			<< sensorLocation.getParameter().toString() << ',' 
			<< ipAddress.getParameter().toString() << endl;
		logger->log(logStringStream.str());
	}
}

// ---------------------------------------------------------------------------- //
// setDistance
// ---------------------------------------------------------------------------- //
void LivestreamInteractionUnit::setDistance(int distance, float signalStrength) {
	// set the raw distance
	rawDistance = distance;
	// set the signal strength
	guiSignalStrength = signalStrength;

	// increment up to max smoothing (deals with initial state)
	if (distSamplesToSmooth < maxDistSamplesToSmooth) distSamplesToSmooth++;
	// Calculate the weight of new value
	float newWeight = ((float)1) / ((float)distSamplesToSmooth);

	if (rawDistance < noiseDistance) {
		// Handle strange case where lidar reports 1cm when should be infinity
		rawDistance = distanceMax * 2;

		// We're within the noise distance
		// Ignore signal strength and go toward newVal quickly
		newWeight = newWeight * (1.f - signalStrengthMin);
	}
	else {
		// Weight by the signal strength
		newWeight = newWeight * ofMap(signalStrength, signalStrengthMin, signalStrengthMax, minSignalWeight, 1.f, true);
	}

	// Calculate the smoothed PWM value
	smoothedDistance = ((float)rawDistance)*newWeight + smoothedDistance*(1 - newWeight);
	guiSmoothedDistance = smoothedDistance;

	if (loggingOn) {
		// Log to file
		ostringstream  logStringStream;
		logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << "LD," << id << ","
			<< distance << "," << (int) (signalStrength*255) << ',' << (int) smoothedDistance << endl;
		logger->log(logStringStream.str());
	}
}
// ---------------------------------------------------------------------------- //
// calculateVolume
// ---------------------------------------------------------------------------- //
float LivestreamInteractionUnit::calculateVolume() {
	volume = 1.f - ofMap(smoothedDistance, distanceMin, distanceMax, volumeMin, volumeMax, true);
	return volume;
}


// ---------------------------------------------------------------------------- //
// getSmoothedDistance
// ---------------------------------------------------------------------------- //
int LivestreamInteractionUnit::getSmoothedDistance() {
	return smoothedDistance;
}

// ---------------------------------------------------------------------------- //
// getVolume
// ---------------------------------------------------------------------------- //
int LivestreamInteractionUnit::getVolume() {
	return volume;
}

// ---------------------------------------------------------------------------- //
// setTemperature
// ---------------------------------------------------------------------------- //
void LivestreamInteractionUnit::setTemperature(int temp) {
	temperature = temp;
	guiTemperature = temperature;

	if (lowTemperature == -100)
	{
		lowTemperature = temperature;
	}
	if (highTemperature == -100)
	{
		highTemperature = temperature;
	}

	lowTemperature = min(temperature, lowTemperature);
	guiLowTemperature = lowTemperature;

	highTemperature = max(temperature, highTemperature);
	guiHighTemperature = highTemperature;
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

// ---------------------------------------------------------------------------- //
// parseUdpPacket
// ---------------------------------------------------------------------------- //
void LivestreamInteractionUnit::parseUdpPacket(char * udpMessage) {
		
		// Convert the UDP message to read its type tag from the header section
		LivestreamNetwork::PacketHeader_V1* header = 
			(LivestreamNetwork::PacketHeader_V1 *) udpMessage;
		// Convert the typeTage char[2] to a string for logging
		string typeTag(header->typeTag, header->typeTag + sizeof(header->typeTag) / sizeof(header->typeTag[0]));
		//ofLog(OF_LOG_VERBOSE) << ipAddress.getParameter().toString() << " >> " << typeTag << endl;

		// Parse DISTANCE Packets
		if (memcmp( header->typeTag, LivestreamNetwork::DISTANCE, 
			sizeof header->typeTag / sizeof header->typeTag[0]) == 0) 
		{
			LivestreamNetwork::Packet_DISTANCE_V1* inPacket = (LivestreamNetwork::Packet_DISTANCE_V1 *) udpMessage;
			ofLog(OF_LOG_VERBOSE) << ipAddress.getParameter().toString() << " >> " << typeTag 
				<< " LD, " << inPacket->distance << ", LS, " << inPacket->signalStrength << endl;
			
			// Parse DISTANCE data
			setDistance(inPacket->distance, (float) inPacket->signalStrength / 255.f);
			// Calculate the new volume level
			calculateVolume();

			// Send volume update
			// Load the packet data
			LivestreamNetwork::Packet_SET_VOLUME_V1 outPacket;
			outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
			outPacket.hdr.packetCount = ++nPacketsSent;
			outPacket.hdr.protocolVersion = packetProtocolVersion;
			strncpy(outPacket.hdr.typeTag, LivestreamNetwork::SET_VOLUME, 
				sizeof LivestreamNetwork::SET_VOLUME / sizeof LivestreamNetwork::SET_VOLUME[0]);
			outPacket.volume = (uint8_t) (volume * 255.f);
							
			// Send the packet
			udpSender.Send((char*) &outPacket, sizeof(LivestreamNetwork::Packet_SET_VOLUME_V1));
			// Convert the typeTage char[2] to a string for logging
			typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / 
				sizeof(outPacket.hdr.typeTag[0]));
			ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << ipAddress.getParameter().toString() << endl;	

			if (loggingOn) {
				// Log to file
				ostringstream  logStringStream;
				logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << "SV," << id << "," << outPacket.volume << endl;
				logger->log(logStringStream.str());
			}

		} else if (memcmp( header->typeTag, LivestreamNetwork::TEMPERATURE, 
			sizeof header->typeTag / sizeof header->typeTag[0]) == 0) 
		{
			LivestreamNetwork::Packet_TEMPERATURE_V1* inPacket = (LivestreamNetwork::Packet_TEMPERATURE_V1 *) udpMessage;
			ofLog(OF_LOG_VERBOSE) << ipAddress.getParameter().toString() << " >> " << typeTag 
				<< inPacket->sensorDesignator << ", " << inPacket->temperature << endl;
			if (inPacket->sensorDesignator == 'C') {
				// raspi cpu temperature
				setTemperature(inPacket->temperature);

				if (loggingOn) {
					// Log to file
					ostringstream  logStringStream;
					logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << "TN," << id << ",C," << temperature << endl;
					logger->log(logStringStream.str());
				}
			}
		} else if (memcmp(header->typeTag, LivestreamNetwork::PONG,
			sizeof header->typeTag / sizeof header->typeTag[0]) == 0) 
		{
			lastPong = ofGetTimestampString("%Y-%m-%d %H:%M:%S");
			ofLog(OF_LOG_VERBOSE) << ipAddress.getParameter().toString() << " >> " << typeTag << endl;
			if (loggingOn) {
				// Log to file
				ostringstream  logStringStream;
				logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << "PO," << id << endl;
				logger->log(logStringStream.str());
			}
		}
}

// ---------------------------------------------------------------------------- //
void LivestreamInteractionUnit::playNote(string dirPath) {
	note = calculateNote();

	LivestreamNetwork::Packet_PLAY_NOTE_V1 outPacket;
	outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
	outPacket.hdr.packetCount = ++nPacketsSent;
	outPacket.hdr.protocolVersion = packetProtocolVersion;
	//string filePath = "audio/Coldspring_Conductivity_01.mp3";
	//string dirPath = dirPath;

	string aligner = "";
	if (note < 10) aligner = aligner + "0";
	string filename = dataName.getParameter().toString() + "_" + sensorLocation.getParameter().toString() 
		+ "_" + aligner + note.getParameter().toString() + ".mp3";
	string filePath = dirPath + filename;
	strncpy(outPacket.filePath, filePath.c_str(), sizeof(outPacket.filePath));
	strncpy(outPacket.hdr.typeTag, LivestreamNetwork::PLAY_NOTE,
		sizeof(LivestreamNetwork::PLAY_NOTE) / sizeof(LivestreamNetwork::PLAY_NOTE[0]));

	// Send the packet
	udpSender.Send((char*)&outPacket, sizeof(outPacket));
	// Convert the typeTage char[2] to a string for logging
	string typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
	ofLog(OF_LOG_VERBOSE) << typeTag << " (" << filePath << ")" << " >> " << ipAddress.getParameter().toString() << endl;

	if (loggingOn) {
		// Log to file
		ostringstream  logStringStream;
		logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << "PN," << id << "," << filePath << endl;
		logger->log(logStringStream.str());
	}
}

void LivestreamInteractionUnit::ping() {
	LivestreamNetwork::Packet_PING_V1 outPacket;
	outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
	outPacket.hdr.packetCount = ++nPacketsSent;
	outPacket.hdr.protocolVersion = packetProtocolVersion;
	strncpy(outPacket.hdr.typeTag, LivestreamNetwork::PING,
		sizeof(LivestreamNetwork::PING) / sizeof(LivestreamNetwork::PING[0]));

	// Send the packet
	udpSender.Send((char*)&outPacket, sizeof(outPacket));
	// Convert the typeTage char[2] to a string for logging
	string typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
	ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << ipAddress.getParameter().toString() << endl;

	if (loggingOn) {
		// Log to file
		ostringstream  logStringStream;
		logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << "PI," << id << endl;
		logger->log(logStringStream.str());
	}
}


void LivestreamInteractionUnit::getDistance() {
	// ********** GET_DISTANCE packet type ********** //
	// Load the packet data
	LivestreamNetwork::Packet_GET_DISTANCE_V1 outPacket;
	outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
	outPacket.hdr.packetCount = ++nPacketsSent;
	outPacket.hdr.protocolVersion = packetProtocolVersion;
	strncpy(outPacket.hdr.typeTag, LivestreamNetwork::GET_DISTANCE,
		sizeof(LivestreamNetwork::GET_DISTANCE) / sizeof(LivestreamNetwork::GET_DISTANCE[0]));

	// Send the packet
	udpSender.Send((char*)&outPacket, sizeof(outPacket));
	// Convert the typeTage char[2] to a string for logging
	string typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
	ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << ipAddress.getParameter().toString() << endl;

	if (loggingOn) {
		// Log to file
		ostringstream  logStringStream;
		logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << LivestreamNetwork::GET_DISTANCE << "," << id << endl;
		//logger->log("happy\n");
		logger->log(logStringStream.str());
	}
	
	//ofLog(OF_LOG_VERBOSE)
	//<< "nPacketsSent, " << outPacket.hdr.packetCount
	//<< ", timeStamp, " << outPacket.hdr.timeStamp
	//<< ", ver, " << outPacket.hdr.protocolVersion
	//	<< endl;
}

void LivestreamInteractionUnit::setLed() {
	// ********** SET_LED packet type ********** //
	// Load the packet data
	LivestreamNetwork::Packet_SET_LED_V1 outPacket;
	outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
	outPacket.hdr.packetCount = ++nPacketsSent;
	outPacket.hdr.protocolVersion = packetProtocolVersion;
	strncpy(outPacket.hdr.typeTag, LivestreamNetwork::SET_LED,
		sizeof(LivestreamNetwork::SET_LED) / sizeof(LivestreamNetwork::SET_LED[0]));

	ledState = !ledState;
	outPacket.state = ledState;

	// Send the packet
	udpSender.Send((char*)&outPacket, sizeof(outPacket));
	// Convert the typeTage char[2] to a string for logging
	string typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
	ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << ipAddress.getParameter().toString() << endl;

	if (loggingOn) {
		// Log to file
		ostringstream  logStringStream;
		logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << LivestreamNetwork::SET_LED << "," << id << "," << ledState << endl;
		logger->log(logStringStream.str());
	}
}

void LivestreamInteractionUnit::getAllTemps() {
	// ********** GET_ALL_TEMPS packet type ********** //
	// Load the packet data
	LivestreamNetwork::Packet_GET_ALL_TEMPS_V1 outPacket;
	outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
	outPacket.hdr.packetCount = ++nPacketsSent;
	outPacket.hdr.protocolVersion = packetProtocolVersion;
	strncpy(outPacket.hdr.typeTag, LivestreamNetwork::GET_ALL_TEMPS,
		sizeof(LivestreamNetwork::GET_ALL_TEMPS) / sizeof(LivestreamNetwork::GET_ALL_TEMPS[0]));

	// Send the packet
	udpSender.Send((char*)&outPacket, sizeof(outPacket));
	// Convert the typeTage char[2] to a string for logging
	string typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
	ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << ipAddress.getParameter().toString() << endl;

	if (loggingOn) {
		// Log to file
		ostringstream  logStringStream;
		logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << LivestreamNetwork::GET_ALL_TEMPS << "," << id << endl;
		logger->log(logStringStream.str());
	}
}

void LivestreamInteractionUnit::setMaestroAddress(string maestroIpAddress) {
	// ********** SET_MAESTRO_ADDRESS packet type ********** //
	// Load the packet data
	LivestreamNetwork::Packet_SET_MAESTRO_ADDRESS_V1 outPacket;
	outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
	outPacket.hdr.packetCount = ++nPacketsSent;
	outPacket.hdr.protocolVersion = packetProtocolVersion;
	strncpy(outPacket.hdr.typeTag, LivestreamNetwork::SET_MAESTRO_ADDRESS,
		sizeof(LivestreamNetwork::SET_MAESTRO_ADDRESS) / sizeof(LivestreamNetwork::SET_MAESTRO_ADDRESS[0]));
	strcpy(outPacket.ipAddress, maestroIpAddress.c_str());

	// Send the packet
	udpSender.Send((char*)&outPacket, sizeof(outPacket));
	// Convert the typeTage char[2] to a string for logging
	string typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
	ofLog(OF_LOG_VERBOSE) << typeTag << " (" << maestroIpAddress << ") >> " << ipAddress.getParameter().toString() << endl;

	if (loggingOn) {
		// Log to file
		ostringstream  logStringStream;
		logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << LivestreamNetwork::SET_MAESTRO_ADDRESS << "," << id << "," << maestroIpAddress << endl;
		logger->log(logStringStream.str());
	}
}

//--------------------------------------------------------------
void LivestreamInteractionUnit::ipAddressChanged(string &_ipAddress) {
	ipAddress = _ipAddress;

	udpSender.Close();
	udpSender.Create();
	udpSender.SetEnableBroadcast(false);
	udpSender.Connect(ipAddress.getParameter().toString().c_str(), 11999);
	udpSender.SetNonBlocking(true);
}

int LivestreamInteractionUnit::readWaterData() {
	if (waterDataFile != NULL && waterDataFile->is_open()) {
		if (waterDataFile->eof()) {
			waterDataFile->clear();
			waterDataFile->seekg(0, ios::beg);
		}
		float f;
		(*waterDataFile) >> f;
		waterData = f;
		//cout << dataName.getParameter().toString() + "_" + sensorLocation.getParameter().toString() + ".csv" << " : " << f;
		//string f;
		//getline(*waterDataFile, f);
		//waterDataFile->getline(f, 256);
		//ofFilePath ofp;
		//cout << ofp.getCurrentExeDir() << endl;
		//cout << ofp.getCurrentExePath() << endl;
		//cout << ofToDataPath("", true) << endl;

		if (loggingOn) {
			// Log to file
			ostringstream  logStringStream;
			logStringStream << ofGetTimestampString("%Y%m%d,%H%M%S,%i,") << "WD," << id << "," << waterData << endl;
			logger->log(logStringStream.str());
		}


		return 0;
	}
	else {
		return -1;
	}
}

int LivestreamInteractionUnit::calculateNote() {
	return ofMap(waterData, waterDataMin, waterDataMax, noteMin, noteMax, true);
}

int LivestreamInteractionUnit::openWaterDataFile() {
	string filename = dataName.getParameter().toString() + "_" + sensorLocation.getParameter().toString() + ".csv";
	waterDataFile = new fstream(waterDataDir + filename);
	string filePath = waterDataDir + filename;
	waterDataFile->open(filePath.c_str(), std::ios::in);
	if (waterDataFile->is_open()) {
		waterDataFile->clear();
		waterDataFile->seekg(0, ios::beg);
		return 0;
	}
	else {
		free(waterDataFile);
		waterDataFile = NULL;
		return -1;
	}
}

int LivestreamInteractionUnit::closeWaterDataFile() {
	if (waterDataFile != NULL && waterDataFile->is_open()) {
		waterDataFile->close();
		free(waterDataFile);
		waterDataFile = NULL;
		return 0;
	}
	else {
		return -1;
	}
}
