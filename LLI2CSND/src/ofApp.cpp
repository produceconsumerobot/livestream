#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(false);
	//ofSetFrameRate(10000)
	
	counter = 0;
	blinkTimer = ofGetSystemTimeMicros();
	logTimer = ofGetElapsedTimeMillis();
	
	// Setup GPIOs
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

	// PWM smoothing parameters
	nSamplesToSmooth = 0;
	maxSamplesToSmooth = 1; // determines smoothing
	smoothPwm = 0; // smoothed PWM value
	minDist = 30; //cm
	maxDist = 30*20; //cm
	noiseDist = 20;	//cm
	minSigStrength = 20;		// For creating weighted distance average
	fullSigStrength = 80;	// For creating weighted distance average
	minSigWeight = 0.05f;
	
	// Sound output
	pitchSound.loadSound("sounds/INT18PIPES1.mp3");
	pitchSound.setVolume(1.0f);
	pitchSound.setMultiPlay(false);
	
	volSound.loadSound("sounds/Tp10.wav");
	volSound.setVolume(1.0f);
	volSound.setMultiPlay(true);
	
	ofSoundSetVolume(1.0f);

	volBend = true;
	pitchBend = false;
	
	// Init LidarLite
	myLidarLite = LidarLite();
	myLidarLite.logLevel = LidarLite::INFO;
	myLidarLite.begin(0);
	int lidarConfig = 0; // 2=low sensitivity, low noise
	myLidarLite.configure(lidarConfig);
	// Exit if the lidar lite didn't initialize properly
	if (!myLidarLite.hasBegun()) ofApp::exit();
	// Print the hardware version of the Lidar Lite
	cout << "LIDAR Lite hardware version: " << myLidarLite.hardwareVersion() << endl;
	cout << "LIDAR Lite software version: " << myLidarLite.softwareVersion() << endl;
	
	// Get the host name
	int z;  
	char buf[32];  
	z = gethostname(buf,sizeof buf);
	if ( z != -1 ) {
		hostname = string(buf);
		//printf("host name = '%s'\n",buf); 
		cout << "host name = " 	<< hostname << endl;
	} else {
		cout << "host name unknown" << endl;
		hostname = "unknown";
	}
	
	nSensors = tempSensor.listDevices();
	
	// Write initialization to the log
	string logFileName = "/logs/livestream/livestream_" + hostname + ".log";
	ofstream mFile;
	mFile.open(logFileName.c_str(), ios::out | ios::app);
	mFile << getDateTimeString() << ",INITILIZATION, LH, " << myLidarLite.hardwareVersion() 
		<< " , LV, " << myLidarLite.softwareVersion()
		<< ", LC, " << lidarConfig << ", TN, " << nSensors << endl;
	mFile.close();
	
	cout << getDateTimeString() << ",INITILIZATION " << hostname << " , LH, " << myLidarLite.hardwareVersion() 
	  << " , LV, " << myLidarLite.softwareVersion()
		<< ", LC, " << lidarConfig << ", TN, " << nSensors << endl;
}

