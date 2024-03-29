/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  mad.cpp - MAD loader by Riven the Mage <riven@ok.ru>
*/

#include "stream/filestream.h"

#include "mad.h"

/* -------- Public Methods -------------------------------- */

Player* MadPlayer::factory()
{
  return new MadPlayer();
}

bool MadPlayer::load(const std::string& filename)
{
  FileStream f( filename );
  if( !f )
  {
    return false;
  }
  const unsigned char conv_inst[10] = { 2, 1, 10, 9, 4, 3, 6, 5, 8, 7 };
  // 'MAD+' - signed ?
  char id[4];
  f.read( id, 4 );
  if( strncmp( id, "MAD+", 4 ) != 0 )
  {
    return false;
  }

  // load instruments
  for( auto& instrument: instruments )
  {
    f.read( instrument.name, 8 );
    f.read( instrument.data, 12 );
  }

  f.seekrel( 1 );

  // data for Protracker
  uint8_t orderCount;
  f >> orderCount;
  uint8_t maxUsedPattern;
  f >> maxUsedPattern;
  uint8_t tmp;
  f >> tmp;
  setCurrentSpeed( tmp );
  setInitialSpeed( tmp );

  // init CmodPlayer
  realloc_patterns( maxUsedPattern, 32, 9 );
  init_trackord();

  // load tracks
  for( auto i = 0; i < maxUsedPattern; i++ )
  {
    for( auto k = 0u; k < 32; k++ )
    {
      for( auto j = 0u; j < 9; j++ )
      {
        auto t = i * 9 + j;

        // read event
        uint8_t event;
        f >> event;

        // convert event
        PatternCell& cell = patternCell( t, k );
        if( event < 0x61 )
        {
          cell.note = event;
        }
        if( event == 0xFF )
        { // 0xFF: Release note
          cell.command = Command::NoteOff;
        }
        if( event == 0xFE )
        { // 0xFE: Pattern Break
          cell.command = Command::PatternBreak;
        }
      }
    }
  }

  // load order
  for( uint8_t order; this->orderCount() < orderCount && f >> order; )
  {
    addOrder( order - 1u );
  }

  // convert instruments
  for( auto& instrument: instruments )
  {
    ModPlayer::Instrument& inst = addInstrument();
    for( auto j = 0; j < 10; j++ )
    {
      inst.data[conv_inst[j]] = instrument.data[j];
    }
  }

  // data for Protracker
  setRestartOrder( 0 );

  rewind( size_t( 0 ) );
  return true;
}

void MadPlayer::rewind(const boost::optional<size_t>& subsong)
{
  ModPlayer::rewind( subsong );

  // default instruments
  for( auto i = 0u; i < 9; i++ )
  {
    channel( i ).instrument = i;

    const ModPlayer::Instrument& inst = instrument( i );
    channel( i ).carrierVolume = 63 - (inst.data[10] & 63);
    channel( i ).modulatorVolume = 63 - (inst.data[9] & 63);
  }
}

size_t MadPlayer::framesUntilUpdate() const
{
  return SampleRate / currentSpeed() / currentSpeed();
}

std::string MadPlayer::type() const
{
  return "Mlat Adlib Tracker";
}

std::string MadPlayer::instrumentTitle(size_t n) const
{
  return std::string( instruments[n].name, 8 );
}

size_t MadPlayer::instrumentCount() const
{
  return 9;
}
