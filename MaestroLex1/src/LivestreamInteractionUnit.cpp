
#include "LivestreamInteractionUnit.h"
#include "logger.h"

// ---------------------------------------------------------------------------- //
// Constructor
// ---------------------------------------------------------------------------- //
LivestreamInteractionUnit::LivestreamInteractionUnit() {
}

// ---------------------------------------------------------------------------- //
// Destructor
// ---------------------------------------------------------------------------- //
LivestreamInteractionUnit::~LivestreamInteractionUnit() {

}

// ---------------------------------------------------------------------------- //
// setup
// ---------------------------------------------------------------------------- //
//void LivestreamInteractionUnit::setup(int _id, string _ipAddress) {
void LivestreamInteractionUnit::setup(int _id, string _ipAddress, string _dataName, string _sensorLocation) {
	id = _id;
	//ipAddress = _ipAddress;
	ixPanel.setup("IXUnit" + to_string(_id),
		"IXUnit" + to_string(_id) + ".xml", 0, 0);
	ixPanel.setup("IXUnit", "IXUnit.xml", 0, 0);
	//ixPanel.add(id.setup("ID", _id, -1, 255));
	ixPanel.add(dataName.setup("dataName", _dataName));
	ixPanel.add(sensorLocation.setup("sensorLoc", _sensorLocation));
	ixPanel.add(ipAddress.setup("IP", _ipAddress));
	ixPanel.add(lastPong.setup("lastPong", "NA"));
	ixPanel.add(rawDistance.setup("rawDistance", 0, -1, 50 * 30));
	ixPanel.add(guiSmoothedDistance.setup("SmoothedDistance", 0, -1, 50 * 30));
	ixPanel.add(guiTemperature.setup("Temperature", -60, -60, 100));						// Celcius
	ixPanel.add(guiLowTemperature.setup("LowTemperature", -60, -60, 100));					// Celcius
	ixPanel.add(guiHighTemperature.setup("HighTemperature", 0, -60, 100));					// Celcius

	ixPanel.add(volume.setup("volume", 0, 0, 1));
	ixPanel.add(volumeMin.setup("volMin", 0.f, 0, 1));
	ixPanel.add(volumeMax.setup("volMax", 1.f, 0, 1));
	ixPanel.add(waterDataMin.setup("waterDataMin", 0.f, -10000, 10000));
	ixPanel.add(waterDataMax.setup("waterDataMax", 0.f, -10000, 10000));
	ixPanel.add(noteMin.setup("noteMin", 1, 1, 32));
	ixPanel.add(noteMax.setup("noteMax", 2, 1, 32));
	ixPanel.add(heartbeatInterval.setup("heartbeatInterval", 1000, 0, 2000));				// ms
	ixPanel.add(waterDataReadInterval.setup("waterDataReadInterval", 3000, 1, 60000));		// ms
	ixPanel.add(notePlayInterval.setup("notePlayInterval", 1000, 1, 5000));					// ms
	ixPanel.add(distanceReadInterval.setup("distanceReadInterval", 1000 / 60, 1, 2000));	// ms
	ixPanel.add(distanceMin.setup("distanceMin", 30, 0, 50 * 30));							// cm
	ixPanel.add(distanceMax.setup("distanceMax", 2 * 30, 0, 50 * 30));						// cm
	ixPanel.add(guiSignalStrength.setup("signalStrength", 0, 0, 255));
	ixPanel.add(signalStrengthMin.setup("signalStrengthMin", 20, 0, 255));
	ixPanel.add(signalStrengthMax.setup("signalStrengthMax", 80, 0, 255));
	ixPanel.add(minSignalWeight.setup("minSignalWeight", 0.05f, 0.f, 1.f));
	ixPanel.add(noiseDistance.setup("noiseDistance", 20, 0, 100));
	ixPanel.add(maxDistSamplesToSmooth.setup("maxDistSamplesToSmooth", 1, 0, 100));
	
	heartbeatTime = ofGetElapsedTimeMillis();
	eyeSafetyOn = false;
	distSensorStatus = 0;
	nPacketsSent = 0;
	packetProtocolVersion = 1;

	//id = -1;
	//dataName = "none";
	//sensorLocation = "none";
	//ipAddress = "0.0.0.0";
	//volume = 0.f;						// 0 to 1
	//volumeMin = 0.f;
	//volumeMax = 1.f;
	sustainPedal = true;
	//waterDataMin = 0;
	//waterDataMax = 1;
	//noteMin = 1;
	//noteMax = 2;
	//waterDataReadInterval = 3000;		// ms
	waterDataReadTime = ofGetElapsedTimeMillis();
	//notePlayInterval = 1000;			// ms
	notePlayTime = ofGetElapsedTimeMillis();
	//distanceReadInterval = 1000/60;		// ms
	distanceReadTime = ofGetElapsedTimeMillis();
	rawDistance = 0;					// cm
	smoothedDistance = 0;				// cm
										//distanceMin = 30;
										//distanceMax = 20 * 30;
										//distanceRange = Range(30, 20*30);	// cm
										//distanceSignalStrength = 0;			// 0 to 255
										//signalStrengthMin = 20;
										//signalStrengthMax = 80;
										//signalStrengthRange = Range(20, 80);	// 0 to 255
										//minSignalWeight = 0.05f;			// 0 to 1
										//noiseDistance = 20;					// cm
	distSamplesToSmooth = 0;			// Start at zero and increment to maxDistSamplesToSmooth
										//maxDistSamplesToSmooth = 1;		
	temperature = -1;
	lowTemperature = -1;
	highTemperature = -1;
	//heartbeat = 1;
	heartbeatInterval = 1000;			// ms
	heartbeatTime = ofGetElapsedTimeMillis();
	eyeSafetyOn = false;
	distSensorStatus = 0;
	nPacketsSent = 0;
	packetProtocolVersion = 1;
	waterDataFilesLocation = "/livestream/data/";

	udpSender.Create();
	udpSender.SetEnableBroadcast(false);
	udpSender.Connect(ipAddress.getParameter().toString().c_str(), 11999);
	udpSender.SetNonBlocking(true);
}

