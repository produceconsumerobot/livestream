#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	//we run at 60 fps!
	//ofSetVerticalSync(true);
	//ofSetFrameRate(60);

  //create the socket and bind to port 11999
	udpReceiver.Create();
	udpReceiver.Bind(11999);
	udpReceiver.SetNonBlocking(true);

	maestroIpAddress = "10.0.0.3";
	udpSender.Create();
	udpSender.Connect(maestroIpAddress.c_str(),11999);
	udpSender.SetNonBlocking(true);
}

//--------------------------------------------------------------
void ofApp::update(){

	char udpMessage[100000];
	udpReceiver.Receive(udpMessage,100000);
	string message=udpMessage;
	if(message!=""){
		//stroke.clear();
		//float x,y;
		//vector<string> strPoints = ofSplitString(message,"[/p]");
		//for(unsigned int i=0;i<strPoints.size();i++){
		//	vector<string> point = ofSplitString(strPoints[i],"|");
		//	if( point.size() == 2 ){
		//		x=atof(point[0].c_str());
		//		y=atof(point[1].c_str());
		//		stroke.push_back(ofPoint(x,y));
		//	}
		char address[20];
		udpReceiver.GetRemoteAddr(address);
		string udpAddress = address;
		//cout << "From " << udpAddress << ": " << message << endl;
		TestPacket* packet = (TestPacket*) &udpMessage;
		string tempOut(packet->temp1, sizeof(packet->temp1));
		cout << "From " << udpAddress << ": " << tempOut << endl;
		//cout << "From " << udpAddress << ": " << (char) inPacket->temp1[0] << (char) inPacket->temp1[1] << endl;
		//cout << "From " << udpAddress << ": " << packet->temp1 << endl;
		
		if (udpAddress.compare(maestroIpAddress) == 0) {
			//string newMessage = "I heard " + ofToString(packet->temp1) + ofToString(packet->temp2);
			TestPacket outPacket;
			if (memcmp( inPacket, "AC", 2) == 0) {
				strncpy(outPacket.temp1, "13", 2);
			} else if (memcmp( inPacket, "BD", 2) == 0) {
				strncpy(outPacket.temp1, "24", 2);
			}
			string tempOut(packet->temp1, sizeof(packet->temp1));
			cout << "Sending: " << tempOut << endl;
			//cout << "Sending: " << (char) outPacket.temp1[0] << (char) outPacket.temp1[1] << endl;
			udpSender.Send((char*) &outPacket, sizeof(TestPacket));
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
  //  ofFill();
  //  ofSetHexColor(0xFFFFFF);
  //  ofRect(0,0,200,30);
	//ofSetHexColor(0x101010);
	//ofDrawBitmapString("UDP Receiver Example ", 10, 20);

	//for(unsigned int i=1;i<stroke.size();i++){
	//	ofLine(stroke[i-1].x,stroke[i-1].y,stroke[i].x,stroke[i].y);
	//}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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
