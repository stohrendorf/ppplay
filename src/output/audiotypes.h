#ifndef AUDIOTYPES_H
#define AUDIOTYPES_H

#include <vector>
#include <deque>
#include <memory>

namespace ppp {
	typedef int16_t BasicSample;
	struct __attribute__((packed)) BasicSampleFrame { BasicSample left,right; };
	typedef int32_t MixerSample;
	struct __attribute__((packed)) MixerSampleFrame { MixerSample left,right; };
	typedef std::shared_ptr< std::vector<BasicSampleFrame> > AudioFrameBuffer; //!< @brief Audio buffer
	typedef std::shared_ptr< std::vector<MixerSampleFrame> > MixerFrameBuffer; //!< @brief Mixer buffer
	typedef std::deque< AudioFrameBuffer > AudioFrameBufferQueue;
}

#endif // AUDIOTYPES_H