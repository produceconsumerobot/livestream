
#include "LivestreamInteractionUnit.h"

// ---------------------------------------------------------------------------- //
// Constructor
// ---------------------------------------------------------------------------- //
LivestreamInteractionUnit::LivestreamInteractionUnit() {


}

void LivestreamInteractionUnit::setup(int _id) {
	//ixPanel.setup("IXUnit" + to_string(_id),
	//	"IXUnit" + to_string(_id) + ".xml", 0, 0);
	ixPanel.setup("IXUnit", "IXUnit.xml", 0, 0);
	//id = _id;
	//ixPanel.add(id.setup(_id, -1, 255));
	//ixPanel.add(dataName.setup("dataNombre", "none"));
	//ixPanel.add(sensorLocation.setup("sensorLoc", "none"));
	//ixPanel.add(ipAddress.setup("IP", "none"));
	//ixPanel.add(volume.setup(0, 0, 1));
	
	//ixPanel.add(volumeMin.setup(0.f, 0, 1));
	/*
	ixPanel.add(volumeMax.setup(0.f, 0, 1));
	ixPanel.add(waterDataMin.setup(0.f, -10000, 10000));
	ixPanel.add(waterDataMax.setup(0.f, -10000, 10000));
	ixPanel.add(noteMin.setup(1, 1, 32));
	ixPanel.add(noteMax.setup(2, 1, 32));
	ixPanel.add(waterDataReadInterval.setup(3000, 1, 60000));
	ixPanel.add(notePlayInterval.setup(1000, 1, 5000));
	ixPanel.add(distanceReadInterval.setup(1000 / 60, 1, 2000));
	ixPanel.add(distanceReadInterval.setup(1000 / 60, 1, 2000));
	ixPanel.add(distanceMin.setup(30, 0, 50 * 30));
	ixPanel.add(distanceMax.setup(20 * 30, 0, 50 * 30));
	ixPanel.add(distanceSignalStrength.setup(0, 0, 255));
	ixPanel.add(signalStrengthMin.setup(20, 0, 255));
	ixPanel.add(signalStrengthMax.setup(80, 0, 255));
	ixPanel.add(minSignalWeight.setup(0.05f, 0.f, 1.f));
	ixPanel.add(noiseDistance.setup(20, 0, 100));
	ixPanel.add(maxDistSamplesToSmooth.setup(1, 0, 100));
	ixPanel.add(heartbeatInterval.setup(1000, 0, 2000));

	ixPanel.add(guiSmoothedDistance.setup(0, 0, 50));
	ixPanel.add(guiTemperature.setup(-60, -60, 100));
	ixPanel.add(guiLowTemperature.setup(-60, -60, 100));
	*/
	ixPanel.add(guiHighTemperature.setup(0, -60, 100));
	

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


}

// ---------------------------------------------------------------------------- //
// setDistance
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
		distance = distanceMax*2;

		// We're within the noise distance
		// Ignore signal strength and go toward newVal quickly
		newWeight = newWeight * (1.f - signalStrengthMin);
	} else {
		// Weight by the signal strength
		newWeight = newWeight * ofMap(signalStrength, signalStrengthMin, signalStrengthMax, minSignalWeight, 1.f, true);
	}

	// Calculate the smoothed PWM value
	smoothedDistance = ((float) rawDistance)*newWeight + smoothedDistance*(1-newWeight);
	guiSmoothedDistance = smoothedDistance;
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
		ofLog(OF_LOG_VERBOSE) << typeTag << "<<" << ipAddress.getParameter().toString() << endl;

		// Parse DISTANCE Packets
		if (memcmp( header->typeTag, LivestreamNetwork::DISTANCE, 
			sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
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
			sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
			LivestreamNetwork::Packet_TEMPERATURE_V1* inPacket = (LivestreamNetwork::Packet_TEMPERATURE_V1 *) udpMessage;
			ofLog(OF_LOG_VERBOSE) << "T" << inPacket->sensorDesignator << ", " << inPacket->temperature << endl;
			setTemperature(inPacket->temperature);
		}
}
