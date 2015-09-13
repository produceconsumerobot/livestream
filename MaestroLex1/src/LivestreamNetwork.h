
#pragma once

#include <string>
#include <stdint.h>


class LivestreamNetwork {
public:

	// ---------------------------------------------
	// Packet Type Tags
	static const char SET_SUSTAIN_PEDAL[2];		// SP
	static const char SET_MAESTRO_ADDRESS[2];	// MA
	static const char GET_DISTANCE[2];			// GD
	static const char GET_SIGNAL_STRENGTH[2];	// GS
	static const char DISTANCE[2];				// LD
	static const char SIGNAL_STRENGTH[2];		// LS 
	static const char SET_VOLUME[2];			// SV
	static const char PING[2];					// PI
	static const char SET_LED[2];				// SL
	static const char GET_EYE_SAFETY[2];		// GE
	static const char GET_TRANSMIT_POWER[2];	// GP
	static const char GET_STATUS[2];			// GU
	static const char GET_LOOP_RATE[2];			// GL
	static const char PONG[2];					// PO
	static const char EYE_SAFETY[2];			// LE
	static const char TRANSMIT_POWER[2];		// LT
	static const char STATUS[2];				// LU
	static const char LOOP_RATE[2];				// LR
	static const char PLAY_SOUND[2];			// PS
	static const char GET_ALL_TEMPS[2];			// GT
	static const char TEMPERATURE[2];			// TN

	// ---------------------------------------------
	// Packet Definitions

	// Packet header structure
	static struct LivestreamPacketHeader_V1 {
		uint32_t timeStamp;		// milliseconds from session start to first data point of packet
		uint32_t packetCount;	// all packets per session (incl. this one)
		uint8_t typeTag;		// length of data value array
		uint8_t protocolVersion;
	};

	// Packet structure for sending an IP address
	static struct LivestreamPacketIPAddress_V1 {
		LivestreamPacketHeader_V1 header;
		char ipAddress[16];
	};

	// Packet structure for sending a file path
	static struct LivestreamPacketFilePath_V1 {
		LivestreamPacketHeader_V1 header;
		char filePath[256];
	};

	// Packet structure for sending a bool
	static struct LivestreamPacketBool_V1 {
		LivestreamPacketHeader_V1 header;
		bool b;
	};

	// Packet structure for sending a uint8
	static struct LivestreamPacketUInt8_V1 {
		LivestreamPacketHeader_V1 header;
		uint8_t u;
	};
	
	// Packet structure for sending an int16
	static struct LivestreamPacketInt16_V1 {
		LivestreamPacketHeader_V1 header;
		int16_t i;
	};
	
	// Packet structure for sending an int16
	static struct LivestreamPacketTemperature_V1 {
		LivestreamPacketHeader_V1 header;
		int16_t temperature;
		char sensorDesignator;
	};

	// ---------------------------------------------
	// Constructor
	LivestreamNetwork() {}; // Constructor
};

const char LivestreamNetwork::SET_SUSTAIN_PEDAL[2] =	{'S', 'P'};
const char LivestreamNetwork::SET_MAESTRO_ADDRESS[2] =	{'M', 'A'};
const char LivestreamNetwork::GET_DISTANCE[2] =			{'G', 'D'};
const char LivestreamNetwork::GET_SIGNAL_STRENGTH[2] =	{'G', 'S'};
const char LivestreamNetwork::DISTANCE[2] =				{'L', 'D'};
const char LivestreamNetwork::SIGNAL_STRENGTH[2] =		{'L', 'S'};
const char LivestreamNetwork::SET_VOLUME[2] =			{'S', 'V'};
const char LivestreamNetwork::PING[2] =					{'P', 'I'};
const char LivestreamNetwork::SET_LED[2] =				{'S', 'L'};
const char LivestreamNetwork::GET_EYE_SAFETY[2] =		{'G', 'E'};
const char LivestreamNetwork::GET_TRANSMIT_POWER[2] =	{'G', 'P'};
const char LivestreamNetwork::GET_STATUS[2] =			{'G', 'U'};
const char LivestreamNetwork::GET_LOOP_RATE[2] =		{'G', 'L'};
const char LivestreamNetwork::PONG[2] =					{'P', 'O'};
const char LivestreamNetwork::EYE_SAFETY[2] =			{'L', 'E'};
const char LivestreamNetwork::TRANSMIT_POWER[2] =		{'L', 'T'};
const char LivestreamNetwork::STATUS[2] =				{'L', 'U'};
const char LivestreamNetwork::LOOP_RATE[2] =			{'L', 'R'};
const char LivestreamNetwork::PLAY_SOUND[2] =			{'P', 'S'};
const char LivestreamNetwork::GET_ALL_TEMPS[2] =		{'G', 'T'};
const char LivestreamNetwork::TEMPERATURE[2] =			{'T', 'N'};
