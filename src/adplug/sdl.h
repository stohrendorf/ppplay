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

#ifndef H_SDL
#define H_SDL

#include <SDL.h>

#include "output.h"

class SDLPlayer: public Player
{
private:
  opl::Opl3		*oplChip;
  SDL_AudioSpec	spec;

  static void callback(void *, Uint8 *, int);
  ppp::BresenInterpolation interp;
  ppp::OplFilter filters[2];

public:
  SDLPlayer(opl::Opl3 *nopl, int freq, unsigned long bufsize);
  virtual ~SDLPlayer();

  virtual void frame();
  virtual opl::Opl3 *get_opl() { return oplChip; }
};

#endif
