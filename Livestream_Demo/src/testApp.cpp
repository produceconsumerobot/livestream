#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	cout << "setup()" << endl;

	// **** Setup frame rate **** //
	ofSetVerticalSync(true);
	ofSetFrameRate(60);

	// **** Setup sensor conversion variables **** //
	maxSensorDist = 25.0;		// total range of sensor
	maxOuputDist = 10.0;		// target output range of sensor
	maxAnalogValue = 1024;		// ADC bit resolution
	maxOutputValue = 127;		// maximum value of output

	// **** Setup timers **** //
	resendNote = true;
	noteResendTime = ofGetFrameRate()*60; // duration between note resend events
	noteResendCounter = 0;
	blink13On = true;
	blinkCounter = 0;

	// **** Setup arduino **** //
	// Arduino Ports for Sean's Windows computer
	string arduinoPorts_arr[nSensors] = 
	{
		"COM9",
		"COM8"
	};
	// Copy array to a vector for safety
	std::vector< string > arduinoPorts(arduinoPorts_arr, arduinoPorts_arr + nSensors);
	arduinos.resize(nSensors);
	bSetupArduinos.resize(nSensors);
	sensorAnalogPin = 0;
	int arduinoBaud = 57600; // baud rate
	for (int i=0; i<nSensors; i++) {
		cout << "Setting up " << arduinoPorts.at(i) << endl;
		arduinos.at(i).connect(arduinoPorts.at(i), arduinoBaud);
		//arduinos.at(i).connect("COM8", arduinoBaud);
		bSetupArduinos.at(i) = false;				// it's not safe to send commands to Arduino
		while(!arduinos.at(i).isArduinoReady());	// Wait till the Arduino is ready
		setupArduino(arduinos.at(i));
		bSetupArduinos.at(i) = true;				// it is now safe to send commands to the Arduino
	}

	// **** Setup midi **** //
	// Midi Port
	// midiPort = 0; // Mac
	midiPort = 1; // PC
	midiout.listPorts();
	midiout.openPort(midiPort);
	midiMapMode = false;

	// MIDI parameters
	midiChannel = 1;
	midiValue = 127;
	int midiIds_arr[nSensors] = { // midiIDs that each sensor controls
		60,
		70
	};
	midiIds.resize(nSensors);
	for (int i=0; i<nSensors; i++) { 
		midiIds.at(i) = midiIds_arr[i]; // Copy array to a vector for safety
		midiout.sendNoteOn(midiChannel, midiIds.at(i), midiValue);
	}

	soundCheck = 0;	// for debugging sound

	// **** Filter Setup **** //
	smoothData.resize(nSensors);
	float Fs = ofGetFrameRate(); // sampling freq (Hz)
	float smoothDuration = 1; // seconds
	maxSamplesToSmooth = Fs*smoothDuration; // determines degree of smoothing
	nSamplesToSmooth = 0; // starting with 0 and incrementing up to max converges smoothed average to initial values quickly

}

// Setup the passed arduino
//--------------------------------------------------------------
void testApp::setupArduino(ofArduino & ard) {
	cout << "setupArduino()" << endl;
	
	// remove listener because we don't need it anymore
	//ofRemoveListener(ard.EInitialized, this, &testApp::setupArduino);
    
    // print firmware name and version to the console
    ofLogNotice() << ard.getFirmwareName(); 
    ofLogNotice() << "firmata v" << ard.getMajorFirmwareVersion() << "." << ard.getMinorFirmwareVersion();
        
    // Note: pins A0 - A5 can be used as digital input and output.
    // Refer to them as pins 14 - 19 if using StandardFirmata from Arduino 1.0.
    // If using Arduino 0022 or older, then use 16 - 21.
    // Firmata pin numbering changed in version 2.3 (which is included in Arduino 1.0)
    
    // set pin A0 to analog input
    ard.sendAnalogPinReporting(sensorAnalogPin, ARD_ANALOG);

	// set pin D13 as digital output
	ard.sendDigitalPinMode(13, ARD_OUTPUT);
	
    // Listen for changes on the digital and analog pins
	// Not sure how to make listener work with multiple arduinos
    //ofAddListener(ard.EAnalogPinChanged, this, &testApp::analogPinChanged);    
}

// analog pin event handler, called whenever an analog pin value has changed
//--------------------------------------------------------------
void testApp::analogPinChanged(const int & pinNum) {
	cout << "analogPinChanged: " << ofToString(pinNum) << endl;
    // do something with the analog input. here we're simply going to print the pin number and
    // value to the screen each time it changes
    //potValue = "analog pin: " + ofToString(pinNum) + " = " + ofToString(ard.getAnalog(pinNum));
}

