/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2006 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * mtk.cpp - MPU-401 Trakker Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include "stream/filestream.h"

#include "mtk.h"
#include <stuff/stringutils.h>

/*** public methods **************************************/

Player* MtkPlayer::factory()
{
  return new MtkPlayer();
}

bool MtkPlayer::load(const std::string& filename)
{
  FileStream f( filename );
  if( !f )
  {
    return false;
  }

  // read header
#pragma pack(push, 1)
  struct
  {
    char id[18] = "";
    uint16_t crc = 0, size = 0;
  } header{};
#pragma pack(pop)
  f >> header;

  // file validation section
  if( strncmp( header.id, "mpu401tr\x92kk\xeer@data", 18 ) != 0 )
  {
    return false;
  }

  // load section
  std::vector<uint8_t> cmp( f.size() - 22u );
  std::vector<uint8_t> org( header.size );
  f.read( cmp.data(), cmp.size() );

  size_t orgptr = 0;
  uint16_t ctrlbits = 0, ctrlmask = 0;
  for( auto cmpptr = cmp.begin(); cmpptr < cmp.end(); )
  { // decompress
    ctrlmask >>= 1;
    if( !ctrlmask )
    {
      ctrlbits = *cmpptr | (*(cmpptr + 1) << 8);
      cmpptr += 2;
      ctrlmask = 0x8000;
    }
    if( !(ctrlbits & ctrlmask) )
    { // uncompressed data
      if( orgptr >= header.size )
      {
        return false;
      }

      org[orgptr] = *cmpptr;
      orgptr++;
      ++cmpptr;
      continue;
    }

    // compressed data
    const auto cmd = (*cmpptr >> 4) & 0x0f;
    uint16_t cnt = *cmpptr & 0x0f;
    ++cmpptr;
    uint16_t offs;
    switch( cmd )
    {
    case 0:
      if( orgptr + cnt > header.size )
      {
        return false;
      }
      cnt += 3;
      memset( &org[orgptr], *cmpptr, cnt );
      ++cmpptr;
      orgptr += cnt;
      break;

    case 1:
      if( orgptr + cnt > header.size )
      {
        return false;
      }
      cnt += (*cmpptr << 4) + 19;
      memset( &org[orgptr], *cmpptr, cnt );
      cmpptr += 2;
      orgptr += cnt;
      break;

    case 2:
      if( orgptr + cnt > header.size )
      {
        return false;
      }
      offs = (cnt + 3) + (*cmpptr << 4);
      cnt = *cmpptr + 16;
      cmpptr += 2;
      memcpy( &org[orgptr], &org[orgptr - offs], cnt );
      orgptr += cnt;
      break;

    default:
      if( orgptr + cmd > header.size )
      {
        return false;
      }
      offs = (cnt + 3) + (*cmpptr << 4);
      ++cmpptr;
      memcpy( &org[orgptr], &org[orgptr - offs], cmd );
      orgptr += cmd;
      break;
    }
  }
#pragma pack(push, 1)
  struct mtkdata
  {
    char songname[34], composername[34], instname[0x80][34];
    uint8_t insts[0x80][12], order[0x80], dummy, patterns[0x32][0x40][9];
  };
#pragma pack(pop)
  const auto* data = reinterpret_cast<const mtkdata*>(org.data());

  // convert to HSC replay data
  m_title = stringncpy( data->songname + 1, 33 );
  m_composer = stringncpy( data->composername + 1, 33 );
  for( int i = 0; i < 0x80; i++ )
  {
    m_instrumentNames[i] = stringncpy( data->instname[i] + 1, 33 );
  }
  memcpy( instrumentData(), data->insts, 0x80 * 12 );
  for( auto i: data->order )
  {
    addOrder( i );
  }
  memcpy( patternData(), data->patterns, header.size - 6084u );
  for( int i = 0; i < 128; i++ )
  { // correct instruments
    instrumentData()[i][2] ^= (instrumentData()[i][2] & 0x40) << 1;
    instrumentData()[i][3] ^= (instrumentData()[i][3] & 0x40) << 1;
    instrumentData()[i][11] >>= 4; // make unsigned
  }

  rewind( size_t( 0 ) );
  return true;
}