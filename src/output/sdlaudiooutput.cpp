#include "sdlaudiooutput.h"
#include "logger/logger.h"

using namespace ppp;

void SDLAudioOutput::sdlAudioCallback(void *userdata, Uint8 *stream, int len_bytes) {
	std::fill_n(stream, len_bytes, 0);
	SDLAudioOutput* outpSdl = static_cast<SDLAudioOutput*>(userdata);
	while(len_bytes>0) {
		if(!outpSdl->fillFifo()) {
			outpSdl->setErrorCode(InputDry);
			outpSdl->pause();
			break;
		}
		std::size_t copied = outpSdl->m_fifo.pull(reinterpret_cast<BasicSampleFrame*>(stream), len_bytes/sizeof(BasicSampleFrame));
		if(copied==0) {
			outpSdl->setErrorCode(InputDry);
			outpSdl->pause();
			break;
		}
		copied *= sizeof(BasicSampleFrame);
		len_bytes -= copied;
		stream += copied;
	}
}

SDLAudioOutput::SDLAudioOutput(IAudioSource* src) : IAudioOutput(src), m_fifo(2048)
{
}

SDLAudioOutput::~SDLAudioOutput() {
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

int SDLAudioOutput::init(int desiredFrq) {
	if(!SDL_WasInit(SDL_INIT_AUDIO)) {
		if(-1 == SDL_Init(SDL_INIT_AUDIO))
			return 0;
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
		return 0;
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

bool SDLAudioOutput::fillFifo() {
	while(m_fifo.needsData()) {
		AudioFrameBuffer buf;
		if(0 == source()->getAudioData(buf, m_fifo.minFrameCount() - m_fifo.queuedLength()))
			return false;
		m_fifo.push(buf);
	}
	return true;
}

bool SDLAudioOutput::playing() {
	return SDL_GetAudioStatus() == SDL_AUDIO_PLAYING;
}
bool SDLAudioOutput::paused() {
	return SDL_GetAudioStatus() == SDL_AUDIO_PAUSED;
}

void SDLAudioOutput::play() {
	SDL_PauseAudio(0);
}
void SDLAudioOutput::pause() {
	SDL_PauseAudio(1);
}

uint16_t SDLAudioOutput::volumeLeft() const {
	return m_fifo.volumeLeft();
}

uint16_t SDLAudioOutput::volumeRight() const {
	return m_fifo.volumeRight();
}
