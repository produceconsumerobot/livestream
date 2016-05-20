
#include "LivestreamNetwork.h"

const char LivestreamNetwork::MODE_SLAVE[2] =				{'M', 'S'};
const char LivestreamNetwork::MODE_MASTER[2] =				{'M', 'M'};
const char LivestreamNetwork::SET_SUSTAIN_PEDAL[2] =		{'S', 'P'};
const char LivestreamNetwork::SET_MAESTRO_ADDRESS[2] =		{'M', 'A'};
const char LivestreamNetwork::GET_DISTANCE[2] =				{'G', 'D'};
//const char LivestreamNetwork::GET_SIGNAL_STRENGTH[2] =	{'G', 'S'};
const char LivestreamNetwork::DISTANCE[2] =					{'L', 'D'};
//const char LivestreamNetwork::SIGNAL_STRENGTH[2] =		{'L', 'S'};
const char LivestreamNetwork::SET_VOLUME[2] =				{'S', 'V'};
const char LivestreamNetwork::PING[2] =						{'P', 'I'};
const char LivestreamNetwork::PONG[2] =						{'P', 'O'};
const char LivestreamNetwork::SET_LED[2] =					{'S', 'L'};
const char LivestreamNetwork::GET_EYE_SAFETY[2] =			{'G', 'E'};
const char LivestreamNetwork::EYE_SAFETY[2] =				{'L', 'E'};
const char LivestreamNetwork::GET_TRANSMIT_POWER[2] =		{'G', 'P'};
const char LivestreamNetwork::TRANSMIT_POWER[2] =			{'L', 'T'};
const char LivestreamNetwork::GET_STATUS[2] =				{'G', 'U'};
const char LivestreamNetwork::STATUS[2] =					{'L', 'U'};
//const char LivestreamNetwork::GET_LOOP_RATE[2] =			{'G', 'L'};
//const char LivestreamNetwork::LOOP_RATE[2] =				{'L', 'R'};
const char LivestreamNetwork::LOAD_NOTE_FILE[2]				{'N', 'L'};
const char LivestreamNetwork::CLEAR_NOTE_FILES[2]			{ 'C', 'N' };
const char LivestreamNetwork::GET_NOTE_FILES_COUNT[2]		{ 'N', 'G' };
const char LivestreamNetwork::NOTE_FILES_COUNT[2]			{ 'N', 'C' };
const char LivestreamNetwork::PLAY_NOTE[2] =				{'N', 'P'};
const char LivestreamNetwork::GET_ALL_TEMPS[2] =			{'G', 'T'};
const char LivestreamNetwork::TEMPERATURE[2] =				{'T', 'N'};