// ---------------------------------------------------------------------------- //
// setDistance
// ---------------------------------------------------------------------------- //
void LivestreamInteractionUnit::setDistance(int distance, int signalStrength) {
	// set the raw distance
	rawDistance = distance;
	// set the signal strength
	guiSignalStrength = signalStrength;

	if (distance != -1 && signalStrength != -1)
	{

		// increment up to max smoothing (deals with initial state)
		if (distSamplesToSmooth < maxDistSamplesToSmooth) distSamplesToSmooth++;
		// Calculate the weight of new value
		float newWeight = ((float)1) / ((float)distSamplesToSmooth);

		if (rawDistance < noiseDistance) {
			// Handle strange case where lidar reports 1cm when should be infinity
			distance = distanceMax * 2;

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

	if (lowTemperature == -1)
	{
		lowTemperature = temperature;
	}
	if (highTemperature == -1)
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
		ofLog(OF_LOG_VERBOSE) << ipAddress.getParameter().toString() << " >> " << typeTag << endl;

		// Parse DISTANCE Packets
		if (memcmp( header->typeTag, LivestreamNetwork::DISTANCE, 
			sizeof header->typeTag / sizeof header->typeTag[0]) == 0) 
		{
			LivestreamNetwork::Packet_DISTANCE_V1* inPacket = (LivestreamNetwork::Packet_DISTANCE_V1 *) udpMessage;
			ofLog(OF_LOG_VERBOSE) << "LD, " << inPacket->distance << ", LS, " << inPacket->signalStrength << endl;
			
			// Parse DISTANCE data
			setDistance(inPacket->distance, inPacket->signalStrength);
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

		} else if (memcmp( header->typeTag, LivestreamNetwork::TEMPERATURE, 
			sizeof header->typeTag / sizeof header->typeTag[0]) == 0) 
		{
			LivestreamNetwork::Packet_TEMPERATURE_V1* inPacket = (LivestreamNetwork::Packet_TEMPERATURE_V1 *) udpMessage;
			ofLog(OF_LOG_VERBOSE) << "T" << inPacket->sensorDesignator << ", " << inPacket->temperature << endl;
			setTemperature(inPacket->temperature);
		} else if (memcmp(header->typeTag, LivestreamNetwork::PONG,
			sizeof header->typeTag / sizeof header->typeTag[0]) == 0) 
		{
			lastPong = LoggerThread::dateTimeString();
		}
}

// ---------------------------------------------------------------------------- //
void LivestreamInteractionUnit::playNote() {
	LivestreamNetwork::Packet_PLAY_NOTE_V1 outPacket;
	outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
	outPacket.hdr.packetCount = ++nPacketsSent;
	outPacket.hdr.protocolVersion = packetProtocolVersion;
	string filePath = "audio/Coldspring_Conductivity_01.mp3";
	//string filename = dataName.getParameter().toString() + "_" + sensorLocation.getParameter().toString() + "_" + note.getParameter().toString();
	//string filePath = waterDataFilesLocation + filename;
	strncpy(outPacket.filePath, filePath.c_str(), sizeof(outPacket.filePath));
	strncpy(outPacket.hdr.typeTag, LivestreamNetwork::PLAY_NOTE,
		sizeof(LivestreamNetwork::PLAY_NOTE) / sizeof(LivestreamNetwork::PLAY_NOTE[0]));

	// Send the packet
	udpSender.Send((char*)&outPacket, sizeof(outPacket));
	// Convert the typeTage char[2] to a string for logging
	string typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
	ofLog(OF_LOG_VERBOSE) << typeTag << " (" << filePath << ")" << " >> " << ipAddress.getParameter().toString() << endl;
}
