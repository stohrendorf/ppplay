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

SDLPlayer::SDLPlayer(opl::Opl3 *nopl, int freq, unsigned long bufsize)
  : oplChip(nopl), interp(freq, opl::Opl3::SampleRate), filters{ppp::OplFilter{freq},ppp::OplFilter{freq}}
{
   memset(&spec, 0x00, sizeof(SDL_AudioSpec));

   if(SDL_Init(SDL_INIT_AUDIO) < 0) {
      message(MSG_ERROR, "unable to initialize SDL -- %s", SDL_GetError());
      exit(EXIT_FAILURE);
   }

   spec.freq = freq;
   spec.format = AUDIO_S16SYS;
   spec.channels = 2;
   spec.samples = bufsize;
   spec.callback = SDLPlayer::callback;
   spec.userdata = this;

   if(SDL_OpenAudio(&spec, NULL) < 0) {
      message(MSG_ERROR, "unable to open audio -- %s", SDL_GetError());
      exit(EXIT_FAILURE);
   }

   message(MSG_DEBUG, "got audio buffer size -- %d", spec.size);
}

SDLPlayer::~SDLPlayer()
{
  if(!SDL_WasInit(SDL_INIT_AUDIO)) return;

  SDL_CloseAudio();
  SDL_Quit();
}

void SDLPlayer::frame()
{
  SDL_PauseAudio(0);
  SDL_Delay(spec.freq / (spec.size / 4));
}

void SDLPlayer::callback(void *userdata, Uint8 *audiobuf, int len)
{
  SDLPlayer	*self = (SDLPlayer *)userdata;
  static long	minicnt = 0;
  long		i, towrite = len / 4;
  int16_t *pos = reinterpret_cast<int16_t*>(audiobuf);

  // Prepare audiobuf with emulator output
  while(towrite > 0) {
    while(minicnt < 0) {
      minicnt += self->spec.freq;
      self->playing = self->p->update();
    }
    i = std::min(towrite, (long)(minicnt / self->p->getrefresh() + 4) & ~3);
    for(int j=0; j<i; ++j) {
        std::array<int16_t,4> samples;
        self->oplChip->read(&samples);
        pos[0] = self->filters[0].filter(samples[0] + samples[2]);
        pos[1] = self->filters[1].filter(samples[1] + samples[3]);
        pos += 2;
        
        if( self->interp.next() == 2 ) {
            self->oplChip->read( nullptr ); // skip a sample
        }
        self->interp = 0;
    }
    towrite -= i;
    minicnt -= (long)(self->p->getrefresh() * i);
  }
}
