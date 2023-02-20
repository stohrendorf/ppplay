/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2002 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * sng.cpp - SNG Player by Simon Peter <dn.tlp@gmx.net>
 */

#include "stream/filestream.h"

#include "sng.h"

Player* SngPlayer::factory()
{
  return new SngPlayer();
}

bool SngPlayer::load(const std::string& filename)
{
  FileStream f( filename );
  if( !f )
  {
    return false;
  }

  // load header
  f >> m_header;

  // file validation section
  if( strncmp( m_header.id, "ObsM", 4 ) != 0 )
  {
    return false;
  }

  // load section
  m_header.length /= 2;
  m_header.start /= 2;
  m_header.loop /= 2;
  m_data.resize( m_header.length );
  f.read( m_data.data(), m_data.size() );

  rewind( size_t( 0 ) );
  return true;
}

bool SngPlayer::update()
{
  if( m_header.compressed && m_del )
  {
    m_del--;
    return !m_songEnd;
  }

  while( m_data[m_pos].reg != 0 )
  {
    getOpl()->writeReg( m_data[m_pos].reg, m_data[m_pos].val );
    m_pos++;
    if( m_pos >= m_header.length )
    {
      m_songEnd = true;
      m_pos = m_header.loop;
    }
  }

  if( !m_header.compressed )
  {
    getOpl()->writeReg( m_data[m_pos].reg, m_data[m_pos].val );
  }

  if( m_data[m_pos].val != 0 )
  {
    m_del = m_data[m_pos].val - 1u;
  }
  m_pos++;
  if( m_pos >= m_header.length )
  {
    m_songEnd = true;
    m_pos = m_header.loop;
  }
  return !m_songEnd;
}

void SngPlayer::rewind(const boost::optional<size_t>&)
{
  m_pos = m_header.start;
  m_del = m_header.delay;
  m_songEnd = false;
  getOpl()->writeReg( 1, 32 ); // go to OPL2 mode
}