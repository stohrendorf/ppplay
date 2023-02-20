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

#include "output.h"

/***** EmuPlayer *****/

EmuPlayer::EmuPlayer(unsigned long nfreq, size_t nbufsize)
  : PlayerHandler()
  , m_audioBuf( nbufsize * 2, 0 )
  , m_freq( nfreq )
  , m_oplInterp( opl::Opl3::SampleRate, m_freq )
{
}

void EmuPlayer::frame()
{
  static long framesUntilUpdate = 0;
  size_t towrite = m_audioBuf.size() / 2;
  int16_t* pos = m_audioBuf.data();

  // Prepare audiobuf with emulator output
  while( towrite > 0 )
  {
    while( framesUntilUpdate < 0 )
    {
      setIsPlaying( getPlayer()->update() );
      framesUntilUpdate += getPlayer()->framesUntilUpdate();
    }

    std::array<int16_t, 4> samples;
    getPlayer()->read( &samples );
    pos[0] = samples[0] + samples[1];
    pos[1] = samples[2] + samples[3];
    pos += 2;

    if( m_oplInterp.next() == 2 )
    {
      getPlayer()->read( nullptr ); // skip a sample
      --framesUntilUpdate;
    }
    m_oplInterp = 0;
    --framesUntilUpdate;
    --towrite;
  }

  // call output driver
  output( m_audioBuf );
}