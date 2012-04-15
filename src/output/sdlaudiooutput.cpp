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

void SDLAudioOutput::sdlAudioCallback( void* userdata, uint8_t* stream, int len_bytes )
{
	SDLAudioOutput* outpSdl = static_cast<SDLAudioOutput*>( userdata );
	logger()->trace(L4CXX_LOCATION, "Requested %d bytes of data", len_bytes);
	len_bytes -= sizeof(BasicSampleFrame) * outpSdl->getSdlData(reinterpret_cast<BasicSampleFrame*>(stream), len_bytes/sizeof(BasicSampleFrame));
	std::fill_n(stream, len_bytes, 0);
}

size_t SDLAudioOutput::getSdlData( BasicSampleFrame* data, size_t numFrames )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	AudioFrameBuffer buffer;
	IAudioSource::Ptr src( source().lock() );
	if( !src || src->paused() ) {
		return 0;
	}
	size_t copied = src->getAudioData( buffer, numFrames );
	if( copied == 0 ) {
		logger()->trace(L4CXX_LOCATION, "Source did not return any data - input is dry");
		setErrorCode( InputDry );
	}
	numFrames -= copied;
	if(numFrames != 0) {
		logger()->trace(L4CXX_LOCATION, "Source provided not enough data: %d frames missing", numFrames);
	}
	std::copy( buffer->begin(), buffer->end(), data );
	return copied;
}

SDLAudioOutput::SDLAudioOutput( const IAudioSource::WeakPtr& src ) : IAudioOutput( src ), m_mutex()
{
	logger()->trace( L4CXX_LOCATION, "Created" );
}

SDLAudioOutput::~SDLAudioOutput()
{
	SDL_CloseAudio();
	SDL_QuitSubSystem( SDL_INIT_AUDIO );
	logger()->trace( L4CXX_LOCATION, "Destroyed" );
}

int SDLAudioOutput::internal_init( int desiredFrq )
{
	logger()->trace( L4CXX_LOCATION, "Initializing" );
	if( !SDL_WasInit( SDL_INIT_AUDIO ) ) {
		logger()->trace( L4CXX_LOCATION, "Initializing SDL Audio component" );
		if( -1 == SDL_Init( SDL_INIT_AUDIO ) ) {
			logger()->fatal( L4CXX_LOCATION, "SDL Audio component initialization failed. SDL Error: '%s'", SDL_GetError() );
			setErrorCode( OutputError );
			return 0;
		}
	}
	else {
		// in case audio was already inited, shut down the callbacks
		SDL_CloseAudio();
	}
	std::shared_ptr<SDL_AudioSpec> desired( new SDL_AudioSpec );
	std::shared_ptr<SDL_AudioSpec> obtained( new SDL_AudioSpec );
	desired->freq = desiredFrq;
	desired->channels = 2;
	desired->format = AUDIO_S16LSB;
	desired->samples = 2048;
	desired->callback = sdlAudioCallback;
	desired->userdata = this;
	if( SDL_OpenAudio( desired.get(), obtained.get() ) < 0 ) {
		logger()->fatal( L4CXX_LOCATION, "Couldn't open audio. SDL Error: '%s'", SDL_GetError() );
		setErrorCode( OutputError );
		return 0;
	}
	/*	LOG_TEST_ERROR(desired->freq != obtained->freq);
		LOG_TEST_ERROR(desired->channels != obtained->channels);
		LOG_TEST_ERROR(desired->format != obtained->format);
		LOG_TEST_WARN(desired->samples != obtained->samples);*/
	desiredFrq = desired->freq;
	char driverName[256];
	if( SDL_AudioDriverName( driverName, 255 ) ) {
		logger()->info( L4CXX_LOCATION, "Using audio driver '%s'", driverName );
	}
	setErrorCode( NoError );
	logger()->trace( L4CXX_LOCATION, "Initialized" );
	return desiredFrq;
}

bool SDLAudioOutput::internal_playing() const
{
	return SDL_GetAudioStatus() == SDL_AUDIO_PLAYING;
}

bool SDLAudioOutput::internal_paused() const
{
	return SDL_GetAudioStatus() == SDL_AUDIO_PAUSED;
}

void SDLAudioOutput::internal_play()
{
	SDL_PauseAudio( 0 );
}

void SDLAudioOutput::internal_pause()
{
	SDL_PauseAudio( 1 );
}

light4cxx::Logger::Ptr SDLAudioOutput::logger()
{
	return light4cxx::Logger::get( IAudioOutput::logger()->name() + ".sdl" );
}