//--------------------------------------------------------------
void testApp::update(){
	//cout << "update()" << endl;
	for (int i=0; i<nSensors; i++) {
		arduinos.at(i).update();
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	// Update smoothing weights
	if (nSamplesToSmooth < maxSamplesToSmooth ) nSamplesToSmooth++; // increment up to max
	float newWeight = 1/(nSamplesToSmooth); // weight by which new values contribute to the smoothed average

	// Loop through sensors, read data, filter, map and send outputs
	for (int i=0; i<nSensors; i++) {
		int sensorData = 0;
		if (bSetupArduinos.at(i)) {
			sensorData = arduinos.at(i).getAnalog(sensorAnalogPin); // get sensor data
		}
		float newVal = (float) sensorData; // convert to float
		// Smoothing occurs every time a new data point is obtained //
		smoothData.at(i) = newVal*newWeight + smoothData.at(i)*(1-newWeight); // smooth a running average on the incoming data
		sensorData = (int) smoothData.at(i); // convert back to int

		// Map sensor data to output range
		float dist2Value = maxOuputDist/maxSensorDist*maxAnalogValue;		// distance to analog value ratio
		int outputData = ofClamp(sensorData, 0, dist2Value);				// clamp to maxOuputDist
		outputData = ofMap(outputData, 0, dist2Value, 0, maxOutputValue);	// map onto output range

		// Set screen color to reflect incoming data
		
		//ofBackground(255-(2*outputData), 0, 0);
		ofSetColor(255-(2*outputData), 0, 0);
		ofFill();
		ofRect(ofGetWindowWidth()/nSensors*i, 0, ofGetWindowWidth()/nSensors, ofGetWindowHeight());
		
		// If we're not in midiMapMode, send MIDI outputs
		if (!midiMapMode) { 
			midiout.sendControlChange(midiChannel, midiIds.at(i), 127-outputData);
			cout << ", " << ofToString(127-outputData);

			if (++soundCheck > 1024) {
				soundCheck = 0;
			}
			//midiout.sendControlChange(midiChannel, midiIds.at(i), soundCheck/8);
		}
	}
	if (!midiMapMode) { 
		cout << endl;
	}

	// Periodically resend midi note
	if (resendNote && !midiMapMode) {
		if (++noteResendCounter > noteResendTime) {
			noteResendCounter = 0;
			cout << "Resend Midi Notes: ";
			for (int i=0; i<nSensors; i++) {
				cout << ofToString(i) << "," << midiIds.at(i) << "; ";
				midiout.sendNoteOff(midiChannel, midiIds.at(i), midiValue );
				midiout.sendNoteOn(midiChannel, midiIds.at(i), midiValue );
			}
			cout << endl;
		}
	}

	// heartbeat blink digital channel 13 to verify arduino is working
	if (++blinkCounter > ofGetFrameRate()/2) { // 1Hz
		blinkCounter = 0;
		blink13On = !blink13On;
		for (int i=0; i<nSensors; i++) {
			if (bSetupArduinos.at(i)) {
				if (blink13On) {
					arduinos.at(i).sendDigital(13, ARD_HIGH);
				} else {
					arduinos.at(i).sendDigital(13, ARD_LOW);
				}
			}
		}
	}
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	if (((char) key) == '=') {
		midiMapMode = !midiMapMode;
		printf("=");
	}

	// test MIDI control change commands
	char controlKeys[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};
	for (int i=0; i<nSensors; i++) {
		if (((char) key) ==  controlKeys[i]) {
			midiout.sendControlChange(midiChannel, midiIds.at(i), midiValue);
			cout << ofToString( controlKeys[i] );
		}
	} 

	// test MIDI send note on commands
	char noteOnKeys[8] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i'};
	for (int i=0; i<nSensors; i++) {
		if (((char) key) ==  noteOnKeys[i]) {
			midiout.sendNoteOn(midiChannel, midiIds.at(i), midiValue);
			cout << ofToString( noteOnKeys[i] );
		}
	} 

	// test MIDI send note off commands
	char noteOffKeys[8] = {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k'};
	for (int i=0; i<nSensors; i++) {
		if (((char) key) ==  noteOffKeys[i]) {
			midiout.sendNoteOff(midiChannel, midiIds.at(i), midiValue);
			cout << ofToString( noteOffKeys[i] );
		}
	} 

	if ( key == 'm') {
		ofToggleFullscreen();
	}

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void testApp::exit(){
	printf("exit()\n");

	for (int i=0; i<nSensors; i++) {
		midiout.sendNoteOff(midiChannel, midiIds.at(i), midiValue );
		if (bSetupArduinos.at(i)) {
			arduinos.at(i).disconnect();
		}
	}
	if (midiout.isOpen()) {
		midiout.closePort();
	}

}