//--------------------------------------------------------------
void ofApp::update(){
	// update the sound playing system:
	ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofSleepMillis(25);
	
	int rawDist = myLidarLite.distance();
	int newVal = rawDist;
	int signalStrength = myLidarLite.signalStrength();
	int maxNoise = myLidarLite.maxNoise();
	int corrPeakVal = myLidarLite.correlationPeakValue();

	if (rawDist < noiseDist) {
		// Handle strange case where lidar reports 1cm when should be infinity
		newVal = maxDist*2;
	}
	
	// increment up to max smoothing (deals with initial state)
	if (nSamplesToSmooth < maxSamplesToSmooth ) nSamplesToSmooth++; 
	// Calculate the weight of new value
	float newWeight = ((float)1)/((float)nSamplesToSmooth); 
	if (rawDist < noiseDist) {
		// We're within the noise distance
		// Ignore signal strength and go toward newVal quickly
		newWeight = newWeight * (1.f - minSigWeight);  
	} else {
		// Weight by the signal strength
		newWeight = newWeight * ofMap(signalStrength, minSigStrength, fullSigStrength, minSigWeight, 1.f, true);
	}
	// Calculate the smoothed PWM value
	smoothPwm = ((float) newVal)*newWeight + smoothPwm*(1-newWeight);
	
	counter++;
	
	if (pitchBend) { // Pitch bend mode
		//cout << ofGetSystemTimeMicros() << "," << blinkTimer << endl;
		if (ofGetSystemTimeMicros() - blinkTimer >= 200000) {
			
			// Blink the LED
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
			
			// Play sound
			pitchSound.play();
			float soundSpeed = 1.5f - ofMap(smoothPwm, minDist, maxDist, 0.f, 1.f, true);
			pitchSound.setSpeed( soundSpeed );
			//ofSoundSetVolume(ofClamp(0.5f + smoothPwm, 0, 1));
			cout << getDateTimeString() << setprecision(3) << ", LR, " << ofGetFrameRate();
			cout << ", LD, " << rawDist << ", LW, " << (int) smoothPwm;
			cout << ", AV, " << soundSpeed << ", LS, " << signalStrength; 
			cout << ", LN, " << maxNoise << ", LP, " << corrPeakVal;
			cout << ", HS, " << gpio15outState;
			cout << endl;
			
			blinkTimer = ofGetSystemTimeMicros();
			counter = 0;
		}
	} else if (volBend) { // Volume bend mode
		float soundVolume = 1.f - ofMap(smoothPwm, minDist, maxDist, 0.f, 1.f, true);
		
		if (ofGetSystemTimeMicros() - blinkTimer >= 1000000) {
			volSound.play();
			
			// Blink the LED
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
		
			cout << getDateTimeString() << setprecision(3) << ", LR, " << ofGetFrameRate();
			cout << ", LD, " << rawDist << ", LW, " << (int) smoothPwm;
			cout << ", AV, " << soundVolume << ", LS, " << signalStrength; 
			cout << ", LN, " << maxNoise << ", LP, " << corrPeakVal;
			cout << ", HS, " << gpio15outState;
			cout << endl;
				
			blinkTimer = ofGetSystemTimeMicros();
			counter = 0;
		}
		volSound.setVolume(soundVolume);
		ofSoundSetVolume(soundVolume);
	}
	
	// Log data to file
	if (ofGetElapsedTimeMillis() - logTimer >= 60000) {

		
		// Get the temperature
		FILE *temperatureFile;
		double T;
		temperatureFile = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
		if (temperatureFile == NULL) {
			cout << "Failed to read temp file\n";
		} else {
			fscanf (temperatureFile, "%lf", &T);
			T /= 1000;
			//printf ("The temperature is %6.3f C.\n", T);
			fclose (temperatureFile);
		}
		
		// Log the data
		logTimer = ofGetElapsedTimeMillis();
		//string logFileName = "/logs/livestream/livestream01.log";
		string logFileName = "/logs/livestream/livestream_" + hostname + ".log";
		ofstream mFile;
		mFile.open(logFileName.c_str(), ios::out | ios::app);
		mFile << getDateTimeString() << setprecision(3) << ", TC, " << T;
		for (int j=0; j<nSensors; j++) {
			float tempData = tempSensor.read(j);
			mFile << ", T" << j << ", " << tempData;
		}
		mFile << ", LD, " << rawDist  << ", LW, " << (int) smoothPwm << ", LR, " << ofGetFrameRate();
		mFile << ", LS, " << signalStrength << ", LN, " << maxNoise << ", LP, " << corrPeakVal;
		mFile << endl;
		mFile.close();
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
	 if (key == 'v'){
		 cout << "Volume Bend Mode" << endl;
		 volBend = true;
		 pitchBend = false;
	 } else if (key == 'p'){
		 cout << "Pitch Bend Mode" << endl;
		 volBend = false;
		 pitchBend = true; 
		 //ofSoundSetVolume(.5f);
	 }
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

//--------------------------------------------------------------
void ofApp::exit(){ 
	cout << "EXIT" << endl;
	gpio15->setval_gpio("0");
	gpio21->setval_gpio("0");
}