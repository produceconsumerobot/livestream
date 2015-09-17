
#pragma once

#include <string>
#include <stdint.h>


class LivestreamNetwork {
public:

	// ---------------------------------------------
	// Packet Type Tags
	static const char MODE_SLAVE[2];			// MS x
	static const char MODE_MASTER[2];			// MM x
	static const char SET_SUSTAIN_PEDAL[2];		// SP
	static const char SET_MAESTRO_ADDRESS[2];	// MA *
	static const char GET_DISTANCE[2];			// GD x
	static const char GET_SIGNAL_STRENGTH[2];	// GS [deprecated, use GET_DISTANCE]
	static const char DISTANCE[2];				// LD x
	static const char SIGNAL_STRENGTH[2];		// LS [deprecated, use DISTANCE]
	static const char SET_VOLUME[2];			// SV x
	static const char PING[2];					// PI x
	static const char SET_LED[2];				// SL x
	static const char GET_EYE_SAFETY[2];		// GE
	static const char GET_TRANSMIT_POWER[2];	// GP
	static const char GET_STATUS[2];			// GU
	static const char GET_LOOP_RATE[2];			// GL [deprecated?]
	static const char PONG[2];					// PO x
	static const char EYE_SAFETY[2];			// LE
	static const char TRANSMIT_POWER[2];		// LT
	static const char STATUS[2];				// LU
	static const char LOOP_RATE[2];				// LR [deprecated?]
	static const char PLAY_NOTE[2];				// PS x
	static const char GET_ALL_TEMPS[2];			// GT x
	static const char TEMPERATURE[2];			// TN x

	// ---------------------------------------------
	// Packet Definitions

	// Packet header structure
	struct PacketHeader_V1 {
		uint64_t timeStamp;			// milliseconds from session start to first data point of packet
		uint64_t packetCount;		// all packets per session (incl. this one)
		uint16_t protocolVersion;	// Version of the type tags
		char typeTag[2];			// type tag indicating the packet type
	};

	// Packet structure for sending a bool
	struct PacketNoPayload_V1 {
		PacketHeader_V1 hdr;
	};

	// Packet structure for sending a bool
	struct PacketBool_V1 {
		PacketHeader_V1 hdr;
		bool b;
	};

	// Packet structure for sending a uint8
	struct PacketUInt8_V1 {
		PacketHeader_V1 hdr;
		uint8_t u;
	};
	
	// Packet structure for sending an int16
	struct PacketInt16_V1 {
		PacketHeader_V1 hdr;
		int16_t i;
	};

	// Packet structure for sending an IP address
	struct PacketIPAddress_V1 {
		PacketHeader_V1 hdr;
		char ipAddress[20];
	};

	// Packet structure for sending a file path
	struct PacketFilePath_V1 {
		PacketHeader_V1 hdr;
		char filePath[256];
	};
	
	// Packet structure for sending temperature data
	struct PacketTemperature_V1 {
		PacketHeader_V1 hdr;
		int16_t temperature;
		char sensorDesignator;
	};

	// Packet structure for sending distance data
	struct PacketDistance_V1 {
		PacketHeader_V1 hdr;
		int16_t distance;
		int16_t signalStrength;
	};

	// ---------------------------------------------
	// Constructor
	LivestreamNetwork() {}; // Constructor
};

