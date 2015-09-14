#include "ofApp.h"
//#include "LivestreamNetwork.h"

#define RECONNECT_TIME 400

//--------------------------------------------------------------
void ofApp::setup(){
	// we don't want to be running to fast
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofSetLogLevel(OF_LOG_VERBOSE);

    //create the socket and set to send to 127.0.0.1:11999
	udpSender.Create();
	udpSender.SetEnableBroadcast(true);
	udpSender.Connect("10.0.0.255",11999);
	udpSender.SetNonBlocking(true);

	udpReceiver.Create();
	udpReceiver.Bind(11999);
	udpReceiver.SetNonBlocking(true);

	nPacketsSent = 0;
	packetProtocolVersion = 1;

	nInterXUnits = 1;
	interXUnit.resize(1);
	interXUnit.at(0).ipAddress = "10.0.0.8";
	interXUnit.at(0).udpSender.Create();
	interXUnit.at(0).udpSender.SetEnableBroadcast(false);
	interXUnit.at(0).udpSender.Connect(interXUnit.at(0).ipAddress.c_str(),11999);
	interXUnit.at(0).udpSender.SetNonBlocking(true);
}

//--------------------------------------------------------------
void ofApp::update(){
	char udpMessage[100000];
	udpReceiver.Receive(udpMessage,100000);
	string message=udpMessage;
	if(message!=""){
		// We got a message!
		char udpAddress[20];
		// Get the sender's address
		udpReceiver.GetRemoteAddr(udpAddress);
		string address = udpAddress;
		ofLog(OF_LOG_VERBOSE) << "UDP MESSAGE << " << address << endl;

		for (int j=0; j<interXUnit.size(); j++) {
			if (interXUnit.at(j).ipAddress.compare(address) == 0) {
				ofLog(OF_LOG_VERBOSE) << "Address Match << " << interXUnit.at(j).ipAddress << ", " << address << endl;
				interXUnit.at(j).parseUdpPacket(udpMessage);
			}
		}
	} //message!=""
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	string typeTag;
	if ((char) key == 'p') {
		// Load the packet data
		LivestreamNetwork::PacketNoPayload_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::PING, 
			sizeof(LivestreamNetwork::PING) / sizeof(LivestreamNetwork::PING[0]));

		// Send the packet
		udpSender.Send((char*) &outPacket, sizeof(LivestreamNetwork::PacketNoPayload_V1));
		// Convert the typeTage char[2] to a string for logging
		typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / sizeof(outPacket.hdr.typeTag[0]));
		ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "broadcast" << endl;	
	} else if ((char) key == 'd') {
		// Load the packet data
		LivestreamNetwork::PacketNoPayload_V1 outPacket;
		outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
		outPacket.hdr.packetCount = ++nPacketsSent;
		outPacket.hdr.protocolVersion = packetProtocolVersion;
		strncpy(outPacket.hdr.typeTag, LivestreamNetwork::GET_DISTANCE, 
			sizeof(LivestreamNetwork::GET_DISTANCE) / sizeof(LivestreamNetwork::GET_DISTANCE[0]));

		// Send the packet
		udpSender.Send((char*) &outPacket, sizeof(LivestreamNetwork::PacketNoPayload_V1));
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
