#include "ofApp.h"
#include "LivestreamNetwork.h"

//--------------------------------------------------------------
void ofApp::setup(){
	slaveMode = true;  // Slave mode follows UDP commands to run
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	//***********************************************************
	// Variables for "Master" Mode (not requiring UDP input)
	//***********************************************************
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
	
	//***********************************************************
	// Variables for "Slave" Mode (relying on UDP input)
	//***********************************************************
	
	//create the receiver socket and bind to port 11999
	udpReceiver.Create();
	udpReceiver.Bind(11999);
	udpReceiver.SetNonBlocking(true);

	// create the receiver socket and bind to the maestro address 11999
	maestroIpAddress = "10.0.0.3";
	udpSender.Create();
	udpSender.Connect(maestroIpAddress.c_str(),11999);
	udpSender.SetNonBlocking(true);
	
	nPacketsSent = 0;
	packetProtocolVersion = 1;
	
	//***********************************************************
	// General Setup
	//***********************************************************
	ofSetVerticalSync(false);
	//ofSetFrameRate(10000)
	
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
	
	// Sound output setup
	volSound.loadSound("sounds/Tp10.wav");
	volume = 0.0f;
	volSound.setVolume(volume);
	volSound.setMultiPlay(true);
	ofSoundSetVolume(volume);

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
		cout << "host name = " 	<< hostname << endl;
	} else {
		cout << "host name unknown" << endl;
		hostname = "unknown";
	}
	
	// Get the number of temperature sensors
	nSensors = tempSensor.listDevices();
	
	// Set the delay between loop cycles
	loopInterval = 25;
	
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
void ofApp::draw() {
	ofSleepMillis(loopInterval);
	
	if (slaveMode) {
		if (ofGetSystemTimeMicros() - blinkTimer >= 1000000) {
				volSound.play();
				volSound.setVolume(volume);
				ofSoundSetVolume(volume);
				blinkTimer = ofGetSystemTimeMicros();
		}
		
		char udpMessage[100000];
		udpReceiver.Receive(udpMessage,100000);
		string message=udpMessage;
		
		if(message!=""){
			// We got a message!
			char udpAddress[20];
			// Get the sender's address
			udpReceiver.GetRemoteAddr(udpAddress);
			string address = udpAddress;
			
			// Convert the UDP message to read its type tag from the header section
			LivestreamNetwork::PacketHeader_V1* header = 
				(LivestreamNetwork::PacketHeader_V1 *) &udpMessage;
			// Convert the typeTage char[2] to a string for logging
			string typeTag(header->typeTag, header->typeTag + sizeof header->typeTag / sizeof header->typeTag[0]);
			ofLog(OF_LOG_VERBOSE) << typeTag << "<<" << address << endl;			
			
			if (memcmp( header->typeTag, LivestreamNetwork::SET_MAESTRO_ADDRESS, 
				sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
				// SET_MAESTRO_ADDRESS packet type
				// TODO: Reconnect the udpSender to the new address
			} else {
				if (address.compare(maestroIpAddress) == 0) {
					// Only look at messages from maestroIpAddress
					
					if (memcmp( header->typeTag, LivestreamNetwork::GET_DISTANCE, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// **** GET_DISTANCE packet type **** //
						
						// Read data from the LidarLite
						int rawDist = myLidarLite.distance();
						int signalStrength = myLidarLite.signalStrength();
						
						// Load the packet data
						LivestreamNetwork::PacketDistance_V1 outPacket;
						outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
						outPacket.hdr.packetCount = ++nPacketsSent;
						outPacket.hdr.protocolVersion = packetProtocolVersion;
						strncpy(outPacket.hdr.typeTag, LivestreamNetwork::DISTANCE, 
							sizeof LivestreamNetwork::DISTANCE / sizeof LivestreamNetwork::DISTANCE[0]);
						outPacket.distance = rawDist;
						outPacket.signalStrength = signalStrength;
							
						// Send the packet
						udpSender.Send((char*) &outPacket, sizeof(LivestreamNetwork::PacketDistance_V1));
						// Convert the typeTage char[2] to a string for logging
						typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / 
							sizeof(outPacket.hdr.typeTag[0]));
						ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << maestroIpAddress << endl;		
						
					} else if(memcmp( header->typeTag, LivestreamNetwork::PING, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// **** PING packet type **** //
						
						// Load the packet data
						LivestreamNetwork::PacketNoPayload_V1 outPacket;
						outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
						outPacket.hdr.packetCount = ++nPacketsSent;
						outPacket.hdr.protocolVersion = packetProtocolVersion;
						strncpy(outPacket.hdr.typeTag, LivestreamNetwork::PONG, 
							sizeof LivestreamNetwork::PONG / sizeof LivestreamNetwork::PONG[0]);
							
						// Send the packet
						udpSender.Send((char*) &outPacket, sizeof(LivestreamNetwork::PacketNoPayload_V1));
						// Convert the typeTage char[2] to a string for logging
						typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / 
							sizeof(outPacket.hdr.typeTag[0]));
						ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << maestroIpAddress << endl;		
					} else if(memcmp( header->typeTag, LivestreamNetwork::SET_VOLUME, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// **** SET_VOLUME packet type **** //
						
						// Get the Volume data
						LivestreamNetwork::PacketUInt8_V1* inPacket = (LivestreamNetwork::PacketUInt8_V1 *) &udpMessage;
						// Set the Volume
						volume = (float) inPacket->u / 255.f;
						volSound.setVolume(volume);
						ofSoundSetVolume(volume);
						ofLog(OF_LOG_VERBOSE) << "SV, " << (int) inPacket->u << endl;			
					}
				} //address.compare(maestroIpAddress) == 0
			} // !SET_MAESTRO_ADDRESS
		} // message!=""
	} else { 
		// Master mode
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