/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#include <SDL.h>
#include "iaudiooutput.h"
#include "audiofifo.h"

/**
 * @class OutputSDL
 * @ingroup Output
 * @brief Output class for SDL
 */
class SDLAudioOutput : public IAudioOutput {
		DISABLE_COPY( SDLAudioOutput )
		SDLAudioOutput() = delete;
	public:
		//! @copydoc IAudioOutput::IAudioOutput(IAudioSource*)
		explicit SDLAudioOutput( IAudioSource* src );
		virtual ~SDLAudioOutput();
		virtual int init( int desiredFrq );
		virtual bool playing();
		virtual bool paused();
		virtual void play();
		virtual void pause();
		virtual uint16_t volumeLeft() const;
		virtual uint16_t volumeRight() const;
	private:
		/**
		 * @brief SDL Audio callback handler
		 * @param[in] userdata Pointer to SDLAudioOutput
		 * @param[out] stream Audio buffer pointer
		 * @param[in] len_bytes Byte length of @a stream
		 */
		static void sdlAudioCallback( void* userdata, Uint8* stream, int len_bytes );
		ppp::AudioFifo m_fifo; //!< @brief FIFO buffer
		/**
		 * @brief Fills m_fifo
		 * @return @c false if the FIFO could not be filled
		 */
		bool fillFifo();
};

#endif
