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

#include <stdio.h>
#include <adplug/emuopl.h>
#include <adplug/kemuopl.h>

#include "output.h"
#include "defines.h"

/***** Player *****/

Player::Player()
  : p(0), playing(false)
{
}

Player::~Player()
{
  if(p) delete p;
}

/***** EmuPlayer *****/

EmuPlayer::EmuPlayer(Copl *nopl, unsigned char nbits, unsigned char nchannels,
		     unsigned long nfreq, unsigned long nbufsize)
  : opl(nopl), buf_size(nbufsize), freq(nfreq), bits(nbits), channels(nchannels)
{
  audiobuf = new char [buf_size * getsampsize()];
}

EmuPlayer::~EmuPlayer()
{
  delete [] audiobuf;
}

// Some output plugins (ALSA) need to change the buffer size mid-init
void EmuPlayer::setbufsize(unsigned long nbufsize)
{
  delete [] audiobuf;
  buf_size = nbufsize;
  audiobuf = new char [buf_size * getsampsize()];
}

void EmuPlayer::frame()
{
  static long minicnt = 0;
  long i, towrite = buf_size;
  char *pos = audiobuf;

  // Prepare audiobuf with emulator output
  while(towrite > 0) {
    while(minicnt < 0) {
      minicnt += freq;
      playing = p->update();
    }
    i = std::min(towrite, (long)(minicnt / p->getrefresh() + 4) & ~3);
    opl->update((short *)pos, i);
    pos += i * getsampsize(); towrite -= i;
    minicnt -= (long)(p->getrefresh() * i);
  }

  // call output driver
  output(audiobuf, buf_size * getsampsize());
}
