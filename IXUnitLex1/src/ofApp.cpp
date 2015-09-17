#include <stdlib.h>
#include <cstdio>
#include "ofApp.h"
#include "LivestreamNetwork.h"

//--------------------------------------------------------------
void ofApp::setup(){
	slaveMode = false;  // Slave mode follows UDP commands to run
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	// debugTest used to isolate parts of code
	debugTest = NO_LIDAR;
	
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
	
	if (debugTest != NO_GPIO) {
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
	}
	
	if (debugTest != NO_SOUND) {
		// Sound output setup
		volSound.loadSound("sounds/Tp10.wav");
		volume = 0.0f;
		volSound.setVolume(volume);
		volSound.setMultiPlay(true);
		ofSoundSetVolume(volume);
	}

	// Init LidarLite
	int lidarConfig = 0; // 2=low sensitivity, low noise
	if (debugTest != NO_LIDAR) {
		myLidarLite = LidarLite();
		myLidarLite.logLevel = LidarLite::INFO;
		myLidarLite.begin(0);
		myLidarLite.configure(lidarConfig);
		// Exit if the lidar lite didn't initialize properly
		if (!myLidarLite.hasBegun()) ofApp::exit();
		
		// Print the hardware version of the Lidar Lite
		cout << "LIDAR Lite hardware version: " << myLidarLite.hardwareVersion() << endl;
		cout << "LIDAR Lite software version: " << myLidarLite.softwareVersion() << endl;
	}
	
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
	loopInterval = 5;
	

	
	// Write initialization to the log
	string logFileName = "/logs/livestream/livestream_" + hostname + ".log";
	ofstream mFile;
	mFile.open(logFileName.c_str(), ios::out | ios::app);
	mFile << getDateTimeString() << ",INITILIZATION";
	if (debugTest != NO_LIDAR) {
		mFile	<< ", LH, " << myLidarLite.hardwareVersion();
		mFile	<< " , LV, " << myLidarLite.softwareVersion();
		mFile	<< ", LC, " << lidarConfig;
	}
	mFile << ", TN, " << nSensors; 
	mFile << endl;
	mFile.close();
	
	cout << getDateTimeString() << ",INITILIZATION " << hostname;
	if (debugTest != NO_LIDAR) {
		cout << " , LH, " << myLidarLite.hardwareVersion(); 
		cout << " , LV, " << myLidarLite.softwareVersion();
		cout << ", LC, " << lidarConfig;
	}
	cout << ", TN, " << nSensors;
	cout << endl;
}

