#include "ofApp.h"

#define RECONNECT_TIME 400

//--------------------------------------------------------------
void ofApp::setup(){
	// we don't want to be running to fast
	ofSetVerticalSync(true);
	ofSetFrameRate(60);

    //create the socket and set to send to 127.0.0.1:11999
	udpSender.Create();
	udpSender.SetEnableBroadcast(true);
	udpSender.Connect("192.168.1.255",11999);
	//udpSender.Bind(11999);
	udpSender.SetNonBlocking(true);

	udpReceiver.Create();
	udpReceiver.Bind(11999);
	udpReceiver.SetNonBlocking(true);
}

//--------------------------------------------------------------
void ofApp::update(){
	
	char udpMessage[100000];
	udpReceiver.Receive(udpMessage,100000);
	string message=udpMessage;
	TestPacket* packet = (TestPacket*) &udpMessage;
	if(message!=""){
		//char address[20];
		string address;
		int port;
		udpReceiver.GetRemoteAddr(address, port);
		string udpAddress = address;
		string tempOut(packet->temp1, sizeof(packet->temp1));
		cout << "From " << udpAddress << ": " << tempOut << endl;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(20, 20, 20);
	ofDrawBitmapString("openFrameworks UDP Send Example ", 15, 30);
    ofDrawBitmapString("drag to draw", 15, 50);
	for(unsigned int i=1;i<stroke.size();i++){
		ofLine(stroke[i-1].x,stroke[i-1].y,stroke[i].x,stroke[i].y);
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	string message;
	TestPacket packet;
	switch( key ) {
		case '1':
			strncpy(packet.temp1, "AC", 2);
			break;
		case '2':
			strncpy(packet.temp1, "BD", 2);
			break;
		default:
			break;
	}
	string tempOut(packet.temp1, sizeof(packet.temp1));
	cout << "Broadcasting: " << tempOut << endl;
	udpSender.Send((char*) &packet, sizeof(TestPacket));
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	stroke.push_back(ofPoint(x,y));
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	stroke.clear();
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	string message="";
	for(unsigned int i=0; i<stroke.size(); i++){
		message+=ofToString(stroke[i].x)+"|"+ofToString(stroke[i].y)+"[/p]";
	}
	//udpSender.Send(message.c_str(),message.length());
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
