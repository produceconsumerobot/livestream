#include "ofApp.h"
#include "ofxThreadedLogger.h"

#define RECONNECT_TIME 400

//--------------------------------------------------------------
void ofApp::setup(){
	// Draw the run data to the screen
	int panelSpacing = 180;
	panelWidth = panelSpacing - 5;
	panelRowHeight = 350;

	ofxGuiSetFont("verdana.ttf", 6);
	ofxGuiSetDefaultHeight(10);
	ofxGuiSetDefaultWidth(panelWidth);
	
	maestroPanel.add(softwareVersion.setup("softwareVersion", "1.1.1"));

	// **** Network specific variables **** //

	//string baseIpAddress = "192.168.1.";		// Jacobson Park
	//maestroIpAddress = baseIpAddress + "201"; // Jacobson Park
	//logDir = "/livestream/logs/";
	//waterDataFilesLocation.setup("dataLoc", "/livestream/data/"); // Jacobson Park
	//soundFilesLocation.setup("soundLoc", "/livestream/audio/"); // Jacobson Park
																						 
	string baseIpAddress = "192.168.0.";		// Sean's network
	maestroIpAddress = baseIpAddress + "105";	// Sean's computer
	logDir = ofToDataPath("");
	waterDataFilesLocation.setup("dataLoc", ofToDataPath("") + "data\\"); // Sean's network
	soundFilesLocation.setup("soundLoc", "/home/pi/openFrameworks/apps/livestream/IXUnitLex1/bin/data/audio/"); // Sean's network

	// **** END Network specific variables **** //

	string broadcastIpAddress = baseIpAddress + "255";

	loggingOn.setup("loggingOn", true);

	ofSetVerticalSync(false);
	ofSetLogLevel(OF_LOG_ERROR);

    //create the socket and set to send to 127.0.0.1:11999
	udpBroadcaster.Create();
	udpBroadcaster.SetEnableBroadcast(true);
	udpBroadcaster.Connect(broadcastIpAddress.c_str(),11999);
	udpBroadcaster.SetNonBlocking(true);

	udpReceiver.Create();
	udpReceiver.Bind(11999);
	udpReceiver.SetNonBlocking(true);

	nPacketsSent = 0;
	packetProtocolVersion = 1;

	temperature = -100;
	lowTemperature = -100;
	highTemperature = -100;

	maestroPanel.setup("Maestro", "maestroSettings.xml", 0, 0);
	maestroPanel.add(maestroIpAddress.setup("IP", maestroIpAddress));
	maestroPanel.add(currentDateTime.setup("current date", ofGetTimestampString("%Y-%m-%d %H:%M:%S")));
	maestroPanel.add(lastStartupDateTime.setup("startup date", ofGetTimestampString("%Y-%m-%d %H:%M:%S")));
	maestroPanel.add(guiTemperature.setup("Temperature", -60, -60, 100));						// Celcius
	maestroPanel.add(guiLowTemperature.setup("LowTemperature", -60, -60, 100));					// Celcius
	maestroPanel.add(guiHighTemperature.setup("HighTemperature", -60, -60, 100));					// Celcius
	maestroPanel.add(frameRate.setup("frameRate", 0, 0, 1000));
	maestroPanel.add(soundOn.setup("soundOn", true));
	maestroPanel.add(&loggingOn);
	maestroPanel.add(temperatureLogInterval.setup("temperatureLogInterval", 60000));
	
	defaultSettingsPanel.setup("Default Settings", "defaultSettings.xml", 0, 150);
	defaultSettingsPanel.add(&waterDataFilesLocation); // Jacobson Park
	defaultSettingsPanel.add(&soundFilesLocation); // Jacobson Park
	defaultSettingsPanel.add(volumeMin.setup("volMin", 0, 0, 1));
	defaultSettingsPanel.add(volumeMax.setup("volMax", 1, 0, 1));
	defaultSettingsPanel.add(distanceReadInterval.setup("distanceReadInterval", 1000 / 5, 1, 3000));
	defaultSettingsPanel.add(waterDataReadInterval.setup("waterDataReadInterval", 1000, 1, 60000));
	defaultSettingsPanel.add(notePlayInterval.setup("notePlayInterval", 1000, 1, 10000));
	defaultSettingsPanel.add(signalStrengthMin.setup("signalStrengthMin", 20.f / 255.f, 0, 1.f));
	defaultSettingsPanel.add(signalStrengthMax.setup("signalStrengthMax", 80.f / 255.f, 0, 1.f));
	defaultSettingsPanel.add(minSignalWeight.setup("minSignalWeight", 0.05f, 0, 1));
	defaultSettingsPanel.add(noiseDistance.setup("noiseDistance", 20, 0, 255));
	defaultSettingsPanel.add(maxDistSamplesToSmooth.setup("maxDistSamplesToSmooth", 1, 0, 60));

	// Load settings from file
	//defaultSettingsPanel.loadFromFile("defaultSettings.xml");

	logger.setDirPath(logDir);
	logger.setFilename("livestream_MaestroLex1_" + ofGetTimestampString("%m") + ".log");
	logger.startThread();

	if (loggingOn) {
		// Log to file
		ostringstream  logStringStream;
		logStringStream << "MV," << ofGetTimestampString("%Y%m%d,%H%M%S,") << softwareVersion.getParameter().toString() << endl;
		logger.push(logStringStream.str());
	}

	// Setup logger for each IXUnit
	//for (int j = 0; j < interXUnit.size(); j++) {
	//	interXUnit.at(j).logger = &logger;
	//}
	//loggingOn = false;
	
	interXUnit.resize(9);
	int ixID;
	int i;
	ixID = 13;
	i = 0;
	interXUnit.at(0).setup(ixID, baseIpAddress + "2" + to_string(ixID), "Coldspring", "Conductivity", waterDataFilesLocation.getParameter().toString());
	interXUnit.at(i).waterDataMin = 318;
	interXUnit.at(i).waterDataMax = 900;
	interXUnit.at(i).noteMax = 13;
	interXUnit.at(0).ixPanel.setPosition(panelSpacing * 1, 0);
	ixID = 17;
	i = 1;
	interXUnit.at(i).setup(ixID, baseIpAddress + "2" + to_string(ixID), "McSpring", "Temp", waterDataFilesLocation.getParameter().toString());
	interXUnit.at(i).waterDataMin = 12;
	interXUnit.at(i).waterDataMax = 20;
	interXUnit.at(i).noteMax = 17;
	interXUnit.at(i).ixPanel.setPosition(panelSpacing * 2, 0);
	ixID = 11;
	i = 2;
	interXUnit.at(i).setup(ixID, baseIpAddress + "2" + to_string(ixID), "Lost", "Flow", waterDataFilesLocation.getParameter().toString());
	interXUnit.at(i).waterDataMin = 4;
	interXUnit.at(i).waterDataMax = 42;
	interXUnit.at(i).noteMax = 13;
	interXUnit.at(i).ixPanel.setPosition(panelSpacing * 3, 0);
	ixID = 14;
	i = 3;
	interXUnit.at(i).setup(ixID, baseIpAddress + "2" + to_string(ixID), "Lost", "Temp", waterDataFilesLocation.getParameter().toString());
	interXUnit.at(i).waterDataMin = 13.1;
	interXUnit.at(i).waterDataMax = 20.1;
	interXUnit.at(i).noteMax = 17;
	interXUnit.at(i).ixPanel.setPosition(panelSpacing * 4, 0);
	ixID = 16;
	i = 4;
	interXUnit.at(i).setup(ixID, baseIpAddress + "2" + to_string(ixID), "McSpring", "Conductivity", waterDataFilesLocation.getParameter().toString());
	interXUnit.at(i).waterDataMin = 19;
	interXUnit.at(i).waterDataMax = 1714;
	interXUnit.at(i).noteMax = 13;
	interXUnit.at(i).ixPanel.setPosition(panelSpacing * 0, panelRowHeight);
	ixID = 21;
	i = 5;
	interXUnit.at(i).setup(ixID, baseIpAddress + "2" + to_string(ixID), "McSpring", "Flow", waterDataFilesLocation.getParameter().toString());
	interXUnit.at(i).waterDataMin = 0.1;
	interXUnit.at(i).waterDataMax = 50;
	interXUnit.at(i).noteMax = 13;
	interXUnit.at(i).ixPanel.setPosition(panelSpacing * 1, panelRowHeight);
	ixID = 22;
	i = 6;
	interXUnit.at(i).setup(ixID, baseIpAddress + "2" + to_string(ixID), "Coldspring", "Flow", waterDataFilesLocation.getParameter().toString());
	interXUnit.at(i).waterDataMin = 0.02;
	interXUnit.at(i).waterDataMax = 25;
	interXUnit.at(i).noteMax = 13;
	interXUnit.at(i).ixPanel.setPosition(panelSpacing * 2, panelRowHeight);
	ixID = 19;
	i = 7;
	interXUnit.at(i).setup(ixID, baseIpAddress + "2" + to_string(ixID), "Coldspring", "Temp", waterDataFilesLocation.getParameter().toString());
	interXUnit.at(i).waterDataMin = 9.5;
	interXUnit.at(i).waterDataMax = 11.6;
	interXUnit.at(i).noteMax = 17;
	interXUnit.at(i).ixPanel.setPosition(panelSpacing * 3, panelRowHeight);
	ixID = 18;
	i = 8;
	interXUnit.at(i).setup(ixID, baseIpAddress + "2" + to_string(ixID), "Lost", "Conductivity", waterDataFilesLocation.getParameter().toString());
	interXUnit.at(i).waterDataMin = 13.6;
	interXUnit.at(i).waterDataMax = 684;
	interXUnit.at(i).noteMax = 13;
	interXUnit.at(i).ixPanel.setPosition(panelSpacing * 4, panelRowHeight);

	testLED = false;
	udpSendTimersOn = true;
	//loggingOn = false;
}

