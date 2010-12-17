#include "outsdl.h"

using namespace ppp;

static void sdlAudioCallback(void *userdata, Uint8 *stream, int len_bytes) {
	OutputSDL* outpSdl = static_cast<OutputSDL*>(userdata);
}

OutputSDL::OutputSDL (GenModule* mod) : OutputGen(mod)
{
}

int OutputSDL::init (int desiredFrq) {
	if(!SDL_WasInit(SDL_INIT_AUDIO)) {
		if(-1==SDL_Init(SDL_INIT_AUDIO))
			return -1;
	}
	else {
		// in case audio was inited, shut down the callbacks
		SDL_CloseAudio();
	}
	SDL_AudioSpec *desired = new SDL_AudioSpec;
	SDL_AudioSpec *obtained = new SDL_AudioSpec;
	desired->freq = desiredFrq;
	desired->channels = 2;
	desired->format = AUDIO_S16LSB;
	desired->samples = 2048;
	desired->callback = sdlAudioCallback;
	desired->userdata = this;
	if (SDL_OpenAudio(desired, obtained) < 0) {
		LOG_ERROR("Couldn't open audio: %s", SDL_GetError());
		delete obtained;
		delete desired;
		return -1;
	}
	LOG_TEST_ERROR(desired->freq != obtained->freq);
	LOG_TEST_ERROR(desired->channels != obtained->channels);
	LOG_TEST_ERROR(desired->format != obtained->format);
	LOG_TEST_WARN(desired->samples != obtained->samples);
	desiredFrq = desired->freq;
	delete obtained;
	delete desired;
	char driverName[1024];
	if(SDL_AudioDriverName(driverName, 1023))
		LOG_MESSAGE("Using audio driver '%s'", driverName);
	return desiredFrq;
}
