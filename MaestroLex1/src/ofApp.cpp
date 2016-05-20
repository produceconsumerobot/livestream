#include "ofApp.h"
//#include "LivestreamNetwork.h"

#define RECONNECT_TIME 400

//--------------------------------------------------------------
void ofApp::setup(){
	// we don't want to be running to fast
	ofSetVerticalSync(false);
	//ofSetFrameRate(60);
	ofSetLogLevel(OF_LOG_VERBOSE);

    //create the socket and set to send to 127.0.0.1:11999
	udpSender.Create();
	udpSender.SetEnableBroadcast(true);
	udpSender.Connect("192.168.1.255",11999);
	udpSender.SetNonBlocking(true);

	maestroIpAddress = "192.168.1.201";

	udpReceiver.Create();
	udpReceiver.Bind(11999);
	udpReceiver.SetNonBlocking(true);

	nPacketsSent = 0;
	packetProtocolVersion = 1;



	// Draw the run data to the screen
	int panelSpacing = 200;
	panelWidth = panelSpacing - 5;
	panelRowHeight = 300;

	ofxGuiSetFont("verdana.ttf", 6);
	ofxGuiSetDefaultHeight(10);
	ofxGuiSetDefaultWidth(panelWidth);

	maestroPanel.setup("Maestro", "maestroSettings.xml", 0, 0);
	maestroPanel.add(datetimeString.setup(string("current date")));
	maestroPanel.add(lastStartupTime.setup(string("startup date")));
	maestroPanel.add(currentTemp.setup(string("0.0C")));
	maestroPanel.add(lowTemp.setup(string("0.0C")));
	maestroPanel.add(highTemp.setup(string("0.0C")));
	
	globalSettingsPanel.setup("Default Settings", "defaultSettings.xml", 0, 100);
	globalSettingsPanel.add(waterDataFilesLocation.setup("dataLoc", "/livestream/data/"));
	globalSettingsPanel.add(soundFilesLocation.setup("soundLoc", "/livestream/audio/"));
	globalSettingsPanel.add(volumeMin.setup("volMin", 0, 0, 1));
	globalSettingsPanel.add(volumeMax.setup("volMax", 1, 0, 1));
	globalSettingsPanel.add(waterDataReadInterval.setup("dataReadInterval", 3000, 1, 60000));
	globalSettingsPanel.add(distanceReadInterval.setup("distanceReadInterval", 1000 / 60, 1, 3000));
	globalSettingsPanel.add(notePlayInterval.setup("notePlayInterval", 1000, 1, 10000));
	globalSettingsPanel.add(signalStrengthMin.setup("signalStrengthMin", 20, 0, 255));
	globalSettingsPanel.add(signalStrengthMax.setup("signalStrengthMax", 80, 0, 255));
	globalSettingsPanel.add(minSignalWeight.setup("minSignalWeight", 0.05f, 0, 1));
	globalSettingsPanel.add(noiseDistance.setup("noiseDistance", 20, 0, 255));
	globalSettingsPanel.add(maxDistSamplesToSmooth.setup("maxDistSamplesToSmooth", 1, 0, 60));
	// Load settings from file
	globalSettingsPanel.loadFromFile("globalSettings.xml");
	
	interXUnit.resize(9);
	interXUnit.at(0).setup(11, "192.168.1.102");
	interXUnit.at(0).ixPanel.setPosition(panelSpacing * 1, 0);


	testLED = false;
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
		//string address = udpAddress;
		ofLog(OF_LOG_VERBOSE) << "UDP MESSAGE << " << udpAddress << endl;

		for (int j=0; j<interXUnit.size(); j++) {
			if (interXUnit.at(j).ipAddress.getParameter().toString().compare(udpAddress) == 0) {
				ofLog(OF_LOG_VERBOSE) << "Address Match << " << interXUnit.at(j).ipAddress.getParameter().toString() << ", " << udpAddress << endl;
				interXUnit.at(j).parseUdpPacket(udpMessage);
			}
		}
	} //message!=""
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackgroundGradient(ofColor::white, ofColor::gray);
	
	//ofSleepMillis(1000);


	for (int j=0; j<interXUnit.size(); j++) {
		if (ofGetElapsedTimeMillis() - interXUnit.at(j).distanceReadTime > interXUnit.at(j).distanceReadInterval) {
			// Read the distance
			keyReleased('d');
			interXUnit.at(j).distanceReadTime = ofGetElapsedTimeMillis();
		}
		if (ofGetElapsedTimeMillis() - interXUnit.at(j).notePlayTime > interXUnit.at(j).notePlayInterval) {
			// Play a note
			keyReleased('n');
			interXUnit.at(j).notePlayTime = ofGetElapsedTimeMillis();
		}
		if (ofGetElapsedTimeMillis() - interXUnit.at(j).heartbeatTime > interXUnit.at(j).heartbeatInterval) {
			// blink the heartbeat LED
			keyReleased('l');
			// Send the Maestro IP address
			keyReleased('a');
			// Get all temps
			keyReleased('t');
			interXUnit.at(j).heartbeatTime = ofGetElapsedTimeMillis();
			cout << setprecision(3) 
				<< "LR, " << ofGetFrameRate() 
				<< ", LD, " << interXUnit.at(j).getSmoothedDistance()
				<< ", LS, " << interXUnit.at(j).guiSignalStrength
				<< endl;
		}
		interXUnit.at(j).ixPanel.draw();
	}
	
	// Draw the run data to the screen


	globalSettingsPanel.draw();
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
		ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "broadcast" << endl;	

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
		ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "broadcast" << endl;	
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
		ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "broadcast" << endl;	

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
		ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "broadcast" << endl;

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
		ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "broadcast" << endl;	

	} else if ((char) key == 'n') {
		// ********** PLAY_NOTE packet type ********** //
		// Load the packet data
		LivestreamNetwork::Packet_PLAY_NOTE_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		outPacket.pitch = 0;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::PLAY_NOTE, 
			sizeof(LivestreamNetwork::PLAY_NOTE) / sizeof(LivestreamNetwork::PLAY_NOTE[0]));

		// Send the packet
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "broadcast" << endl;	
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
		ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "broadcast" << endl;	
	} else if ((char) key == 'a') {
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
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "broadcast" << endl;	
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
