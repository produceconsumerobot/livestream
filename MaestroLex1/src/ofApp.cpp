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
	udpSender.Connect("192.168.254.255",11999);
	udpSender.SetNonBlocking(true);

	maestroIpAddress = "192.168.254.5";

	udpReceiver.Create();
	udpReceiver.Bind(11999);
	udpReceiver.SetNonBlocking(true);

	nPacketsSent = 0;
	packetProtocolVersion = 1;

	nInterXUnits = 1;
	interXUnit.resize(1);
	interXUnit.at(0).ipAddress = "192.168.254.2";
	interXUnit.at(0).udpSender.Create();
	interXUnit.at(0).udpSender.SetEnableBroadcast(false);
	interXUnit.at(0).udpSender.Connect(interXUnit.at(0).ipAddress.c_str(),11999);
	interXUnit.at(0).udpSender.SetNonBlocking(true);

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
		string address = udpAddress;
		ofLog(OF_LOG_VERBOSE) << "UDP MESSAGE << " << address << endl;

		for (int j=0; j<interXUnit.size(); j++) {
			if (interXUnit.at(j).ipAddress.compare(udpAddress) == 0) {
				ofLog(OF_LOG_VERBOSE) << "Address Match << " << interXUnit.at(j).ipAddress << ", " << address << endl;
				interXUnit.at(j).parseUdpPacket(udpMessage);
			}
		}
	} //message!=""
}

//--------------------------------------------------------------
void ofApp::draw(){
	/*
	ofSleepMillis(1000);

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
			// Play a note
			keyReleased('l');
			interXUnit.at(j).heartbeatTime = ofGetElapsedTimeMillis();
			cout << setprecision(3) 
				<< "LR, " << ofGetFrameRate() 
				<< ", LD, " << interXUnit.at(j).getSmoothedDistance()
				<< ", LS, " << interXUnit.at(j).distanceSignalStrength
				<< endl;
		}
	}
	*/
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

		LivestreamNetwork::PacketNoPayload_V1 outPacket;
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
		LivestreamNetwork::PacketNoPayload_V1 outPacket;
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
		LivestreamNetwork::PacketNoPayload_V1 outPacket;
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
		LivestreamNetwork::PacketNoPayload_V1 outPacket;
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
		LivestreamNetwork::PacketBool_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::SET_LED, 
			sizeof(LivestreamNetwork::SET_LED) / sizeof(LivestreamNetwork::SET_LED[0]));

		testLED = !testLED;
		outPacket.b = testLED;

		// Send the packet
		udpSender.Send((char*) &outPacket, sizeof(outPacket));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "broadcast" << endl;	

	} else if ((char) key == 'n') {
		// ********** PLAY_NOTE packet type ********** //
		// Load the packet data
		LivestreamNetwork::PacketNoPayload_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
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
		LivestreamNetwork::PacketNoPayload_V1 outPacket;
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
		LivestreamNetwork::PacketIPAddress_V1 outPacket;
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