//--------------------------------------------------------------
void ofApp::update(){
	string message = "yomama";
	while (message != "") {
		char udpMessage[100000];
		udpReceiver.Receive(udpMessage,100000);
		message=udpMessage;
		if (message != "") {
			// We got a message!
			//char udpAddress[20];
			string udpAddress;
			int port;
			// Get the sender's address
			udpReceiver.GetRemoteAddr(udpAddress, port);

			// Convert the UDP message to read its type tag from the header section
			LivestreamNetwork::PacketHeader_V1* header =
				(LivestreamNetwork::PacketHeader_V1 *) udpMessage;
			string typeTag(header->typeTag, header->typeTag + sizeof(header->typeTag) / sizeof(header->typeTag[0]));

			if (maestroIpAddress.getParameter().toString().compare(udpAddress) != 0) {
				ofLog(OF_LOG_VERBOSE) << udpAddress << " >> " << typeTag << endl;
			}

			for (int j = 0; j < interXUnit.size(); j++) {
				if (interXUnit.at(j).ipAddress.getParameter().toString().compare(udpAddress) == 0) {
					ofLog(OF_LOG_VERBOSE) << udpAddress << " >> Address Match (" << interXUnit.at(j).ipAddress.getParameter().toString() << ")" << endl;
					interXUnit.at(j).parseUdpPacket(udpMessage);
				}
			}
		}
	} //message!=""
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackgroundGradient(ofColor::white, ofColor::gray);
	
	if (udpSendTimersOn)
	{
		bool distanceTripped = false;

		for (int j = 0; j<interXUnit.size(); j++) {
			if (ofGetElapsedTimeMillis() - interXUnit.at(j).distanceReadTime > interXUnit.at(j).distanceReadInterval) {
				// Read the distance
				interXUnit.at(j).getDistance();
				interXUnit.at(j).distanceReadTime = ofGetElapsedTimeMillis();
			}
			if (ofGetElapsedTimeMillis() - interXUnit.at(j).waterDataReadTime > interXUnit.at(j).waterDataReadInterval) {
				// Read the water data
				interXUnit.at(j).readWaterData();
				interXUnit.at(j).waterDataReadTime = ofGetElapsedTimeMillis();
			}
			if (ofGetElapsedTimeMillis() - interXUnit.at(j).notePlayTime > interXUnit.at(j).notePlayInterval) {
				// Play a note
				if (soundOn) {
					interXUnit.at(j).playNote(soundFilesLocation.getParameter().toString());
				}
				//keyReleased('n');
				interXUnit.at(j).notePlayTime = ofGetElapsedTimeMillis();
			}

			if (ofGetElapsedTimeMillis() - interXUnit.at(j).heartbeatTime > interXUnit.at(j).heartbeatInterval) {
				// Ping the IXUnit
				interXUnit.at(j).ping();
				// blink the heartbeat LED
				interXUnit.at(j).setLed();
				// Send the Maestro IP address
				interXUnit.at(j).setMaestroAddress(maestroIpAddress);
				// Get all temps
				interXUnit.at(j).getAllTemps();

				if (interXUnit.at(j).guiSmoothedDistance > interXUnit.at(j).distanceMin
					&& interXUnit.at(j).guiSmoothedDistance < interXUnit.at(j).distanceMax)
				{
					distanceTripped = true;
				}

				if (j == 1) {
					// Set the logger path so a new file is created every month
					logger.setFilename("livestream_MaestroLex1_" + ofGetTimestampString("%m") + ".log");
				}

				interXUnit.at(j).heartbeatTime = ofGetElapsedTimeMillis();
			}
		}

		if (distanceTripped) {
			if (loggingOn) {
				// Log the distances if any were "tripped" in the previous heartbeat interval
				ostringstream  logStringStream;
				logStringStream << "LD," << ofGetTimestampString("%Y%m%d,%H%M%S");
				for (int j = 0; j < interXUnit.size(); j++) {
					logStringStream << ",L" << interXUnit.at(j).id << "," << interXUnit.at(j).guiSmoothedDistance;
				}
				logStringStream << endl;
				logger.push(logStringStream.str());
			}
		}

		if (ofGetElapsedTimeMillis() - temperatureLogTime > temperatureLogInterval) {
			// Get Maestros CPU temp
#ifdef TARGET_LINUX
			FILE *temperatureFile;
			double T;
			temperatureFile = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
			if (temperatureFile == NULL) {
				cout << "Failed to read temp file\n";
			}
			else {
				fscanf(temperatureFile, "%lf", &T);
				T /= 1000;
				fclose(temperatureFile);
			}
			setTemperature((int)T);
#endif
			if (loggingOn) {
				ostringstream  logStringStream;
				logStringStream << "TN," << ofGetTimestampString("%Y%m%d,%H%M%S,M,") << temperature;

				for (int j = 0; j < interXUnit.size(); j++) {
					logStringStream << ",L" << interXUnit.at(j).id << "," << interXUnit.at(j).guiTemperature;
				}
				logStringStream << endl;
				logger.push(logStringStream.str());
			}

			temperatureLogTime = ofGetElapsedTimeMillis();
		}
	}

	for (int j = 0; j < interXUnit.size(); j++) {
		interXUnit.at(j).ixPanel.draw();
	}
	
	// Draw the run data to the screen
	currentDateTime = ofGetTimestampString("%Y-%m-%d %H:%M:%S");
	defaultSettingsPanel.draw();
	maestroPanel.draw();

	frameRate = (int) ofGetFrameRate();
}

