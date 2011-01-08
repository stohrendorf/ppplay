#include "outsdl.h"
#include "logger/logger.h"

using namespace ppp;

void OutputSDL::sdlAudioCallback(void *userdata, Uint8 *stream, int len_bytes) {
	OutputSDL* outpSdl = static_cast<OutputSDL*>(userdata);
	while(len_bytes>0) {
		outpSdl->fillFifo();
		std::size_t copied = outpSdl->m_fifo.pull(reinterpret_cast<BasicSampleFrame*>(stream), len_bytes/sizeof(BasicSampleFrame));
		len_bytes -= copied*sizeof(BasicSampleFrame);
		LOG_TEST_ERROR(copied==0);
	}
}

OutputSDL::OutputSDL(IAudioSource* src) : OutputGen(src), m_fifo(2048)
{
}

OutputSDL::~OutputSDL() {
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

int OutputSDL::init(int desiredFrq) {
	if(!SDL_WasInit(SDL_INIT_AUDIO)) {
		if(-1 == SDL_Init(SDL_INIT_AUDIO))
			return -1;
	}
	else {
		// in case audio was already inited, shut down the callbacks
		SDL_CloseAudio();
	}
	std::shared_ptr<SDL_AudioSpec> desired(new SDL_AudioSpec);
	std::shared_ptr<SDL_AudioSpec> obtained(new SDL_AudioSpec);
	desired->freq = desiredFrq;
	desired->channels = 2;
	desired->format = AUDIO_S16LSB;
	desired->samples = 2048;
	desired->callback = sdlAudioCallback;
	desired->userdata = this;
	if (SDL_OpenAudio(desired.get(), obtained.get()) < 0) {
		LOG_ERROR("Couldn't open audio: %s", SDL_GetError());
		return -1;
	}
	LOG_TEST_ERROR(desired->freq != obtained->freq);
	LOG_TEST_ERROR(desired->channels != obtained->channels);
	LOG_TEST_ERROR(desired->format != obtained->format);
	LOG_TEST_WARN(desired->samples != obtained->samples);
	desiredFrq = desired->freq;
	char driverName[1024];
	if(SDL_AudioDriverName(driverName, 1023))
		LOG_MESSAGE("Using audio driver '%s'", driverName);
	return desiredFrq;
}

void OutputSDL::fillFifo() {
	while(m_fifo.needsData()) {
		AudioFrameBuffer buf;
		source()->getAudioData(buf, m_fifo.minFrameCount() - m_fifo.queuedLength());
		if(buf->size()==0)
			break;
		m_fifo.push(buf);
	}
}

bool OutputSDL::isPlaying() {
	return SDL_GetAudioStatus() == SDL_AUDIO_PLAYING;
}

void OutputSDL::play() {
	SDL_PauseAudio(0);
}

void OutputSDL::pause() {
	SDL_PauseAudio(1);
}
