#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	cout << "setup()" << endl;

	// **** Setup sensor conversion variables **** //
	maxSensorDist = settings.maxSensorDist;	// total range of sensor (ft)
	maxOuputDist = settings.maxOuputDist;	// target output range of sensor (ft)
	maxAnalogValue = 1024;		// ADC bit resolution
	maxMidiControlValue = 127;	// maximum value of output

	// **** Setup frame rate **** //
	ofSetVerticalSync(true);
	Fs = 4; // sampling freq (Hz)
	ofSetFrameRate(Fs);

	// **** Setup timers **** //
	blink13On = true;
	blinkCounter = 0;

	//pipes.resize(settings.nSensors); 

	// **** Setup Arduino **** //
	for (int i=0; i<settings.nSensors; i++) {
		settings.pipes.at(i).isArduinoSetup = false;				// it's not safe to send commands to Arduino
	}
	int arduinoBaud = 57600; // baud rate
	sensorAnalogPin = 0;
	if (settings.usingDistanceSensors) {
		for (int i=0; i<settings.nSensors; i++) {
			cout << "Setting up " << settings.pipes.at(i).arduinoPort << endl;

			settings.pipes.at(i).arduino.connect(settings.pipes.at(i).arduinoPort, arduinoBaud);	// connect to the Arduino
			while(!settings.pipes.at(i).arduino.isArduinoReady());	// Wait till the Arduino is ready
			setupArduino(settings.pipes.at(i).arduino);				// Setup Arduino
			settings.pipes.at(i).isArduinoSetup = true;				// it is now safe to send commands to the Arduino
		}
	}

	// **** Setup midi **** //
	// Midi Port
	midiPort = settings.midiPort;
	midiout.listPorts();
	midiout.openPort(midiPort);
	midiMapMode = false;

	// MIDI parameters
	midiDistControl = 0;

	// MIDI Test Controls (for pairing with Ableton)
	testMidiId = 0;
	testMidiChannel = 0;
	testMidiIdSlider.addListener(this, &testApp::testMidiIdSliderChanged);
	testMidiChannelSlider.addListener(this, &testApp::testMidiChannelSliderChanged);
	gui.setup();
	gui.add(testMidiChannelSlider.setup("Test MIDI Channel", 1, 1, 16));
	gui.add(testMidiIdSlider.setup("Test MIDI ID", 0, 0, 127));
	// TODO: deal with crossChannelSustain that's too big
	crossChannelSustainCounter = 0;

	soundCheck = settings.soundCheck;	// for debugging sound

	// **** Filter Setup **** //
	smoothData.resize(settings.nSensors);
	float smoothDuration = 1.5; // seconds
	maxSamplesToSmooth = Fs*smoothDuration; // determines degree of smoothing
	nSamplesToSmooth = 0; // starting with 0 and incrementing up to max converges smoothed average to initial values quickly

	//dataSendHelper = 0;
	for (int i=1; i<5; i++) {
		midiout.sendControlChange(i, 64, 127); // trying to get get a "sustain pedal" control signal
	}
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

	// set pin D7 as digital output 
	ard.sendDigitalPinMode(7, ARD_OUTPUT);

	if (settings.triggeredSensorPolling) {
		// Trigger the sensor so that data is ready on first update loop
		ard.sendDigital(7, ARD_LOW);
		ard.update();
		ofSleepMillis(200);
		ard.sendDigital(7, ARD_HIGH); // Trigger the sensor to read new data
		ard.update();
		ofSleepMillis(1); // Sleep >20uSec to trigger sensor to make a reading
		ard.sendDigital(7, ARD_LOW); // Pull low to stay in real-time/trigger mode
		ard.update();
		ofSleepMillis(200);
	} else {
		ard.sendDigital(7, ARD_HIGH); // set high to enable automatic range sampling on sensor
	}
	
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
void testApp::testMidiIdSliderChanged(int & tempMidiId){
	testMidiId = tempMidiId;
}

//--------------------------------------------------------------
void testApp::testMidiChannelSliderChanged(int & tempMidiChannel){
	testMidiChannel = tempMidiChannel;
}

