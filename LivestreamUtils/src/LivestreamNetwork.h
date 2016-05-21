
#pragma once

#include <string>
#include <stdint.h>


class LivestreamNetwork {
public:

	// ---------------------------------------------
	// Packet Type Tags
	static const char MODE_SLAVE[2];				// MS x
	static const char MODE_MASTER[2];				// MM x
	static const char SET_SUSTAIN_PEDAL[2];			// SP
	static const char SET_MAESTRO_ADDRESS[2];		// MA *
	static const char GET_DISTANCE[2];				// GD x
	//static const char GET_SIGNAL_STRENGTH[2];		// GS [deprecated, use GET_DISTANCE]
	static const char DISTANCE[2];					// LD x
	//static const char SIGNAL_STRENGTH[2];			// LS [deprecated, use DISTANCE]
	static const char SET_VOLUME[2];				// SV x
	static const char PING[2];						// PI x
	static const char PONG[2];						// PO x
	static const char SET_LED[2];					// SL x
	static const char GET_EYE_SAFETY[2];			// GE
	static const char EYE_SAFETY[2];				// LE
	static const char GET_TRANSMIT_POWER[2];		// GP
	static const char TRANSMIT_POWER[2];			// LT
	static const char GET_STATUS[2];				// GU
	static const char STATUS[2];					// LU
	//static const char GET_LOOP_RATE[2];			// GL [deprecated?]
	//static const char LOOP_RATE[2];				// LR [deprecated?]
	static const char PLAY_NOTE[2];					// PN
	static const char GET_ALL_TEMPS[2];				// GT x
	static const char TEMPERATURE[2];				// TN x

	// ---------------------------------------------
	// Packet Definitions

	// Packet header structure
	struct PacketHeader_V1 {
		uint64_t timeStamp;			// milliseconds from session start to first data point of packet
		uint64_t packetCount;		// all packets per session (incl. this one)
		uint16_t protocolVersion;	// Version of the type tags
		char typeTag[2];			// type tag indicating the packet type
	};
	/*
	// Packet structure for sending a bool
	struct Packet_NoPayload_V1 {
		PacketHeader_V1 hdr;
	};

	// Packet structure for sending a bool
	struct Packet_Bool_V1 {
		PacketHeader_V1 hdr;
		bool b;
	};

	// Packet structure for sending a uint8
	struct Packet_UInt8_V1 {
		PacketHeader_V1 hdr;
		uint8_t u;
	};
	
	// Packet structure for sending an int16
	struct Packet_Int16_V1 {
		PacketHeader_V1 hdr;
		int16_t i;
	};

	// Packet structure for sending an IP address
	struct Packet_IPAddress_V1 {
		PacketHeader_V1 hdr;
		char ipAddress[20];
	};

	// Packet structure for sending a file path
	struct Packet_FilePath_V1 {
		PacketHeader_V1 hdr;
		char filePath[256];
	};
	*/
	
	struct Packet_MODE_SLAVE_V1 {
		PacketHeader_V1 hdr;
	};

	struct Packet_MODE_MASTER_V1 {
		PacketHeader_V1 hdr;
	};

	struct Packet_SET_SUSTAIN_PEDAL_V1 {
		PacketHeader_V1 hdr;
		bool state;
	};

	struct Packet_SET_MAESTRO_ADDRESS_V1 {
		PacketHeader_V1 hdr;
		char ipAddress[20];
	};

	struct Packet_GET_DISTANCE_V1 {
		PacketHeader_V1 hdr;
	};

	struct Packet_DISTANCE_V1 {
		PacketHeader_V1 hdr;
		int16_t distance;
		int16_t signalStrength;
	};

	struct Packet_SET_VOLUME_V1 {
		PacketHeader_V1 hdr;
		uint8_t volume;
	};

	struct Packet_PING_V1 {
		PacketHeader_V1 hdr;
	};

	struct Packet_PONG_V1 {
		PacketHeader_V1 hdr;
	};

	struct Packet_SET_LED_V1 {
		PacketHeader_V1 hdr;
		bool state;
	};

	struct Packet_GET_EYE_SAFETY_V1 {
		PacketHeader_V1 hdr;
	};

	struct Packet_EYE_SAFETY_V1 {
		PacketHeader_V1 hdr;
		int8_t eyeSafety;
	};

	struct Packet_GET_TRANSMIT_POWER_V1 {
		PacketHeader_V1 hdr;
	};

	struct Packet_TRANSMIT_POWER_V1 {
		PacketHeader_V1 hdr;
		int16_t transmitPower;
	};

	struct Packet_GET_STATUS_V1 {
		PacketHeader_V1 hdr;
	};

	struct Packet_STATUS_V1 {
		PacketHeader_V1 hdr;
		int16_t status;
	};

	struct Packet_PLAY_NOTE_V1 {
		PacketHeader_V1 hdr;
		char filePath[256];
	};

	struct Packet_GET_ALL_TEMPS_V1 {
		PacketHeader_V1 hdr;
	};

	struct Packet_TEMPERATURE_V1 {
		PacketHeader_V1 hdr;
		int16_t temperature;
		char sensorDesignator;
	};

	// ---------------------------------------------
	// Constructor
	LivestreamNetwork() {}; // Constructor
};

