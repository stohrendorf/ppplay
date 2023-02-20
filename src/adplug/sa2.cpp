/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * sa2.cpp - SAdT2 Loader by Simon Peter <dn.tlp@gmx.net>
 *           SAdT Loader by Mamiya <mamiya@users.sourceforge.net>
 */

#include <cstdio>

#include "stream/filestream.h"

#include "sa2.h"
#include <stuff/stringutils.h>

Player* Sa2Player::factory()
{
  return new Sa2Player();
}

bool Sa2Player::load(const std::string& filename)
{
  FileStream f( filename );
  if( !f )
  {
    return false;
  }

#pragma pack(push, 1)
  struct InstrumentData
  {
    uint8_t data[11], arpstart, arpspeed, arppos, arpspdcnt;
  };
#pragma pack(pop)

  const Command convfx[16] = { Command::None,
                               Command::SlideUp,
                               Command::SlideDown,
                               Command::Porta,
                               Command::Vibrato,
                               Command::PortaVolSlide,
                               Command::VibVolSlide,
                               Command::Sentinel,
                               Command::NoteOff,
                               Command::Sentinel,
                               Command::SA2VolSlide,
                               Command::OrderJump,
                               Command::SetFineVolume,
                               Command::PatternBreak,
                               Command::Sentinel,
                               Command::SA2Speed };
  unsigned char sat_type;
  enum SAT_TYPE
  {
    HAS_ARPEGIOLIST = (1 << 7),
    HAS_V7PATTERNS = (1 << 6),
    HAS_ACTIVECHANNELS = (1 << 5),
    HAS_TRACKORDER = (1 << 4),
    HAS_ARPEGIO = (1 << 3),
    HAS_OLDBPM = (1 << 2),
    HAS_OLDPATTERNS = (1 << 1),
    HAS_UNKNOWN127 = (1 << 0)
  };

  // read header
  f.read( m_header.sadt, 4 );
  f >> m_header.version;

  // file validation section
  if( strncmp( m_header.sadt, "SAdT", 4 ) != 0 )
  {
    return false;
  }
  int notedis = 0;
  switch( m_header.version )
  {
  case 1:
    notedis = +0x18;
    sat_type = HAS_UNKNOWN127 | HAS_OLDPATTERNS | HAS_OLDBPM;
    break;
  case 2:
    notedis = +0x18;
    sat_type = HAS_OLDPATTERNS | HAS_OLDBPM;
    break;
  case 3:
    notedis = +0x0c;
    sat_type = HAS_OLDPATTERNS | HAS_OLDBPM;
    break;
  case 4:
    notedis = +0x0c;
    sat_type = HAS_ARPEGIO | HAS_OLDPATTERNS | HAS_OLDBPM;
    break;
  case 5:
    notedis = +0x0c;
    sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_OLDPATTERNS | HAS_OLDBPM;
    break;
  case 6:
    sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_OLDPATTERNS | HAS_OLDBPM;
    break;
  case 7:
    sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_V7PATTERNS;
    break;
  case 8:
    sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_TRACKORDER;
    break;
  case 9:
    sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_TRACKORDER | HAS_ACTIVECHANNELS;
    break;
  default: /* unknown */
    return false;
  }

  // load section
  // instruments
  for( int i = 0; i < 31; i++ )
  {
    InstrumentData sa2Instr;
    ModPlayer::Instrument& inst = addInstrument();
    if( sat_type & HAS_ARPEGIO )
    {
      f >> sa2Instr;

      inst.arpeggioStart = sa2Instr.arpstart;
      inst.arpeggioSpeed = sa2Instr.arpspeed;
    }
    else
    {
      f.read( sa2Instr.data, 11 );
      inst.arpeggioStart = 0;
      inst.arpeggioSpeed = 0;
    }
    std::copy_n( sa2Instr.data, 11, inst.data.begin() );
    inst.misc = 0;
    inst.slide = 0;
  }

  // instrument names
  for( int i = 0; i < 29; i++ )
  {
    char tmp[17];
    f.read( tmp, 17 );
    m_instrumentNames[i] = stringncpy( tmp, 17 );
  }

  f.seekrel( 3 ); // dummy bytes

  {
    uint8_t orderData[128];
    f.read( orderData, 128 );
    if( sat_type & HAS_UNKNOWN127 )
    {
      f.seekrel( 127 );
    }
    //f >> m_maxUsedPattern;
    f.seekrel( 2 );

    uint8_t orderCount;
    f >> orderCount;
    if( orderCount > 128 )
    {
      orderCount = 128;
    }
    for( uint8_t i = 0; i < orderCount; ++i )
    {
      addOrder( orderData[i] );
    }

    uint8_t restartPos;
    f >> restartPos;
    setRestartOrder( restartPos );
  }

  // bpm
  uint16_t initialTempo;
  f >> initialTempo;
  if( sat_type & HAS_OLDBPM )
  {
    setInitialTempo( initialTempo * 125u / 50u );
  }
  else
  {
    setInitialTempo( initialTempo );
  }

  if( sat_type & HAS_ARPEGIOLIST )
  {
    ArpeggioData data;
    f.read( data.data(), 256 );
    setArpeggioList( data );
    f.read( data.data(), 256 );
    setArpeggioCommands( data );
  }

  for( uint32_t i = 0; i < 64; i++ )
  { // track orders
    for( uint32_t j = 0; j < 9; j++ )
    {
      if( sat_type & HAS_TRACKORDER )
      {
        uint8_t tmp;
        f >> tmp;
        setCellColumnMapping( i, j, tmp );
      }
      else
      {
        setCellColumnMapping( i, j, i * 9u + j );
      }
    }
  }

  if( sat_type & HAS_ACTIVECHANNELS )
  {
    disableAllChannels();
    uint16_t tmp;
    f >> tmp;
    for( uint8_t i = 0; i < 16; ++i )
    {
      if( tmp & (0x8000 >> i) )
      {
        enableChannel( i );
      }
    }
  }

  // track data
  if( sat_type & HAS_OLDPATTERNS )
  {
    int i = 0;
    while( f.pos() < f.size() )
    {
      for( size_t j = 0; j < 64; j++ )
      {
        for( size_t k = 0; k < 9; k++ )
        {
          uint8_t buf;
          f >> buf;
          PatternCell& cell = patternCell( i + k, j );
          cell.note = buf ? (buf + notedis) : 0;
          f >> buf;
          cell.instrument = buf;
          f >> buf;
          cell.command = convfx[buf & 0xf];
          f >> buf;
          cell.hiNybble = buf;
          f >> buf;
          cell.loNybble = buf;
        }
      }
      i += 9;
    }
  }
  else if( sat_type & HAS_V7PATTERNS )
  {
    int i = 0;
    while( f.pos() < f.size() )
    {
      for( size_t j = 0; j < 64; j++ )
      {
        for( size_t k = 0; k < 9; k++ )
        {
          uint8_t buf;
          f >> buf;
          PatternCell& cell = patternCell( i + k, j );
          cell.note = buf >> 1;
          cell.instrument = (buf & 1) << 4;
          f >> buf;
          cell.instrument |= buf >> 4;
          cell.command = convfx[buf & 0x0f];
          f >> buf;
          cell.hiNybble = buf >> 4;
          cell.loNybble = buf & 0x0f;
        }
      }
      i += 9;
    }
  }
  else
  {
    size_t i = 0;
    while( f && f.pos() < f.size() )
    {
      for( size_t j = 0; j < 64; j++ )
      {
        uint8_t buf;
        f >> buf;
        PatternCell& cell = patternCell( i, j );
        cell.note = buf >> 1;
        cell.instrument = (buf & 1) << 4;
        f >> buf;
        cell.instrument |= buf >> 4;
        cell.command = convfx[buf & 0x0f];
        f >> buf;
        cell.hiNybble = buf >> 4;
        cell.loNybble = buf & 0x0f;
      }
      i++;
    }
  }

  // fix instrument names
  for( int i = 0; i < 29; i++ )
  {
    for( int j = 0; j < 17; j++ )
    {
      if( !m_instrumentNames[i][j] )
      {
        m_instrumentNames[i][j] = ' ';
      }
    }
  }

  rewind( size_t( 0 ) ); // rewind module
  return true;
}

std::string Sa2Player::type() const
{
  return stringFmt( "Surprise! Adlib Tracker 2 (version %d)", int( m_header.version ) );
}

std::string Sa2Player::title() const
{
  std::string bufInst;

  // parse instrument names for song name
  for( int i = 0; i < 29; i++ )
  {
    std::string tmp = m_instrumentNames[i];
    if( !tmp.empty() )
    {
      tmp = tmp.substr( 1 );
    }

    while( !tmp.empty() && tmp.back() == ' ' )
    {
      tmp.pop_back();
    }

    if( !tmp.empty() && tmp.length() < 16 )
    {
      tmp.back() += ' ';
    }
    bufInst += tmp;
  }

  if( bufInst.find( '"' ) != std::string::npos )
  {
    return bufInst.substr( bufInst.find( '"' ) + 1 );
  }
  else
  {
    return std::string();
  }
}
