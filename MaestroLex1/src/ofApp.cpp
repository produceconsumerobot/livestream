#include "ofApp.h"
#include "logger.h"
//#include "LivestreamNetwork.h"

#define RECONNECT_TIME 400

//--------------------------------------------------------------
void ofApp::setup(){
	string baseIpAddress = "192.168.1.";		// Jacobson Park
	maestroIpAddress = baseIpAddress + "201"; // Jacobson Park
	//string baseIpAddress = "192.168.0.";		// Sean's network
	//maestroIpAddress = baseIpAddress + "103";	// Sean's computer
	string broadcastIpAddress = baseIpAddress + "255";

	// we don't want to be running to fast
	ofSetVerticalSync(false);
	//ofSetFrameRate(60);
	ofSetLogLevel(OF_LOG_VERBOSE);

    //create the socket and set to send to 127.0.0.1:11999
	udpSender.Create();
	udpSender.SetEnableBroadcast(true);
	udpSender.Connect(broadcastIpAddress.c_str(),11999);
	udpSender.SetNonBlocking(true);

	udpReceiver.Create();
	udpReceiver.Bind(11999);
	udpReceiver.SetNonBlocking(true);

	nPacketsSent = 0;
	packetProtocolVersion = 1;

	// Draw the run data to the screen
	int panelSpacing = 190;
	panelWidth = panelSpacing - 5;
	panelRowHeight = 375;

	ofxGuiSetFont("verdana.ttf", 6);
	ofxGuiSetDefaultHeight(10);
	ofxGuiSetDefaultWidth(panelWidth);

	maestroPanel.setup("Maestro", "maestroSettings.xml", 0, 0);
	maestroPanel.add(maestroIpAddress.setup("IP", maestroIpAddress));
	maestroPanel.add(currentDateTime.setup("current date", LoggerThread::dateTimeString()));
	maestroPanel.add(lastStartupDateTime.setup("startup date", LoggerThread::dateTimeString()));
	maestroPanel.add(currentTemp.setup(string("0.0C")));
	maestroPanel.add(lowTemp.setup(string("0.0C")));
	maestroPanel.add(highTemp.setup(string("0.0C")));
	
	defaultSettingsPanel.setup("Default Settings", "defaultSettings.xml", 0, 100);
	//defaultSettingsPanel.add(waterDataFilesLocation.setup("dataLoc", "/livestream/data/"));
	defaultSettingsPanel.add(waterDataFilesLocation.setup("dataLoc", "C:\\pub\\LocalDev\\Sean\\of_v0.9.3_vs_release\\of_v0.9.3_vs_release\\apps\\livestream\\MaestroLex1\\bin\\data\\data\\"));
	//defaultSettingsPanel.add(soundFilesLocation.setup("soundLoc", "/livestream/audio/"));
	defaultSettingsPanel.add(soundFilesLocation.setup("soundLoc", "/home/pi/openFrameworks/apps/livestream/IXUnitLex1/bin/data/audio/"));
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
	defaultSettingsPanel.add(soundOn.setup("soundOn", true));
	// Load settings from file
	//defaultSettingsPanel.loadFromFile("defaultSettings.xml");
	
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
}

//--------------------------------------------------------------
void ofApp::update(){
	char udpMessage[100000];
	udpReceiver.Receive(udpMessage,100000);
	string message=udpMessage;
	if(message!=""){
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

		for (int j=0; j<interXUnit.size(); j++) {
			if (interXUnit.at(j).ipAddress.getParameter().toString().compare(udpAddress) == 0) {
				ofLog(OF_LOG_VERBOSE) << udpAddress << " >> Address Match (" << interXUnit.at(j).ipAddress.getParameter().toString() << ")" << endl;
				interXUnit.at(j).parseUdpPacket(udpMessage);
			}
		}
	} //message!=""
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackgroundGradient(ofColor::white, ofColor::gray);
	
	//ofSleepMillis(1000);

	if (udpSendTimersOn)
	{
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

				interXUnit.at(j).heartbeatTime = ofGetElapsedTimeMillis();
			}
		}
	}

	for (int j = 0; j < interXUnit.size(); j++) {
		interXUnit.at(j).ixPanel.draw();
	}
	
	// Draw the run data to the screen

	currentDateTime = LoggerThread::dateTimeString();
	defaultSettingsPanel.draw();
	maestroPanel.draw();

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
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
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
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
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
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
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
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
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
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
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
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
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
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
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
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << " (" << maestroIpAddress.getParameter().toString() << ") >> " << "broadcast" << endl;
	}
	else if ((char) key == '!') {
		udpSendTimersOn = !udpSendTimersOn;
	}


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
