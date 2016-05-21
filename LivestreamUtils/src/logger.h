//
//  telephoneRewired.h
//
//  Created by Sean Montgomery on 12/18/12.
//  http://produceconsumerobot.com/
//
//  This work is licensed under the Creative Commons 
//  Attribution-ShareAlike 3.0 Unported License. 
//  To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/.
//

#ifndef _TELEPHONE_REWIRED_LOGGER
#define _TELEPHONE_REWIRED_LOGGER

#include <algorithm>
#include <vector>

#include "ofMain.h"



/*-------------------------------------------------
* LoggerThread
* Thread to read log data in the following format
* ofTimestamp, dataTypeTag, dataPayload
* Types of data: 
* zeoRawData 
*	data float[128]
* zeoSliceData
*	int packetNumber
*	zeoTimestamp int
*	powerData float[7]
*	SQI int
*	Impedance int
*	badSignal bool
*	int version
*	int stage
* entrainmentData
*	isOutputOn bool 
*	freqency float
* 
*-------------------------------------------------*/
class LoggerThread : public ofThread {
private:
	string _logDirPath;
	string _fileName;
public:
	LoggerThread();
	~LoggerThread();
	LoggerThread(string logDirPath);
	void setDirPath(string logDirPath);
	static string dateTimeString();
	void log(string data);
	void logNext();
	void logAll();

	void threadedFunction();

	queue<string> loggerQueue;
};

#endif


