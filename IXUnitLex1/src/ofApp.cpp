#include <stdlib.h>
#include <cstdio>
//#include <string>
#include "ofApp.h"
#include "LivestreamNetwork.h"

//--------------------------------------------------------------
void ofApp::setup(){
    softwareVersion = "1.1.1";
	slaveMode = true;  // Slave mode follows UDP commands to run
	ofSetLogLevel(OF_LOG_ERROR);
	
	// debugTest used to isolate parts of code
	debugTest = 0;
	
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

	// create the receiver socket and bind to the maestro ipAddress 11999
	maestroIpAddress = "192.168.254.0";
	udpSender.Create();
    udpSender.SetEnableBroadcast(false);
	udpSender.Connect(maestroIpAddress.c_str(),11999);
    //udpSender.Connect("192.168.254.255",11999);
	udpSender.SetNonBlocking(true);

    udpBroadcastAddress = "192.168.254.255";
    udpBroadcaster.Create();
    udpBroadcaster.SetEnableBroadcast(true);
	udpBroadcaster.Connect(udpBroadcastAddress.c_str(),11999);
	udpBroadcaster.SetNonBlocking(true);
	
	nPacketsSent = 0;
	packetProtocolVersion = 1;
	
	//***********************************************************
	// General Setup
	//***********************************************************
	ofSetVerticalSync(false);
	//ofSetFrameRate(10000)
	
	if (debugTest != NO_GPIO) {
		// Setup GPIOs
		//blinkLED = new GPIO("16");
        //blinkLED->setup("16");
		//blinkLED->export_gpio();
		//blinkLED->setdir_gpio("out");
		//blinkLED->setval_gpio("0");
        blinkLED = "16";
        int out;
        out = system(string("gpio mode " + blinkLED + " out").c_str());
        out = system(string("gpio write " + blinkLED + " 0").c_str());		
        blinkLEDoutState = false;
		
		//netLED = new GPIO("15");
		//netLED->export_gpio();
		//netLED->setdir_gpio("out");
		//netLED->setval_gpio("0");
        netLED = "15";
        out = system(string("gpio mode " + netLED + " out").c_str());
        out = system(string("gpio write " + netLED + " 0").c_str());
		netLEDoutState = false;
	}
	
    nSoundPlayers = 10;
    currentSoundPlayer = 0;
    soundPlayers.resize(nSoundPlayers);
	if (debugTest != NO_SOUND) {
      
		// Sound output setup
        // ToDo: change to data/tp10.wav
        //volSound.loadSound("/livestream/audio/Tp10.wav");
		soundPlayers.at(currentSoundPlayer).loadSound("sounds/Tp10.wav");
		volume = 0.0f;
		soundPlayers.at(currentSoundPlayer).setVolume(volume);
		soundPlayers.at(currentSoundPlayer).setMultiPlay(true);
		ofSoundSetVolume(volume);
	}

	// Init LidarLite
	int lidarConfig = 0; // 2=low sensitivity, low noise
	if (debugTest != NO_LIDAR) {
		//myLidarLite = ThreadedLidarLite();
		myLidarLite.logLevel = ThreadedLidarLite::INFO;
		myLidarLite.begin(0);
		myLidarLite.configure(lidarConfig);
		// Exit if the lidar lite didn't initialize properly
		if (!myLidarLite.hasBegun()) ofApp::exit();
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
	//nSensors = tempSensor.listDevices();
	
	// Set the delay between loop cycles
	loopInterval = 5;
	

	// Write initialization to the log
    string logFileName = ofToDataPath("") + hostname + "_" + ofGetTimestampString("%m") + ".log";
    //string logFileName = "/logs/livestream/livestream_" + hostname + "_" + ofGetTimestampString("%m") + ".log";
    ostringstream  logStringStream;
    logStringStream << "SV," << ofGetTimestampString("%Y%m%d,%H%M%S,") << softwareVersion << endl;
    if (debugTest != NO_LIDAR) {
        logStringStream << "LH," << ofGetTimestampString("%Y%m%d,%H%M%S,") << myLidarLite.hardwareVersion() << endl;
        logStringStream << "LV," << ofGetTimestampString("%Y%m%d,%H%M%S,") << myLidarLite.softwareVersion() << endl;
        logStringStream << "LC," << ofGetTimestampString("%Y%m%d,%H%M%S,") << lidarConfig << endl;
	}
     
	ofstream mFile;
	mFile.open(logFileName.c_str(), ios::out | ios::app);
	mFile << logStringStream.str();
	mFile.close();
	
	cout << logStringStream.str();
            
    // Start the LidarLite thread
    myLidarLite.start();
    
    // Setup blink timer
    blinkInterval = 1000000;
    blinkTimer = ofGetSystemTimeMicros();
}

//--------------------------------------------------------------
void ofApp::update(){
	// update the sound playing system:
	if (debugTest != NO_SOUND) {
		//ofSoundUpdate();
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofSleepMillis(loopInterval);
    
    // Handle blinking the blinkLED
    if (ofGetSystemTimeMicros() - blinkTimer >= blinkInterval) {       
        if (debugTest != NO_GPIO) {
            // Blink the LED
            if (blinkLEDoutState) {
                int out = system(string("gpio write " + blinkLED + " 0").c_str());
                blinkLEDoutState = false;
            } else {
                int out = system(string("gpio write " + blinkLED + " 1").c_str());
                blinkLEDoutState = true;
            }
        }            
        blinkTimer = ofGetSystemTimeMicros();
    }
    
    if (slaveMode) {
        if (debugTest != NO_LIDAR) {
            if (myLidarLite.isOutputNew()) {
                // There's new distance data available!
                int rawDist = -1;
                int signalStrength = -1;
                
                // Read the distance and signalStrength
                bool success = myLidarLite.getOutput(rawDist, signalStrength);
                if (success) {
                    // Load the packet data
                    LivestreamNetwork::Packet_DISTANCE_V1 outPacket;
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
                    string typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / 
                        sizeof(outPacket.hdr.typeTag[0]));
                    ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << maestroIpAddress << ", " << rawDist << ", " << signalStrength << endl;
                }
            }
        }
    }
	
	// Check incoming UDP messages
	char udpMessage[100000];
	udpReceiver.Receive(udpMessage,100000);
	string message=udpMessage;
	
	if(message!=""){
		// We got a message!
		//char udpAddress[20];
		string ipAddress;
		int udpPort;
		// Get the sender's ipAddress
		//udpReceiver.GetRemoteAddr(udpAddress, &udpPort);
		udpReceiver.GetRemoteAddr(ipAddress, udpPort);
		//string ipAddress = udpAddress;
		
		// Convert the UDP message to read its type tag from the header section
		LivestreamNetwork::PacketHeader_V1* header = 
			(LivestreamNetwork::PacketHeader_V1 *) &udpMessage;
		// Convert the typeTage char[2] to a string for logging
		string typeTag(header->typeTag, header->typeTag + sizeof header->typeTag / sizeof header->typeTag[0]);
		ofLog(OF_LOG_VERBOSE) << typeTag << "<<" << ipAddress << endl;			
		
		if(memcmp( header->typeTag, LivestreamNetwork::SET_MAESTRO_ADDRESS, 
			sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
			// ********** SET_MAESTRO_ADDRESS packet type ********** //
			// Get the data
			LivestreamNetwork::Packet_SET_MAESTRO_ADDRESS_V1* inPacket = (LivestreamNetwork::Packet_SET_MAESTRO_ADDRESS_V1 *) &udpMessage;
			// Set the maestroIpAddress
			maestroIpAddress = inPacket->ipAddress;
            udpSender.Close();
            udpSender.Create();
            udpSender.SetEnableBroadcast(false);
            udpSender.Connect(maestroIpAddress.c_str(),11999);
            udpSender.SetNonBlocking(true);
		} else {
			if (ipAddress.compare(maestroIpAddress) == 0) {
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
						
                        // Initiate a distance read in another thread
                        int success = myLidarLite.startDistanceRead();
                        if (!success) {
                            cout << "FAIL: myLidarLite.startRead();" << endl;
                        } 						
					} else if(memcmp( header->typeTag, LivestreamNetwork::PING, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** PING packet type ********** //
						
						// Load the packet data
						LivestreamNetwork::Packet_PONG_V1 outPacket;
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
						LivestreamNetwork::Packet_SET_VOLUME_V1* inPacket = (LivestreamNetwork::Packet_SET_VOLUME_V1 *) &udpMessage;
						// Set the Volume
						volume = (float) inPacket->volume / 255.f;
						soundPlayers.at(currentSoundPlayer).setVolume(volume);
						ofSoundSetVolume(volume);
						ofLog(OF_LOG_VERBOSE) << "SV, " << (int) inPacket->volume << endl;			
						
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
						LivestreamNetwork::Packet_TEMPERATURE_V1 outPacket;
						outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
						outPacket.hdr.packetCount = ++nPacketsSent;
						outPacket.hdr.protocolVersion = packetProtocolVersion;
						strncpy(outPacket.hdr.typeTag, LivestreamNetwork::TEMPERATURE, 
							sizeof LivestreamNetwork::TEMPERATURE / sizeof LivestreamNetwork::TEMPERATURE[0]);
							
						// Send the CPU temperature
						outPacket.temperature = (int) T;
						outPacket.sensorDesignator = 'C';
						udpSender.Send((char*) &outPacket, sizeof(outPacket));
						
                        /*
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
                        */
					
					} else if(memcmp( header->typeTag, LivestreamNetwork::SET_LED, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** SET_LED packet type ********** //
						
						// Get the LED data
						LivestreamNetwork::Packet_SET_LED_V1* inPacket = (LivestreamNetwork::Packet_SET_LED_V1 *) &udpMessage;
                        
                        string typeTag(header->typeTag);
                        ofLog(OF_LOG_VERBOSE) << ipAddress << " >> " << typeTag << " (" << inPacket->state << ")" << endl;  
                        
						// Set the LED outputs
						if (inPacket->state) {
							// Blink the LED
							//blinkLED->setval_gpio("1");
							//blinkLEDoutState = true;
							//netLED->setval_gpio("1");
                            int out = system(string("gpio write " + netLED + " 1").c_str());
							netLEDoutState = true;
						} else {							
							//blinkLED->setval_gpio("0");
							//blinkLEDoutState = false;
							//netLED->setval_gpio("0");
                            int out = system(string("gpio write " + netLED + " 0").c_str());
							netLEDoutState = false;
						}
					} else if(memcmp( header->typeTag, LivestreamNetwork::PLAY_NOTE, 
						sizeof header->typeTag / sizeof header->typeTag[0]) == 0) {
						// ********** PLAY_NOTE packet type ********** //
                        //volSound.loadSound("/livestream/audio/Tp10.wav");
                        
                        // Get the note file path
						LivestreamNetwork::Packet_PLAY_NOTE_V1* inPacket = (LivestreamNetwork::Packet_PLAY_NOTE_V1 *) &udpMessage;
					                       
                        ofLog(OF_LOG_VERBOSE) << ipAddress << " >> " << ofToString(header->typeTag) << " (" << inPacket->filePath << ")" << endl;                        
                        
                        // Increment the sound player to support note sustain
                        currentSoundPlayer = (currentSoundPlayer + 1) % nSoundPlayers;
                        
                        // Play the note
                        // Debugging
						//soundPlayers.at(0).play();
						//soundPlayers.at(0).setVolume(volume);
						//ofSoundSetVolume(volume);
                        
                        string filePath(inPacket->filePath);
                        soundPlayers.at(currentSoundPlayer).load(filePath);
                        soundPlayers.at(currentSoundPlayer).setMultiPlay(true);
						soundPlayers.at(currentSoundPlayer).play();
						soundPlayers.at(currentSoundPlayer).setVolume(volume);
						ofSoundSetVolume(volume);
					}
				}
			} //ipAddress.compare(maestroIpAddress) == 0
		} // !SET_MAESTRO_ADDRESS
	} // message!=""
    
    // ************************************** Master mode *************************************** //
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
		
		if (ofGetSystemTimeMicros() - blinkTimer >= blinkInterval) {
			if (debugTest != NO_SOUND) {
				soundPlayers.at(currentSoundPlayer).play();
			}
			
			if (debugTest != NO_GPIO) {
				// Blink the LED
				if (blinkLEDoutState) {
					//blinkLED->setval_gpio("0");
					//blinkLEDoutState = false;
				} else {
					//blinkLED->setval_gpio("1");
					//blinkLEDoutState = true;
				}
				
				if (netLEDoutState) {
					//netLED->setval_gpio("0");
					//netLEDoutState = false;
				} else {
					//netLED->setval_gpio("1");
					//netLEDoutState = true;
				}
			}
		
			cout << getDateTimeString() << setprecision(3) << ", LR, " << ofGetFrameRate();
			cout << ", LD, " << rawDist << ", LW, " << (int) smoothPwm;
			cout << ", AV, " << soundVolume << ", LS, " << signalStrength; 
			//cout << ", LN, " << maxNoise << ", LP, " << corrPeakVal;
			cout << ", HS, " << blinkLEDoutState;
			cout << endl;
				
			blinkTimer = ofGetSystemTimeMicros();
		}
		if (debugTest != NO_SOUND) {
			soundPlayers.at(currentSoundPlayer).setVolume(soundVolume);
			//ofSoundSetVolume(soundVolume);
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
			/*for (int j=0; j<nSensors; j++) {
				float tempData;
				if (debugTest == NO_TEMP) {
					// Isolate the temperature measurements from the code for debug
					tempData = 101;
				} else {
					tempData = tempSensor.read(j);
				}
				mFile << ", T" << j << ", " << tempData;
			}*/
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
		 // Load the packet data
        LivestreamNetwork::Packet_PONG_V1 outPacket;
        outPacket.hdr.timeStamp = ofGetElapsedTimeMillis();
        outPacket.hdr.packetCount = ++nPacketsSent;
        outPacket.hdr.protocolVersion = packetProtocolVersion;
        strncpy(outPacket.hdr.typeTag, LivestreamNetwork::PONG, 
            sizeof LivestreamNetwork::PONG / sizeof LivestreamNetwork::PONG[0]);
            
        // Send the packet
        udpBroadcaster.Send((char*) &outPacket, sizeof(outPacket));
        // Convert the typeTage char[2] to a string for logging
        string typeTag = string(outPacket.hdr.typeTag, outPacket.hdr.typeTag + sizeof(outPacket.hdr.typeTag) / 
            sizeof(outPacket.hdr.typeTag[0]));
        ofLog(OF_LOG_VERBOSE) << typeTag << ">>" << "Broadcast" << endl;		
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
    for (int j=1; j<nSoundPlayers; j++) {
        if (soundPlayers.at(j).isLoaded()) {
            soundPlayers.at(j).unloadSound();
        }
    }
	ofSoundShutdown();
	if (debugTest != NO_GPIO) {
        int out;
        out = system(string("gpio write " + blinkLED + " 0").c_str());
        out = system(string("gpio write " + netLED + " 0").c_str());
		//blinkLED->setval_gpio("0");
        //blinkLED->unexport_gpio();
        //free(blinkLED);
        //blinkLED = NULL;
		//netLED->setval_gpio("0");
        //netLED->unexport_gpio();
        //free(netLED);
        //netLED = NULL;
	}
    myLidarLite.stop();
}