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

SDLPlayer::SDLPlayer(Copl *nopl, unsigned char bits, int channels, int freq,
		     unsigned long bufsize)
  : opl(nopl)
{
   memset(&spec, 0x00, sizeof(SDL_AudioSpec));

   if(SDL_Init(SDL_INIT_AUDIO) < 0) {
      message(MSG_ERROR, "unable to initialize SDL -- %s", SDL_GetError());
      exit(EXIT_FAILURE);
   }

   spec.freq = freq;
   if(bits == 16) spec.format = AUDIO_S16SYS; else spec.format = AUDIO_U8;
   spec.channels = channels;
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
  SDL_Delay(1000 * spec.freq / (spec.size / getsampsize()));
}

void SDLPlayer::callback(void *userdata, Uint8 *audiobuf, int len)
{
  SDLPlayer	*self = (SDLPlayer *)userdata;
  static long	minicnt = 0;
  long		i, towrite = len / self->getsampsize();
  char		*pos = (char *)audiobuf;

  // Prepare audiobuf with emulator output
  while(towrite > 0) {
    while(minicnt < 0) {
      minicnt += self->spec.freq;
      self->playing = self->p->update();
    }
    i = std::min(towrite, (long)(minicnt / self->p->getrefresh() + 4) & ~3);
    self->opl->update((short *)pos, i);
    pos += i * self->getsampsize(); towrite -= i;
    minicnt -= (long)(self->p->getrefresh() * i);
  }
}
