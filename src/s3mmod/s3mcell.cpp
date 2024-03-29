/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @ingroup S3mMod
 * @{
 */

#include "s3mcell.h"

#include "genmod/genbase.h"
#include "s3mbase.h"

#include "stream/abstractarchive.h"

#include <boost/exception/all.hpp>
#include <boost/format.hpp>

namespace ppp
{
namespace s3m
{
S3mCell::S3mCell()
  : IPatternCell(), m_note( s3mEmptyNote ), m_instr( s3mEmptyInstr ), m_volume( s3mEmptyVolume ), m_effect(
  s3mEmptyCommand ), m_effectValue( 0x00 )
{
}

bool S3mCell::load(Stream* str)
{
  try
  {
    clear();
    uint8_t master = 0;
    uint8_t buf;
    *str >> master;
    if( master & 0x20 )
    {
      *str >> buf;
      m_note = buf;
      if( (m_note >= 0x9b) && (m_note != s3mEmptyNote) && (m_note != s3mKeyOffNote) )
      {
        logger()->warn( L4CXX_LOCATION, "File Position %#x: Note out of range: %d", str->pos(), int( m_note ) );
        m_note = s3mEmptyNote;
      }
      *str >> buf;
      m_instr = buf;
    }
    if( master & 0x40 )
    {
      *str >> buf;
      m_volume = buf;
      if( buf > 0x40 )
      {
        logger()->warn( L4CXX_LOCATION, "File Position %#x: Volume out of range: %d", str->pos(), int( m_volume ) );
        m_volume = s3mEmptyVolume;
      }
    }
    if( master & 0x80 )
    {
      *str >> buf;
      m_effect = buf;
      *str >> buf;
      m_effectValue = buf;
    }
  }
  catch( ... )
  {
    BOOST_THROW_EXCEPTION( std::runtime_error( "Exception" ) );
  }
  return str->good();
}

void S3mCell::clear()
{
  m_note = s3mEmptyNote;
  m_instr = s3mEmptyInstr;
  m_volume = s3mEmptyVolume;
  m_effect = s3mEmptyCommand;
  m_effectValue = 0x00;
}

std::string S3mCell::trackerString() const
{
  std::string xmsg;
  if( m_note == s3mEmptyNote )
  {
    xmsg += "... ";
  }
  else if( m_note == s3mKeyOffNote )
  {
    xmsg += "^^  ";
  }
  else
  {
    xmsg += stringFmt( "%s%d ", NoteNames.at( m_note & 0x0f ), int( m_note >> 4 ) );
  }

  if( m_instr != s3mEmptyInstr )
  {
    xmsg += stringFmt( "%02d ", int( m_instr ) );
  }
  else
  {
    xmsg += ".. ";
  }

  if( m_volume != s3mEmptyVolume )
  {
    xmsg += stringFmt( "%02d ", int( m_volume ) );
  }
  else
  {
    xmsg += ".. ";
  }

  if( m_effect != s3mEmptyCommand )
  {
    xmsg += stringFmt( "%c%02x", char( 'A' - 1 + m_effect ), int( m_effectValue ) );
  }
  else
  {
    xmsg += "...";
  }
  return xmsg;
}

uint8_t S3mCell::note() const
{
  return m_note;
}

uint8_t S3mCell::instrument() const
{
  return m_instr;
}

uint8_t S3mCell::volume() const
{
  return m_volume;
}

uint8_t S3mCell::effect() const
{
  return m_effect;
}

uint8_t S3mCell::effectValue() const
{
  return m_effectValue;
}

AbstractArchive& S3mCell::serialize(AbstractArchive* data)
{
  *data
    % m_note
    % m_instr
    % m_volume
    % m_effect
    % m_effectValue;
  return *data;
}

light4cxx::Logger* S3mCell::logger()
{
  return light4cxx::Logger::get( IPatternCell::logger()->name() + ".s3m" );
}
}
}

/**
 * @}
 */