// ---------------------------------------------------------------------------- //
// setTemperature
// ---------------------------------------------------------------------------- //
void ofApp::setTemperature(int temp) {
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

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	string typeTag;
	if ((char) key == 'p') {
		// ********** PING packet type ********** //
		// Load the packet data

		LivestreamNetwork::Packet_PING_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::PING, 
			sizeof(LivestreamNetwork::PING) / sizeof(LivestreamNetwork::PING[0]));

		// Send the packet
		udpBroadcaster.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << "broadcast" << endl;	

	} else if ((char) key == 'd') {
		// ********** GET_DISTANCE packet type ********** //
		// Load the packet data
		LivestreamNetwork::Packet_GET_DISTANCE_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::GET_DISTANCE, 
			sizeof(LivestreamNetwork::GET_DISTANCE) / sizeof(LivestreamNetwork::GET_DISTANCE[0]));

		// Send the packet
		udpBroadcaster.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << "broadcast" << endl;	
			/*
			ofLog(OF_LOG_VERBOSE) 
				<< "nPacketsSent, " << outPacket.hdr.packetCount
				<< ", timeStamp, " << outPacket.hdr.timeStamp
				<< ", ver, " << outPacket.hdr.protocolVersion
				<< endl;
			*/

	} else if ((char) key == 's') {
		// ********** MODE_SLAVE packet type ********** //
		// Load the packet data
		LivestreamNetwork::Packet_MODE_SLAVE_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::MODE_SLAVE, 
			sizeof(LivestreamNetwork::MODE_SLAVE) / sizeof(LivestreamNetwork::MODE_SLAVE[0]));

		// Send the packet
		udpBroadcaster.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << "broadcast" << endl;	

	} else if ((char) key == 'm') {
		// ********** MODE_MASTER packet type ********** //
		// Load the packet data
		LivestreamNetwork::Packet_MODE_MASTER_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::MODE_MASTER, 
			sizeof(LivestreamNetwork::MODE_MASTER) / sizeof(LivestreamNetwork::MODE_MASTER[0]));

		// Send the packet
		udpBroadcaster.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << "broadcast" << endl;

	}  else if ((char) key == 'l') {
		// ********** SET_LED packet type ********** //
		// Load the packet data
		LivestreamNetwork::Packet_SET_LED_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::SET_LED, 
			sizeof(LivestreamNetwork::SET_LED) / sizeof(LivestreamNetwork::SET_LED[0]));

		testLED = !testLED;
		outPacket.state = testLED;

		// Send the packet
		udpBroadcaster.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << "broadcast" << endl;	

	} else if ((char) key == 'n') {
		// ********** PLAY_NOTE packet type ********** //
		// Load the packet data
		LivestreamNetwork::Packet_PLAY_NOTE_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		//outPacket.pitch = 0;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::PLAY_NOTE, 
			sizeof(LivestreamNetwork::PLAY_NOTE) / sizeof(LivestreamNetwork::PLAY_NOTE[0]));

		// Send the packet
		udpBroadcaster.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << "broadcast" << endl;	
	} else if ((char) key == 't') {
		// ********** GET_ALL_TEMPS packet type ********** //
		// Load the packet data
		LivestreamNetwork::Packet_GET_ALL_TEMPS_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::GET_ALL_TEMPS, 
			sizeof(LivestreamNetwork::GET_ALL_TEMPS) / sizeof(LivestreamNetwork::GET_ALL_TEMPS[0]));

		// Send the packet
		udpBroadcaster.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << " >> " << "broadcast" << endl;	
	} else if ((char) key == 'a') {
		// ********** SET_MAESTRO_ADDRESS packet type ********** //
		// Load the packet data
		LivestreamNetwork::Packet_SET_MAESTRO_ADDRESS_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::SET_MAESTRO_ADDRESS, 
			sizeof(LivestreamNetwork::SET_MAESTRO_ADDRESS) / sizeof(LivestreamNetwork::SET_MAESTRO_ADDRESS[0]));
		strcpy(outPacket.ipAddress, maestroIpAddress.getParameter().toString().c_str());

		// Send the packet
		udpBroadcaster.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << " (" << maestroIpAddress.getParameter().toString() << ") >> " << "broadcast" << endl;
	}
	//else if ((char) key == '!') {
	//	udpSendTimersOn = !udpSendTimersOn;
	//}


}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

/*
void ofApp::loggingOnChanged(bool & logOn) {
	for (int j = 0; j<interXUnit.size(); j++) {
		interXUnit.at(j).loggingOn = loggingOn;
	}
}
*/