//--------------------------------------------------------------
void testApp::update(){
	cout << "LoopTime= " << ofGetElapsedTimeMillis() - elapsedTime << ", ";
	elapsedTime = ofGetElapsedTimeMillis();

	// Update smoothing weights
	if (nSamplesToSmooth < maxSamplesToSmooth ) nSamplesToSmooth++; // increment up to max
	float newWeight = 1/(nSamplesToSmooth); // weight by which new values contribute to the smoothed average

	//cout << "update()" << endl;
	for (int i=0; i<settings.nSensors; i++) {
		if (settings.pipes.at(i).isArduinoSetup) {
			// Read new data point from the arduino analog pin
			float newVal = (float) settings.pipes.at(i).arduino.getAnalog(sensorAnalogPin); // get sensor data

			// Smoothing occurs every time a new data point is obtained //
			smoothData.at(i) = newVal*newWeight + smoothData.at(i)*(1-newWeight); // smooth a running average on the incoming data

			if (settings.triggeredSensorPolling) {
				// Trigger the sensor to poll a new datapoint
				settings.pipes.at(i).arduino.sendDigital(7, ARD_HIGH);
				settings.pipes.at(i).arduino.update();
				ofSleepMillis(2); // Sleep >20uSec to trigger sensor to make a reading
				settings.pipes.at(i).arduino.sendDigital(7, ARD_LOW); // Pull low to stay in real-time/trigger mode
				ofSleepMillis(2); // Wait between sensor triggering (dunno if it might help prevent cross talk)
			}

			settings.pipes.at(i).arduino.update();
		}
	}

	// Update data read timer
	for (int i=0; i<settings.nSensors; i++) {
		settings.pipes.at(i).dataReadCounter++;
		if (settings.pipes.at(i).dataReadCounter >= (settings.pipes.at(i).dataReadRate * Fs) / 1000 ) {
			settings.pipes.at(i).dataReadCounter = 0;
		
			settings.pipes.at(i).updateData();
		}
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	cout << "distance= ";
	// Loop through sensors, read data, filter, map and send output controls
	for (int i=0; i<settings.nSensors; i++) {
		// Map sensor data to output range
		float dist2Value = maxOuputDist/maxSensorDist*maxAnalogValue;		// distance to analog value ratio
		int controlValue = ofClamp((int) smoothData.at(i), 0, dist2Value);				// clamp to maxOuputDist
		controlValue = ofMap(controlValue, 0, dist2Value, 0, maxMidiControlValue);	// map onto output range

		// Set screen color to reflect incoming data
		ofSetColor(255-(2*controlValue), 0, 0);
		ofFill();
		ofRect(ofGetWindowWidth()/settings.nSensors*i, 0, ofGetWindowWidth()/settings.nSensors, ofGetWindowHeight());
		
		// If we're not in midiMapMode, send MIDI outputs
		if (!midiMapMode) { 
			// hack to deal with same note MIDI sustain in Live
			//int outChannel = crossChannelSustainCounter * settings.pipes.size() + settings.pipes.at(i).midiChannel; 
			midiout.sendControlChange(settings.pipes.at(i).midiChannel, midiDistControl, 127-controlValue);
			cout << ofToString(127-controlValue) << ", ";

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
	//if (resendNote && !midiMapMode) {
	//	if (++noteResendCounter > noteResendTime) {
	//		noteResendCounter = 0;
	//		cout << "Resend Midi Notes: ";
	//		for (int i=0; i<settings.nSensors; i++) {
	//			cout << ofToString(i) << "," << midiIds.at(i) << "; ";
	//			midiout.sendNoteOff(midiChannel, midiIds.at(i), midiValue );
	//			midiout.sendNoteOn(midiChannel, midiIds.at(i), midiValue );
	//		}
	//		cout << endl;
	//	}
	//}

	// Increment MIDI send timer
	if (!midiMapMode) { 
		for (int i=0; i<settings.nSensors; i++) {		
			settings.pipes.at(i).dataMidiSendCounter++;
			if (settings.pipes.at(i).dataMidiSendCounter >= (settings.pipes.at(i).dataMidiSendRate * Fs) / 1000) {
				// The send counter has tripped... send a note!
				settings.pipes.at(i).dataMidiSendCounter = 0; // reset the counter
				float tempData = settings.pipes.at(i).getData(); // get a new datapoint
				cout << settings.pipes.at(i).dataName << "= " << ofToString(tempData) << endl;
				// Map the data onto the ouput range
				int tempPitch = ofMap(tempData, 
					settings.pipes.at(i).dataRange.min, 
					settings.pipes.at(i).dataRange.max,
					settings.pipes.at(i).midiNoteRange.min,
					settings.pipes.at(i).midiNoteRange.max);
				//dataSendHelper++;
				//if (dataSendHelper > 10) dataSendHelper = 0;
				// hack to deal with same note MIDI sustain in Live
				int outChannel = settings.pipes.at(i).midiChannel;
				if (settings.pipes.at(i).crossChannelSustainCounter > 0) {
					outChannel = outChannel + settings.crossChannelSustainGap;
				}
				settings.pipes.at(i).crossChannelSustainCounter = (settings.pipes.at(i).crossChannelSustainCounter + 1) % 2;
				
				midiout.sendNoteOn(outChannel, tempPitch, settings.midiNoteAttack);
			}
			
		}
	}

		//dataSendHelper = 0;
	for (int i=1; i<5; i++) {
		//midiout.sendControlChange(i, 64, 127); // trying to get get a "sustain pedal" control signal
		//midiout.sendControlChange(i, 66, 127); // trying to get get a "sustain pedal" control signal
		//midiout.sendControlChange(i, 69, 127); // trying to get get a "sustain pedal" control signa6
	}

	// heartbeat blink digital channel 13 to verify arduino is working
	if (++blinkCounter > Fs/2) { // 1Hz
		blinkCounter = 0;
		blink13On = !blink13On;
		for (int i=0; i<settings.nSensors; i++) {
			if (settings.pipes.at(i).isArduinoSetup) {
				if (blink13On) {
					settings.pipes.at(i).arduino.sendDigital(13, ARD_HIGH);
				} else {
					settings.pipes.at(i).arduino.sendDigital(13, ARD_LOW);
				}
			}
		}
	}

	gui.draw();
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
	if (((char) key) ==  '1') {
		midiout.sendControlChange(testMidiChannel, testMidiId, midiValue);
		cout << ofToString( '1' );
	}

	// test MIDI send note on commands
	if (((char) key) ==  'q') {
		midiout.sendNoteOn(testMidiChannel, testMidiId, midiValue);
		cout << ofToString( 'q' );
	}

	// test MIDI send note off commands
	if (((char) key) ==  'a') {
		midiout.sendNoteOff(testMidiChannel, testMidiId, midiValue);
		cout << ofToString( 'a' );
	}
	
	// test MIDI program change commands
	if (((char) key) ==  'z') {
		midiout.sendProgramChange(midiChannel, testMidiId);
		cout << ofToString( 'z' );
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

	for (int i=0; i<settings.nSensors; i++) {
		//midiout.sendNoteOff(midiChannel, midiIds.at(i), midiValue );
		if (settings.pipes.at(i).isArduinoSetup) {
			settings.pipes.at(i).arduino.disconnect();
		}
	}
	if (midiout.isOpen()) {
		midiout.closePort();
	}

}