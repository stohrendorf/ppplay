/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2007 Simon Peter, <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * rad.cpp - RAD Loader by Simon Peter <dn.tlp@gmx.net>
 *
 * BUGS:
 * some volumes are dropped out
 */

#include "stream/filestream.h"

#include "rad.h"

Player* RadPlayer::factory()
{
  return new RadPlayer();
}

bool RadPlayer::load(const std::string& filename)
{
  FileStream f( filename );
  if( !f )
  {
    return false;
  }

  // file validation section
  char id[16];
  f.read( id, 16 );
  uint8_t version;
  f >> version;
  if( strncmp( id, "RAD by REALiTY!!", 16 ) != 0 || version != 0x10 )
  {
    return false;
  }

  // load section
  uint8_t flags;
  f >> flags;
  if( flags & 0x80 )
  { // description
    m_description.clear();
    uint8_t buf;
    while( f >> buf && buf )
    {
      if( buf == 1 )
      {
        m_description += "\n";
      }
      else if( buf >= 2 && buf <= 0x1f )
      {
        m_description.append( buf, ' ' );
      }
      else
      {
        m_description += char( buf );
      }
    }
  }
  {
    uint8_t buf;
    std::vector<ModPlayer::Instrument::Data> instruments;
    while( f >> buf && buf != 0 )
    {
      buf--;
      if( buf >= instruments.size() )
      {
        instruments.resize( buf + 1 );
      }
      ModPlayer::Instrument::Data& inst = instruments[buf];
      for( auto index: { 2, 1, 10, 9, 4, 3, 6, 5, 0, 8, 7 } )
      {
        f >> inst[index];
      }
    }
    for( const ModPlayer::Instrument::Data& inst: instruments )
    {
      addInstrument().data = inst;
    }
  }
  {
    uint8_t length;
    f >> length;
    while( orderCount() < length )
    {
      uint8_t order;
      f >> order;
      addOrder( order );
    }
  }
  uint16_t patofs[32];
  f.read( patofs, 32 );
  init_trackord(); // patterns
  for( auto patternIdx = 0u; patternIdx < 32; patternIdx++ )
  {
    if( patofs[patternIdx] == 0 )
    {
      for( auto j = 0u; j < 9; ++j )
      {
        setCellColumnMapping( patternIdx, j, 0 );
      }
      continue;
    }

    f.seek( patofs[patternIdx] );
    while( true )
    { // for each row
      uint8_t buf;
      f >> buf;
      const uint8_t row = buf & 0x7f;
      while( true )
      {
        uint8_t channelData;
        f >> channelData;
        const uint8_t channel = channelData & 0x7f;

        uint8_t insAndNote;
        f >> insAndNote;
        PatternCell& cell = patternCell( patternIdx * 9 + channel, row );
        cell.note = insAndNote & 127; // 6..4 octave, 3..0 note (1..12)
        cell.instrument = (insAndNote & 0x80) >> 3;

        if( (insAndNote & 0x0f) == 0x0f )
        {
          cell.note = ModPlayer::PatternCell::KeyOff;
        }
        else if( (insAndNote & 0x0f) == 0 )
        {
          cell.note = ModPlayer::PatternCell::NoNote;
        }
        else
        {
          cell.note = ((insAndNote & 0x70) >> 4) * 12 + (insAndNote & 0x0f);
        }

        uint8_t insAndFx;
        f >> insAndFx;
        cell.instrument |= insAndFx >> 4;
        static constexpr Command convfx[16] = { Command::Sentinel,
                                                Command::SlideUp,
                                                Command::SlideDown,
                                                Command::Porta,
                                                Command::Sentinel,
                                                Command::PortaVolSlide,
                                                Command::Sentinel,
                                                Command::Sentinel,
                                                Command::Sentinel,
                                                Command::Sentinel,
                                                Command::RADVolSlide,
                                                Command::Sentinel,
                                                Command::SetFineVolume2,
                                                Command::PatternBreak,
                                                Command::Sentinel,
                                                Command::RADSpeed };
        cell.command = convfx[insAndFx & 0x0f];
        if( cell.command != Command::Sentinel )
        {
          // FX is present, read parameter
          f >> insAndFx;
          insAndFx &= ~0x80;
          cell.hiNybble = insAndFx >> 4;
          cell.loNybble = insAndFx & 0x0f;
        }
        if( channelData & 0x80 )
        { // last column in row
          break;
        }
      }
      if( buf & 0x80 )
      { // last row
        break;
      }
    }
  }

  setRestartOrder( 0 );
  setInitialSpeed( flags & 0x1f );
  setInitialTempo( (flags & 0x40) ? 0 : 50 );

  rewind( size_t( 0 ) );
  return true;
}

size_t RadPlayer::framesUntilUpdate() const
{
  if( currentTempo() != 0 )
  {
    return SampleRate / currentTempo();
  }
  else
  {
    return static_cast<size_t>(SampleRate / 18.2);
  }
}
