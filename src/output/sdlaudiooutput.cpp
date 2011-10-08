/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sdlaudiooutput.h"

#include <SDL.h>
#include <boost/format.hpp>

void SDLAudioOutput::sdlAudioCallback(void* userdata, uint8_t* stream, int len_bytes) {
	std::fill_n(stream, len_bytes, 0);
	SDLAudioOutput* outpSdl = static_cast<SDLAudioOutput*>(userdata);
	while(len_bytes > 0) {
		AudioFrameBuffer buffer;
		IAudioSource::Ptr src(outpSdl->source().lock());
		size_t copied = src->getAudioData(buffer, len_bytes / sizeof(BasicSampleFrame));
		if(copied == 0) {
			outpSdl->setErrorCode(InputDry);
			outpSdl->pause();
			break;
		}
		copied *= sizeof(BasicSampleFrame);
		len_bytes -= copied;
		std::copy(buffer->begin(), buffer->end(), reinterpret_cast<BasicSampleFrame*>(stream));
		stream += copied;
	}
}

SDLAudioOutput::SDLAudioOutput(const IAudioSource::WeakPtr& src) : IAudioOutput(src) {
	logger()->trace(L4CXX_LOCATION, "Created");
}

SDLAudioOutput::~SDLAudioOutput() {
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	logger()->trace(L4CXX_LOCATION, "Destroyed");
}

int SDLAudioOutput::init(int desiredFrq) {
	if(!SDL_WasInit(SDL_INIT_AUDIO)) {
		if(-1 == SDL_Init(SDL_INIT_AUDIO)) {
			setErrorCode( OutputError );
			return 0;
		}
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
	if(SDL_OpenAudio(desired.get(), obtained.get()) < 0) {
		logger()->fatal(L4CXX_LOCATION, boost::format("Couldn't open audio. SDL reports '%s'")%SDL_GetError());
		setErrorCode( OutputError );
		return 0;
	}
/*	LOG_TEST_ERROR(desired->freq != obtained->freq);
	LOG_TEST_ERROR(desired->channels != obtained->channels);
	LOG_TEST_ERROR(desired->format != obtained->format);
	LOG_TEST_WARN(desired->samples != obtained->samples);*/
	desiredFrq = desired->freq;
	char driverName[256];
	if(SDL_AudioDriverName(driverName, 255)) {
		logger()->info(L4CXX_LOCATION, boost::format("Using audio driver '%s'")%driverName);
	}
	setErrorCode( NoError );
	return desiredFrq;
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
	return 0; // TODO
}

uint16_t SDLAudioOutput::volumeRight() const {
	return 0; // TODO
}

light4cxx::Logger::Ptr SDLAudioOutput::logger()
{
	return light4cxx::Logger::get( IAudioOutput::logger()->name() + ".sdl" );
}
