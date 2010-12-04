#include "xminstrument.h"

#include <cstdint>

#pragma pack(push,1)
struct InstrumentHeader {
	uint32_t size;
	char name[22];
	uint8_t type;
	uint16_t numSamples;
};
struct InstrumentHeader2 {
	uint32_t size;
	uint8_t indices[96]; //!< @brief Maps note indices to their samples
	struct { int16_t x,y; } volEnvelope[12];
	struct { int16_t x,y; } panEnvelope[12];
	uint8_t numVolPoints;
	uint8_t numPanPoints;
	uint8_t volSustainPoint;
	uint8_t volLoopStart, volLoopEnd;
	uint8_t panSustainPoint;
	uint8_t panLoopStart, panLoopEnd;
	uint8_t volType, panType;
	uint8_t vibType, vibSweep, vibDepth, vibRate;
	uint8_t volFadeout;
	uint8_t reserved[11];
};
#pragma pack(pop)