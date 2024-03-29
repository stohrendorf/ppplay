/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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
 * @ingroup XmModule
 * @{
 */

#include "xmsample.h"
#include "stream/stream.h"

namespace ppp
{
namespace xm
{
XmSample::XmSample()
  : m_finetune( 0 ), m_panning( 0x80 ), m_relativeNote( 0 ), m_16bit( false )
{
}

bool XmSample::load(Stream* str)
{
  int32_t dataSize;
  *str >> dataSize;
  int32_t loopStart, loopLen;
  *str >> loopStart >> loopLen;
  uint8_t volume;
  *str >> volume;
  setVolume( clip<int>( volume, 0, 64 ) );
  *str >> m_finetune;
  uint8_t type;
  *str >> type;
  if( loopStart + loopLen > dataSize )
  {
    loopStart = loopLen = 0;
    m_loopType = LoopType::None;
  }
  else
  {
    switch( type & 3 )
    {
    case 0:
      m_loopType = LoopType::None;
      break;
    case 1:
      m_loopType = LoopType::Forward;
      break;
    case 2:
    case 3:
      m_loopType = LoopType::Pingpong;
      break;
    }
  }
  m_16bit = (type & 0x10) != 0;
  if( m_16bit )
  {
    resizeData( dataSize / 2u );
    m_loopStart = loopStart / 2u;
    m_loopEnd = (loopStart + loopLen) / 2u;
  }
  else
  {
    resizeData( dataSize );
    m_loopStart = loopStart;
    m_loopEnd = loopStart + loopLen;
  }
  *str >> m_panning >> m_relativeNote;
  str->seekrel( 1 );
  {
    char title[22];
    str->read( title, 22 );
    setTitle( stringncpy( title, 22 ) );
  }
  return str->good();
}

bool XmSample::loadData(Stream* str)
{
  if( length() == 0 )
    return true;
  if( m_16bit )
  { // 16 bit
    int16_t smp16 = 0;
    for( auto it = beginIterator(); it != endIterator(); ++it )
    {
      int16_t delta;
      *str >> delta;
      smp16 += delta;
      it->left = it->right = smp16;
    }
  }
  else
  { // 8 bit
    int8_t smp8 = 0;
    for( auto it = beginIterator(); it != endIterator(); ++it )
    {
      int8_t delta;
      *str >> delta;
      smp8 += delta;
      it->left = it->right = smp8 << 8;
    }
  }
  return str->good();
}

uint8_t XmSample::panning() const
{
  return m_panning;
}

bool XmSample::is16bit() const
{
  return m_16bit;
}

int8_t XmSample::relativeNote() const
{
  return m_relativeNote;
}

int8_t XmSample::finetune() const
{
  return m_finetune;
}
}
}

/**
 * @}
 */