//--------------------------------------------------------------
void ofApp::update(){
	// update the sound playing system:
	if (debugTest != NO_SOUND) {
		ofSoundUpdate();
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofSleepMillis(loopInterval);
	
	// Check incoming UDP messages
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
		
		if(memcmp( header->typeTag, LivestreamNetwork::SET_MAESTRO_ADDRESS, 
			sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
			// ********** SET_MAESTRO_ADDRESS packet type ********** //
			// Get the data
			LivestreamNetwork::PacketIPAddress_V1* inPacket = (LivestreamNetwork::PacketIPAddress_V1 *) &udpMessage;
			// Set the maestroIpAddress
			maestroIpAddress = inPacket->ipAddress;
		} else {
			if (address.compare(maestroIpAddress) == 0) {
				// Only look at messages from maestroIpAddress
				
				if (!slaveMode) {
					// If we're in MODE_MASTER, only listen to MODE_SLAVE messages
					if(memcmp( header->typeTag, LivestreamNetwork::MODE_SLAVE, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** MODE_SLAVE packet type ********** //
						slaveMode = true;
					}
				} else {
					// We're in MODE_SLAVE, Listen up!!
					if(memcmp( header->typeTag, LivestreamNetwork::MODE_MASTER, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** MODE_MASTER packet type ********** //
						slaveMode = false;
					} else if (memcmp( header->typeTag, LivestreamNetwork::GET_DISTANCE, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** GET_DISTANCE packet type ********** //
						
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
						udpSender.Send((char*) &outPacket, sizeof(outPacket));
						// Convert the typeTage char[2] to a string for logging
						typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / 
							sizeof(outPacket.hdr.typeTag[0]));
						ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << maestroIpAddress << endl;		
						
					} else if(memcmp( header->typeTag, LivestreamNetwork::PING, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** PING packet type ********** //
						
						// Load the packet data
						LivestreamNetwork::PacketNoPayload_V1 outPacket;
						outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
						outPacket.hdr.packetCount = ++nPacketsSent;
						outPacket.hdr.protocolVersion = packetProtocolVersion;
						strncpy(outPacket.hdr.typeTag, LivestreamNetwork::PONG, 
							sizeof LivestreamNetwork::PONG / sizeof LivestreamNetwork::PONG[0]);
							
						// Send the packet
						udpSender.Send((char*) &outPacket, sizeof(outPacket));
						// Convert the typeTage char[2] to a string for logging
						typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / 
							sizeof(outPacket.hdr.typeTag[0]));
						ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << maestroIpAddress << endl;		
					} else if(memcmp( header->typeTag, LivestreamNetwork::SET_VOLUME, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** SET_VOLUME packet type ********** //
						// Get the Volume data
						LivestreamNetwork::PacketUInt8_V1* inPacket = (LivestreamNetwork::PacketUInt8_V1 *) &udpMessage;
						// Set the Volume
						volume = (float) inPacket->u / 255.f;
						volSound.setVolume(volume);
						ofSoundSetVolume(volume);
						ofLog(OF_LOG_VERBOSE) << "SV, " << (int) inPacket->u << endl;			
						
					} else if(memcmp( header->typeTag, LivestreamNetwork::GET_ALL_TEMPS, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** GET_ALL_TEMPS packet type ********** //
						
						// Get the CPU temperature
						FILE *temperatureFile;
						double T;
						temperatureFile = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
						if (temperatureFile == NULL) {
							cout << "Failed to read temp file\n";
						} else {
							fscanf (temperatureFile, "%lf", &T);
							T /= 1000;
							fclose (temperatureFile);
						}
						
						// Load the packet data
						LivestreamNetwork::PacketTemperature_V1 outPacket;
						outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
						outPacket.hdr.packetCount = ++nPacketsSent;
						outPacket.hdr.protocolVersion = packetProtocolVersion;
						strncpy(outPacket.hdr.typeTag, LivestreamNetwork::TEMPERATURE, 
							sizeof LivestreamNetwork::TEMPERATURE / sizeof LivestreamNetwork::TEMPERATURE[0]);
							
						// Send the CPU temperature
						outPacket.temperature = (int) T;
						outPacket.sensorDesignator = 'C';
						udpSender.Send((char*) &outPacket, sizeof(outPacket));
						
						// Send data from the other temperature sensors
						for (int j=0; j<nSensors; j++) {
							// Send the temperature
							outPacket.temperature = (int) tempSensor.read(j);
							char sensorDesignator[5];
							//itoa( j, sensorDesignator, 10 );
							sprintf(sensorDesignator, "%i", j);
							outPacket.sensorDesignator = sensorDesignator[0];
							udpSender.Send((char*) &outPacket, sizeof(outPacket));
						}
					
					} else if(memcmp( header->typeTag, LivestreamNetwork::SET_LED, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** SET_LED packet type ********** //
						
						// Get the LED data
						LivestreamNetwork::PacketBool_V1* inPacket = (LivestreamNetwork::PacketBool_V1 *) &udpMessage;
						// Set the LED outputs
						if (inPacket->b) {
							// Blink the LED
							gpio15->setval_gpio("1");
							gpio15outState = true;
							gpio21->setval_gpio("1");
							gpio21outState = true;
						} else {							
							gpio15->setval_gpio("0");
							gpio15outState = false;
							gpio21->setval_gpio("0");
							gpio21outState = false;
						}
					} else if(memcmp( header->typeTag, LivestreamNetwork::PLAY_NOTE, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** PLAY_NOTE packet type ********** //
						volSound.play();
						volSound.setVolume(volume);
						ofSoundSetVolume(volume);
					}
				}
			} //address.compare(maestroIpAddress) == 0
		} // !SET_MAESTRO_ADDRESS
	} // message!=""
	if (!slaveMode) {
		// **** Master mode **** //
		int rawDist;
		int signalStrength;
		if (debugTest == NO_LIDAR) {
			// Isolate the LidarLite measurements for debug
			rawDist = 101;
			signalStrength = 101;
		} else {
			rawDist = myLidarLite.distance();
			signalStrength = myLidarLite.signalStrength();
		}
		int newVal = rawDist;
		//int maxNoise = myLidarLite.maxNoise();
		//int corrPeakVal = myLidarLite.correlationPeakValue();

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
		
		float soundVolume;
		if (debugTest != NO_SOUND) {
			soundVolume = 1.f - ofMap(smoothPwm, minDist, maxDist, 0.f, 1.f, true);
		} else {
			soundVolume = .101f;
		}
		
		if (ofGetSystemTimeMicros() - blinkTimer >= 1000000) {
			if (debugTest != NO_SOUND) {
				volSound.play();
			}
			
			if (debugTest != NO_GPIO) {
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
			}
		
			cout << getDateTimeString() << setprecision(3) << ", LR, " << ofGetFrameRate();
			cout << ", LD, " << rawDist << ", LW, " << (int) smoothPwm;
			cout << ", AV, " << soundVolume << ", LS, " << signalStrength; 
			//cout << ", LN, " << maxNoise << ", LP, " << corrPeakVal;
			cout << ", HS, " << gpio15outState;
			cout << endl;
				
			blinkTimer = ofGetSystemTimeMicros();
		}
		if (debugTest != NO_SOUND) {
			volSound.setVolume(soundVolume);
			ofSoundSetVolume(soundVolume);
		}
		
			// Log data to file
		if (ofGetElapsedTimeMillis() - logTimer >= 60000) {
			logTimer = ofGetElapsedTimeMillis();
			
			double T;
			if (debugTest == NO_TEMP) {
				// Isolate the temperature measurements from the code for debug
				T = 101;
			} else {
				// Get the temperature
				FILE *temperatureFile;
				temperatureFile = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
				if (temperatureFile == NULL) {
					cout << "Failed to read temp file\n";
				} else {
					fscanf (temperatureFile, "%lf", &T);
					T /= 1000;
					//printf ("The temperature is %6.3f C.\n", T);
					fclose (temperatureFile);
				}
			}
			
			// Log the data
			string logFileName = "/logs/livestream/livestream_" + hostname + ".log";
			ofstream mFile;
			mFile.open(logFileName.c_str(), ios::out | ios::app);
			mFile << getDateTimeString() << setprecision(3) << ", TC, " << T;
			for (int j=0; j<nSensors; j++) {
				float tempData;
				if (debugTest == NO_TEMP) {
					// Isolate the temperature measurements from the code for debug
					tempData = 101;
				} else {
					tempData = tempSensor.read(j);
				}
				mFile << ", T" << j << ", " << tempData;
			}
			mFile << ", LD, " << rawDist  << ", LW, " << (int) smoothPwm << ", LR, " << ofGetFrameRate();
			mFile << ", LS, " << signalStrength << ", LN, "; 
			//<< maxNoise << ", LP, " << corrPeakVal;
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
	volSound.unloadSound();
	ofSoundShutdown();
	if (debugTest != NO_GPIO) {
		gpio15->setval_gpio("0");
		gpio21->setval_gpio("0");
	}
}