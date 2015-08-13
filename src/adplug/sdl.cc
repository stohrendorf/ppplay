/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001 - 2003 Simon Peter <dn.tlp@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <unistd.h>

#include "sdl.h"
#include "defines.h"

#include "genmod/breseninter.h"

SDLPlayer::SDLPlayer(int freq, size_t bufsize)
    : m_interp(freq, opl::Opl3::SampleRate)
{
    memset(&m_spec, 0x00, sizeof(SDL_AudioSpec));

    if(SDL_Init(SDL_INIT_AUDIO) < 0) {
        message(MSG_ERROR, "unable to initialize SDL -- %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    m_spec.freq = freq;
    m_spec.format = AUDIO_S16SYS;
    m_spec.channels = 2;
    m_spec.samples = bufsize;
    m_spec.callback = SDLPlayer::callback;
    m_spec.userdata = this;

    if(SDL_OpenAudio(&m_spec, NULL) < 0) {
        message(MSG_ERROR, "unable to open audio -- %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    message(MSG_DEBUG, "got audio buffer size -- %d", m_spec.size);
}

SDLPlayer::~SDLPlayer()
{
    if(!SDL_WasInit(SDL_INIT_AUDIO))
        return;

    SDL_CloseAudio();
    SDL_Quit();
}

void SDLPlayer::frame()
{
    SDL_PauseAudio(0);
    SDL_Delay(m_spec.freq / (m_spec.size / 4));
}

void SDLPlayer::callback(void *userdata, Uint8 *audiobuf, int byteLen)
{
    SDLPlayer* self = reinterpret_cast<SDLPlayer*>(userdata);
    static long framesUntilUpdate = 0;
    size_t framesToWrite = byteLen / 4;
    int16_t *bufPtr = reinterpret_cast<int16_t*>(audiobuf);
    // Prepare audiobuf with emulator output
    while(framesToWrite > 0) {
        while(framesUntilUpdate <= 0) {
            self->setIsPlaying( self->getPlayer()->update() );
            framesUntilUpdate += self->getPlayer()->framesUntilUpdate();
        }

        std::array<int16_t,4> samples;
        self->getPlayer()->read(&samples);
        bufPtr[0] = ppp::clip(samples[0] + samples[2],-32768,32767);
        bufPtr[1] = ppp::clip(samples[1] + samples[3],-32768,32767);
        bufPtr += 2;

        if( self->m_interp.next() == 2 ) {
            self->getPlayer()->read( nullptr ); // skip a sample
            --framesUntilUpdate;
        }
        self->m_interp = 0;
        --framesUntilUpdate;
        --framesToWrite;
    }
}
