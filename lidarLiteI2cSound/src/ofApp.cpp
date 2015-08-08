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
	
	// Sound output
	pitchSound.loadSound("sounds/INT18PIPES1.mp3");
	pitchSound.setVolume(0.3f);
	pitchSound.setMultiPlay(false);
	
	volSound.loadSound("sounds/Tp10.wav");
	volSound.setVolume(0.3f);
	volSound.setMultiPlay(true);
	
	ofSoundSetVolume(.5f);

	volBend = true;
	pitchBend = false;
	
	// Init I2C
	fd = lidar_init(false);
	if (fd == -1) 
	{
		printf("initialization error\n");
		ofApp::exit();
	}
	
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
	
	// Write initialization to the log
	string logFileName = "/logs/livestream/livestream_" + hostname + ".log";
	ofstream mFile;
	mFile.open(logFileName.c_str(), ios::out | ios::app);
	mFile << getDateTimeString() << ",INITILIZATION" << endl;
	mFile.close();
	
	nSensors = tempSensor.listDevices();
}

//--------------------------------------------------------------
void ofApp::update(){
	// update the sound playing system:
	ofSoundUpdate();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofSleepMillis(100);
	
	int newVal = lidar_read(fd);
	unsigned char  st = lidar_status(fd);
	//ver = lidar_version(fd);
	//printf("%3.0d cm \n", res);
	lidar_status_print(st);
	
	// increment up to max smoothing (deals with initial state)
	if (nSamplesToSmooth < maxSamplesToSmooth ) nSamplesToSmooth++; 
	// Calculate the weight of new value
	float newWeight = ((float)1)/((float)nSamplesToSmooth); 
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
			cout << getDateTimeString() << ", " << counter << " loops, " << ofGetFrameRate() 
				<< "Hz , " << smoothPwm << " cm" << " , " << soundSpeed <<", " << st;
			if (gpio15outState){
				cout << ",LED=ON";
			} else {
				cout << ",LED=OFF";
			}
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
			
			cout << getDateTimeString() << ", " << counter << " loops, " << ofGetFrameRate() 
				<< "Hz , " << smoothPwm << " cm" << " , " << soundVolume <<", " << st;
			if (gpio15outState){
				cout << ",LED=ON";
			} else {
				cout << ",LED=OFF";
			}
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
		mFile << getDateTimeString() << ",TC," << T;
		for (int j=0; j<nSensors; j++) {
			float tempData = tempSensor.read(j);
			mFile << ",T" << j << "," << tempData;
		}
		mFile << ",DL," << smoothPwm << " cm" << ",LR," << ofGetFrameRate() << "Hz,";
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