#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetLogLevel(OF_LOG_NOTICE);
	
	loopInterval = 5;

	// Get the host name
	int z;  
	char buf[32];  
	z = gethostname(buf,sizeof buf);
	if ( z != -1 ) {
		hostname = string(buf);
		cout << "host name = " 	<< hostname << endl;
	} else {
		cout << "host name unknown" << endl;
		hostname = "unknown";
	}
	
	// Write initialization to the log
	string logFileName = "/logs/livestream/livestream_" + hostname + ".log";
	ofstream mFile;
	mFile.open(logFileName.c_str(), ios::out | ios::app);
	mFile << getDateTimeString() << ",INITILIZATION";
	mFile << endl;
	mFile.close();


	cout << getDateTimeString() << ",INITILIZATION " << hostname;
	cout << endl;
	
	logTimer = ofGetElapsedTimeMillis();
	blinkTimer = ofGetSystemTimeMicros();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofSleepMillis(loopInterval);
	
	if (ofGetElapsedTimeMillis() - logTimer >= 60000) {
		logTimer = ofGetElapsedTimeMillis();
		// Log the data
		string logFileName = "/logs/livestream/livestream_" + hostname + ".log";
		ofstream mFile;
		mFile.open(logFileName.c_str(), ios::out | ios::app);
		mFile << getDateTimeString() << setprecision(3);
		mFile << ofGetFrameRate();
		mFile << endl;
		mFile.close();
	}
	
	if (ofGetSystemTimeMicros() - blinkTimer >= 1000000) {
		blinkTimer = ofGetSystemTimeMicros();
		cout << getDateTimeString() << setprecision(3) << ", LR, " << ofGetFrameRate();
		cout << endl;
	}
}

string ofApp::getDateTimeString() {
	string output = "";
	
	int year = ofGetYear();
	int month = ofGetMonth();
	int day = ofGetDay();
	int hours = ofGetHours();
	int minutes = ofGetMinutes();
	int seconds = ofGetSeconds();
	
	output = output + ofToString(year) + "-";
	if (month < 10) output = output + "0";
	output = output + ofToString(month) + "-";
	if (day < 10) output = output + "0";
	output = output + ofToString(day) + ",";
	if (hours < 10) output = output + "0";
	output = output + ofToString(hours) + ":";
	if (minutes < 10) output = output + "0";
	output = output + ofToString(minutes) + ":";
	if (seconds < 10) output = output + "0";
	output = output + ofToString(seconds);
	
	return output;
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

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