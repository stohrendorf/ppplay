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

#ifndef SDLAUDIOOUTPUT_H
#define SDLAUDIOOUTPUT_H

#include "iaudiooutput.h"
#include "audiofifo.h"

#include <boost/thread.hpp>

/**
 * @ingroup Output
 * @{
 */

/**
 * @class SDLAudioOutput
 * @brief Output class for SDL
 */
class SDLAudioOutput : public IAudioOutput
{
	DISABLE_COPY( SDLAudioOutput )
	SDLAudioOutput() = delete;
public:
	//! @copydoc IAudioOutput::IAudioOutput(const IAudioSource::WeakPtr&)
	explicit SDLAudioOutput( const IAudioSource::WeakPtr& src );
	virtual ~SDLAudioOutput();
private:
	boost::mutex m_mutex;
	AudioFifo m_fifo;
	/**
	 * @brief SDL Audio callback handler
	 * @param[in] userdata Pointer to SDLAudioOutput
	 * @param[out] stream Audio buffer pointer
	 * @param[in] len_bytes Byte length of @a stream
	 * @note Declared here to get access to m_fifo
	 */
	static void sdlAudioCallback( void* userdata, uint8_t* stream, int len_bytes );
	size_t getSdlData( BasicSampleFrame* data, size_t numFrames );
	virtual int internal_init( int desiredFrq );
	virtual bool internal_playing() const;
	virtual bool internal_paused() const;
	virtual void internal_play();
	virtual void internal_pause();
	virtual uint16_t internal_volumeLeft() const;
	virtual uint16_t internal_volumeRight() const;
protected:
	/**
	 * @brief Get the logger
	 * @return Child logger with attached ".sdl"
	 */
	static light4cxx::Logger* logger();
};

/**
 * @}
 */

#endif
