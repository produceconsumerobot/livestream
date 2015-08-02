#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(false);
	//ofSetFrameRate(10000)
	
	counter = 0;
	sysMicros = ofGetSystemTimeMicros();
	
	gpio15  = new GPIO("15");
	gpio15->export_gpio();
	gpio15->setdir_gpio("out");
	gpio15->setval_gpio("0");
	gpio15outState = false;
	
	gpio21  = new GPIO("21");
	gpio21->export_gpio();
	gpio21->setdir_gpio("out");
	gpio21->setval_gpio("0");
	gpio21outState = false;

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	//ofSleepMillis(100);
	
	/*
	if (gpio15outState) {
		gpio15->setval_gpio("0");
		gpio15outState = false;
	} else {
		gpio15->setval_gpio("1");
		gpio15outState = true;
	}
	*/
	
	counter++;
	//cout << ofGetSystemTimeMicros() << "," << sysMicros << endl;
	if (ofGetSystemTimeMicros() - sysMicros >= 1000000) {
		cout << counter << "loops/sec, " << ofGetFrameRate() << "Hz" << endl;
		
		if (gpio15outState) {
			gpio15->setval_gpio("0");
			gpio15outState = false;
		} else {
			gpio15->setval_gpio("1");
			gpio15outState = true;
		}
		
		if (gpio21outState) {
			gpio21->setval_gpio("0");
			gpio21outState = false;
		} else {
			gpio21->setval_gpio("1");
			gpio21outState = true;
		}
		
		sysMicros = ofGetSystemTimeMicros();
		counter = 0;
	}
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