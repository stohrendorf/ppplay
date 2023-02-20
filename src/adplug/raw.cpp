/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2005 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * raw.c - RAW Player by Simon Peter <dn.tlp@gmx.net>
 */

#include "stream/filestream.h"

#include "raw.h"

/*** public methods *************************************/

Player* RawPlayer::factory()
{
  return new RawPlayer();
}

bool RawPlayer::load(const std::string& filename)
{
  FileStream f( filename );
  if( !f )
  {
    return false;
  }

  // file validation section
  char id[8];
  f.read( id, 8 );
  if( !std::equal( id, id + 8, "RAWADATA" ) )
  {
    return false;
  }

  // load section
  uint16_t clock;
  f >> clock;
  setInitialSpeed( clock );
  setCurrentSpeed( clock );

  BOOST_ASSERT( f.pos() == 10 );
  static_assert( sizeof( TrackData ) == 2, "Ooops" );
  m_data.resize( (f.size() - 10u) / 2u );
  f.read( m_data.data(), m_data.size() );

  addOrder( 0 );

  rewind( size_t( 0 ) );
  return true;
}

bool RawPlayer::update()
{
  if( m_dataPosition >= m_data.size() )
  {
    return false;
  }

  if( m_delay > 0 )
  {
    --m_delay;
    return !m_songend;
  }

  while( m_dataPosition < m_data.size() )
  {
    const auto& d = m_data[m_dataPosition++];
    bool setspeed = false;
    switch( d.command )
    {
    case 0:
      BOOST_ASSERT( d.param > 0 );
      m_delay = d.param - 1;
      break;
    case 2:
      if( d.param != 0 )
      {
        setCurrentSpeed( d.param + (d.command * 256) );
        setspeed = true;
      }
      else
      { ; //FIXME sto opl->setchip(data[pos].param - 1);
      }
      break;
    case 0xff:
      if( d.param == 0xff )
      {
        rewind( size_t( 0 ) ); // auto-rewind song
        m_songend = true;
        return !m_songend;
      }
      break;
    default:
      getOpl()->writeReg( d.command, d.param );
      break;
    }
    if( !setspeed && d.command == 0 )
    {
      break;
    }
  }

  return !m_songend;
}

void RawPlayer::rewind(const boost::optional<size_t>&)
{
  m_dataPosition = m_delay = 0;
  setCurrentSpeed( initialSpeed() );
  m_songend = false;
  getOpl()->writeReg( 1, 32 ); // go to 9 channel mode
}

size_t RawPlayer::framesUntilUpdate() const
{
  return SampleRate * (currentSpeed() ? currentSpeed() : 0xffff) /
    1193180; // timer oscillator speed / wait register = clock
  // frequency
}
