/*
    PPPlay - an old-fashioned module player
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

void SDLAudioOutput::sdlAudioCallback(void* userdata, uint8_t* stream, int len_bytes)
{
    SDLAudioOutput* outpSdl = static_cast<SDLAudioOutput*>(userdata);
    logger()->trace(L4CXX_LOCATION, "Requested %d bytes of data", len_bytes);
    size_t copiedBytes = sizeof(BasicSampleFrame) * outpSdl->getSdlData(reinterpret_cast<BasicSampleFrame*>(stream), len_bytes / sizeof(BasicSampleFrame));
    std::fill_n(stream + copiedBytes, len_bytes - copiedBytes, 0);
}

size_t SDLAudioOutput::getSdlData(BasicSampleFrame* data, size_t numFrames)
{
    std::unique_lock<std::mutex> lock(m_mutex, std::try_to_lock);
    if(!lock.owns_lock())
    {
        logger()->warn(L4CXX_LOCATION, "Failed to lock mutex");
        return 0;
    }
    if(paused() || m_fifo.isSourcePaused())
        return 0;
    AudioFrameBuffer buffer;
    size_t copied = m_fifo.pullData(buffer, numFrames);
    if(copied == 0)
    {
        logger()->trace(L4CXX_LOCATION, "Source did not return any data - input is dry");
        setErrorCode(InputDry);
        pause();
        return 0;
    }
    setErrorCode(NoError);
    numFrames -= copied;
    if(numFrames != 0)
    {
        logger()->trace(L4CXX_LOCATION, "Source provided not enough data: %d frames missing", numFrames);
    }
    std::copy_n(buffer->begin(), copied, data);
    return copied;
}

SDLAudioOutput::SDLAudioOutput(const AbstractAudioSource::WeakPtr& src) :
    AbstractAudioOutput(src),
    m_mutex(),
    m_fifo(src, 4096),
    m_volObserver(&m_fifo),
    m_fftObserver(&m_fifo)
{
    logger()->trace(L4CXX_LOCATION, "Created");
}

SDLAudioOutput::~SDLAudioOutput()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    SDL_CloseAudio();
    logger()->trace(L4CXX_LOCATION, "Destroyed");
}

int SDLAudioOutput::internal_init(int desiredFrq)
{
    logger()->trace(L4CXX_LOCATION, "Initializing");
    if(!SDL_WasInit(SDL_INIT_AUDIO))
    {
        logger()->trace(L4CXX_LOCATION, "Initializing SDL Audio component");
        if(-1 == SDL_Init(SDL_INIT_AUDIO))
        {
            logger()->fatal(L4CXX_LOCATION, "SDL Audio component initialization failed. SDL Error: '%s'", SDL_GetError());
            setErrorCode(OutputError);
            return 0;
        }
    }
    else
    {
        // in case audio was already inited, shut down the callbacks
        SDL_CloseAudio();
    }
    std::unique_ptr<SDL_AudioSpec> desired(new SDL_AudioSpec);
    std::unique_ptr<SDL_AudioSpec> obtained(new SDL_AudioSpec);
    desired->freq = desiredFrq;
    desired->channels = 2;
    desired->format = AUDIO_S16LSB;
    desired->samples = 2048;
    desired->callback = sdlAudioCallback;
    desired->userdata = this;
    if(SDL_OpenAudio(desired.get(), obtained.get()) < 0)
    {
        logger()->fatal(L4CXX_LOCATION, "Couldn't open audio. SDL Error: '%s'", SDL_GetError());
        setErrorCode(OutputError);
        return 0;
    }
    desiredFrq = desired->freq;
    if(const char* driverName = SDL_GetCurrentAudioDriver())
    {
        logger()->info(L4CXX_LOCATION, "Using audio driver '%s'", driverName);
    }
    setErrorCode(NoError);
    logger()->trace(L4CXX_LOCATION, "Initialized");
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
    SDL_PauseAudio(0);
    // only wait at most 1 sec until playing
    for(int n = 0; n < 100 && SDL_GetAudioStatus() != SDL_AUDIO_PLAYING; ++n)
    {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    setErrorCode(NoError);
}

void SDLAudioOutput::internal_pause()
{
    SDL_PauseAudio(1);
    // only wait at most 1 sec until not playing anymore
    for(int n = 0; n < 100 && SDL_GetAudioStatus() == SDL_AUDIO_PLAYING; ++n)
    {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

uint16_t SDLAudioOutput::internal_volumeLeft() const
{
    return m_volObserver.leftVol();
}

uint16_t SDLAudioOutput::internal_volumeRight() const
{
    return m_volObserver.rightVol();
}

light4cxx::Logger* SDLAudioOutput::logger()
{
    return light4cxx::Logger::get(AbstractAudioOutput::logger()->name() + ".sdl");
}