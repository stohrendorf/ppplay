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

#include "output.h"
#include "defines.h"
#include "genmod/breseninter.h"

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

EmuPlayer::EmuPlayer(opl::Opl3 *nopl, unsigned long nfreq, unsigned long nbufsize)
  : opl(nopl), buf_size(nbufsize), freq(nfreq)
{
  audiobuf = new char [buf_size * 4];
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
  audiobuf = new char [buf_size * 4];
}

void EmuPlayer::frame()
{
  static long minicnt = 0;
  long i, towrite = buf_size;
  int16_t *pos = reinterpret_cast<int16_t*>(audiobuf);

  ppp::BresenInterpolation interp( freq, opl::Opl3::SampleRate );

  // Prepare audiobuf with emulator output
  while(towrite > 0) {
    while(minicnt < 0) {
      minicnt += freq;
      playing = p->update();
    }
    i = std::min(towrite, (long)(minicnt / p->getrefresh() + 4) & ~3);
    for(int j=0; j<i; ++j) {
        std::array<int16_t,4> samples;
        opl->read(&samples);
        if( interp.next() == 2 ) {
            opl->read( nullptr ); // skip a sample
        }
        interp = 0;
        pos[0] = samples[0];
        pos[1] = samples[1];
        pos[0] += samples[2];
        pos[1] += samples[3];
        pos += 2;
    }
    towrite -= i;
    minicnt -= (long)(p->getrefresh() * i);
  }

  // call output driver
  output(audiobuf, buf_size * 4);
}
