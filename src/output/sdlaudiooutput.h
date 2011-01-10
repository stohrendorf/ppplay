/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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
		DISABLE_COPY(SDLAudioOutput)
		SDLAudioOutput() = delete;
	public:
		SDLAudioOutput(IAudioSource* src);
		virtual ~SDLAudioOutput();
		virtual int init(int desiredFrq);
		virtual bool playing();
		virtual bool paused();
		virtual bool stopped();
		virtual void play();
		virtual void pause();
		virtual uint16_t volumeLeft() const;
		virtual uint16_t volumeRight() const;
	private:
		static void sdlAudioCallback(void *userdata, Uint8 *stream, int len_bytes);
		ppp::AudioFifo m_fifo;
		void fillFifo();
};

#endif