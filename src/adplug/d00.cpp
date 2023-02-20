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
 * d00.c - D00 Player by Simon Peter <dn.tlp@gmx.net>
 *
 * NOTES:
 * Sorry for the goto's, but the code looks so much nicer now.
 * I tried it with while loops but it was just a mess. If you
 * can come up with a nicer solution, just tell me.
 *
 * BUGS:
 * Hard restart SR is sometimes wrong
 */

#include <cstdio>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "stream/filestream.h"

#include "d00.h"

template<typename T>
constexpr uint8_t HIBYTE(T val)
{
  return static_cast<uint8_t>(val >> 8);
}

template<typename T>
constexpr uint8_t LOBYTE(T val)
{
  return static_cast<uint8_t>(val & 0xff);
}

static const unsigned short notetable[12] = // D00 note table
  { 340, 363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647 };

static inline uint16_t LE_WORD(const uint16_t* val)
{
  const auto* b = reinterpret_cast<const uint8_t*>(val);
  return (b[1] << 8) + b[0];
}

/*** public methods *************************************/

Player* D00Player::factory()
{
  return new D00Player();
}

bool D00Player::load(const std::string& filename)
{
  FileStream f( filename );
  if( !f )
  {
    return false;
  }

  // file validation section
  d00header checkhead;
  f >> checkhead;

  bool ver1 = false;
  // Check for version 2-4 header
  if( strncmp( checkhead.id, "JCH\x26\x02\x66", 6 ) != 0 || checkhead.type || !checkhead.subsongs
    || checkhead.soundcard )
  {
    // Check for version 0 or 1 header (and .d00 file extension)
    if( f.extension() != ".d00" )
    {
      return false;
    }
    f.seek( 0 );
    d00header1 ch;
    f >> ch;
    if( ch.version > 1 || !ch.subsongs )
    {
      return false;
    }
    ver1 = true;
  }

  // load section
  auto filesize = f.size();
  f.seek( 0 );
  m_fileData.resize( filesize + 1 ); // 1 byte is needed for old-style DataInfo block
  f.read( m_fileData.data(), filesize );
  if( !ver1 )
  { // version 2 and above
    m_header = reinterpret_cast<d00header*>(m_fileData.data());
    m_version = m_header->version;
    m_description = m_fileData.data() + LE_WORD( &m_header->infoptr );
    m_instruments = reinterpret_cast<const Instrument*>(m_fileData.data() + LE_WORD( &m_header->instrumentOfs ));
    m_orders = reinterpret_cast<const uint16_t*>(m_fileData.data() + LE_WORD( &m_header->orderListOfs ));
    for( int i = 31; i >= 0; i-- )
    { // erase whitespace
      if( m_header->songname[i] == ' ' )
      {
        m_header->songname[i] = '\0';
      }
      else
      {
        break;
      }
    }
    for( int i = 31; i >= 0; i-- )
    {
      if( m_header->author[i] == ' ' )
      {
        m_header->author[i] = '\0';
      }
      else
      {
        break;
      }
    }
  }
  else
  { // version 1
    m_header1 = reinterpret_cast<d00header1*>(m_fileData.data());
    m_version = m_header1->version;
    m_description = m_fileData.data() + LE_WORD( &m_header1->infoptr );
    m_instruments = reinterpret_cast<const Instrument*>(m_fileData.data() + LE_WORD( &m_header1->instrumentOfs ));
    m_orders = reinterpret_cast<const uint16_t*>(m_fileData.data() + LE_WORD( &m_header1->orderListOfs ));
  }
  switch( m_version )
  {
  case 0:
    m_levPuls = nullptr;
    m_spfx = nullptr;
    m_header1->speed = 70; // v0 files default to 70Hz
    break;
  case 1:
    m_levPuls = reinterpret_cast<const Slevpuls*>(m_fileData.data() + LE_WORD( &m_header1->lpulptr ));
    m_spfx = nullptr;
    break;
  case 2:
    m_levPuls = reinterpret_cast<const Slevpuls*>(m_fileData.data() + LE_WORD( &m_header->spfxptr ));
    m_spfx = nullptr;
    break;
  case 3:
    m_spfx = nullptr;
    m_levPuls = nullptr;
    break;
  case 4:
    m_spfx = reinterpret_cast<const Sspfx*>(m_fileData.data() + LE_WORD( &m_header->spfxptr ));
    m_levPuls = nullptr;
    break;
  default:
    BOOST_THROW_EXCEPTION( std::runtime_error( "Unsupported D00 format" ) );
  }
  if( auto str = strstr( m_description, "\xff\xff" ) )
  {
    while( (*str == '\xff' || *str == ' ') && str >= m_description )
    {
      *str = '\0';
      str--;
    }
  }
  else
  { // old-style block
    m_fileData.back() = 0;
  }

  addOrder( 0 );

  rewind( size_t( 0 ) );
  return true;
}

bool D00Player::update()
{
  // effect handling (timer dependant)
  for( uint8_t c = 0; c < 9; c++ )
  {
    m_channels[c].noteSlideValue += m_channels[c].noteSlideSpeed;
    setfreq( c ); // sliding
    vibrato( c ); // vibrato

    if( m_channels[c].spfx != 0xffff )
    { // SpFX
      if( m_channels[c].fxDelay )
      {
        m_channels[c].fxDelay--;
      }
      else
      {
        m_channels[c].spfx = LE_WORD( &m_spfx[m_channels[c].spfx].ptr );
        m_channels[c].fxDelay = m_spfx[m_channels[c].spfx].duration;
        m_channels[c].instrument = LE_WORD( &m_spfx[m_channels[c].spfx].instnr ) & 0xfff;
        if( m_spfx[m_channels[c].spfx].modlev != 0xff )
        {
          m_channels[c].modulatorVolume = m_spfx[m_channels[c].spfx].modlev;
        }
        setinst( c );
        uint8_t note;
        if( LE_WORD( &m_spfx[m_channels[c].spfx].instnr ) & 0x8000 )
        { // locked frequency
          note = m_spfx[m_channels[c].spfx].halfnote;
        }
        else
        { // unlocked frequency
          note = m_spfx[m_channels[c].spfx].halfnote + m_channels[c].note;
        }
        m_channels[c].frequency = notetable[note % 12] + ((note / 12) << 10);
        setfreq( c );
      }
      m_channels[c].modulatorVolume += m_spfx[m_channels[c].spfx].modlevadd;
      m_channels[c].modulatorVolume &= 63;
      setvolume( c );
    }

    if( m_channels[c].levpuls != 0xff )
    { // Levelpuls
      if( m_channels[c].frameskip )
      {
        m_channels[c].frameskip--;
      }
      else
      {
        m_channels[c].frameskip = m_instruments[m_channels[c].instrument].timer;
        if( m_channels[c].fxDelay )
        {
          m_channels[c].fxDelay--;
        }
        else
        {
          BOOST_ASSERT( m_levPuls != nullptr );
          m_channels[c].levpuls = m_levPuls[m_channels[c].levpuls].ptr - 1;
          m_channels[c].fxDelay = m_levPuls[m_channels[c].levpuls].duration;
          if( m_levPuls[m_channels[c].levpuls].level != 0xff )
          {
            m_channels[c].modulatorVolume = m_levPuls[m_channels[c].levpuls].level;
          }
        }
        m_channels[c].modulatorVolume += m_levPuls[m_channels[c].levpuls].voladd;
        m_channels[c].modulatorVolume &= 63;
        setvolume( c );
      }
    }
  }

  // song handling
  for( uint8_t c = 0; c < 9; c++ )
  {
    if( m_version < 3 ? (m_channels[c].delay != 0) : (m_channels[c].delay <= 0x7f) )
    {
      if( m_version == 4 ) // v4: hard restart SR
      {
        if( m_channels[c].delay == m_instruments[m_channels[c].instrument].timer )
        {
          if( m_channels[c].nextNote )
          {
            getOpl()->writeReg( 0x83 + s_opTable[c], m_instruments[m_channels[c].instrument].sr );
          }
        }
      }
      if( m_version < 3 )
      {
        m_channels[c].delay--;
      }
      else if( m_channels[c].speed != 0 )
      {
        m_channels[c].delay += m_channels[c].speed;
      }
      else
      {
        m_channels[c].seqend = true;
      }
      continue;
    }

    if( m_channels[c].speed != 0 )
    {
      if( m_version < 3 )
      {
        m_channels[c].delay = m_channels[c].speed;
      }
      else
      {
        m_channels[c].delay &= 0x7f;
        m_channels[c].delay += m_channels[c].speed;
      }
    }
    else
    {
      m_channels[c].seqend = true;
      continue;
    }

    if( m_channels[c].restHoldDelay )
    { // process pending REST/HOLD events
      m_channels[c].restHoldDelay--;
      continue;
    }
readorder: // process arrangement (orderlist)
    auto ord = LE_WORD( &m_channels[c].patternData[m_channels[c].orderPos] );
    const uint16_t* patt = nullptr;
    switch( ord )
    {
    case 0xfffe:
      m_channels[c].seqend = true;
      continue; // end of arrangement stream
    case 0xffff: // jump to order
      m_channels[c].orderPos = LE_WORD( &m_channels[c].patternData[m_channels[c].orderPos + 1] );
      m_channels[c].seqend = true;
      goto readorder;
    default:
      if( ord >= 0x9000 )
      { // set speed
        m_channels[c].speed = ord & 0xff;
        ord = LE_WORD( &m_channels[c].patternData[m_channels[c].orderPos - 1] );
        m_channels[c].orderPos++;
      }
      else if( ord >= 0x8000 )
      { // transpose track
        m_channels[c].transpose = ord & 0xff;
        if( ord & 0x100 )
        {
          m_channels[c].transpose = -m_channels[c].transpose;
        }
        ord = LE_WORD( &m_channels[c].patternData[++m_channels[c].orderPos] );
      }
      patt = reinterpret_cast<const uint16_t*>(m_fileData.data() + LE_WORD( &m_orders[ord] ));
      break;
    }
    m_channels[c].fxflag = 0;
readseq: // process sequence (pattern)
    if( !m_version )
    { // v0: always initialize restHoldDelay
      m_channels[c].restHoldDelay = m_channels[c].irhcnt;
    }
    auto pattpos = LE_WORD( &patt[m_channels[c].patternPos] );
    if( pattpos == 0xffff )
    { // pattern ended?
      m_channels[c].patternPos = 0;
      m_channels[c].orderPos++;
      goto readorder;
    }
    auto cnt = HIBYTE( pattpos );
    auto note = LOBYTE( pattpos );
    const auto fx = pattpos >> 12;
    const uint16_t fxop = pattpos & 0x0fff;
    m_channels[c].patternPos++;
    pattpos = LE_WORD( &patt[m_channels[c].patternPos] );
    m_channels[c].nextNote = LOBYTE( pattpos ) & 0x7f;
    if( m_version ? cnt < 0x40 : !fx )
    { // note event
      switch( note )
      {
      case 0: // REST event
      case 0x80:
        if( !note || m_version )
        {
          m_channels[c].keyOn = false;
          setfreq( c );
        }
        // fall through...
      case 0x7e: // HOLD event
        if( m_version )
        {
          m_channels[c].restHoldDelay = cnt;
        }
        m_channels[c].nextNote = 0;
        break;
      default: // play note
        // restart fx
        if( !(m_channels[c].fxflag & 1) )
        {
          m_channels[c].vibratoDepth = 0;
        }
        if( !(m_channels[c].fxflag & 2) )
        {
          m_channels[c].noteSlideValue = m_channels[c].noteSlideSpeed = 0;
        }

        if( m_version )
        { // note handling for v1 and above
          if( note > 0x80 )
          { // locked note (no channel transpose)
            note -= 0x80;
          }
          else
          { // unlocked note
            note += m_channels[c].transpose;
          }
          m_channels[c].note = note; // remember note for SpFX

          if( m_channels[c].ispfx != 0xffff && cnt < 0x20 )
          { // reset SpFX
            m_channels[c].spfx = m_channels[c].ispfx;
            if( LE_WORD( &m_spfx[m_channels[c].spfx].instnr ) & 0x8000 )
            { // locked frequency
              note = m_spfx[m_channels[c].spfx].halfnote;
            }
            else
            { // unlocked frequency
              note += m_spfx[m_channels[c].spfx].halfnote;
            }
            m_channels[c].instrument = LE_WORD( &m_spfx[m_channels[c].spfx].instnr ) & 0xfff;
            m_channels[c].fxDelay = m_spfx[m_channels[c].spfx].duration;
            if( m_spfx[m_channels[c].spfx].modlev != 0xff )
            {
              m_channels[c].modulatorVolume = m_spfx[m_channels[c].spfx].modlev;
            }
            else
            {
              m_channels[c].modulatorVolume = m_instruments[m_channels[c].instrument].data[7] & 63;
            }
          }

          if( m_channels[c].ilevpuls != 0xff && cnt < 0x20 )
          { // reset LevelPuls
            m_channels[c].levpuls = m_channels[c].ilevpuls;
            m_channels[c].fxDelay = m_levPuls[m_channels[c].levpuls].duration;
            m_channels[c].frameskip = m_instruments[m_channels[c].instrument].timer;
            if( m_levPuls[m_channels[c].levpuls].level != 0xff )
            {
              m_channels[c].modulatorVolume = m_levPuls[m_channels[c].levpuls].level;
            }
            else
            {
              m_channels[c].modulatorVolume = m_instruments[m_channels[c].instrument].data[7] & 63;
            }
          }

          m_channels[c].frequency = notetable[note % 12] + ((note / 12) << 10);
          if( cnt < 0x20 )
          { // normal note
            playnote( c );
          }
          else
          { // tienote
            setfreq( c );
            cnt -= 0x20; // make count proper
          }
          m_channels[c].restHoldDelay = cnt;
        }
        else
        { // note handling for v0
          if( cnt < 2 )
          { // unlocked note
            note += m_channels[c].transpose;
          }
          m_channels[c].note = note;

          m_channels[c].frequency = notetable[note % 12] + ((note / 12) << 10);
          if( cnt == 1 )
          { // tienote
            setfreq( c );
          }
          else
          { // normal note
            playnote( c );
          }
        }
        break;
      }
      continue; // event is complete
    }
    else
    { // effect event
      switch( fx )
      {
      case 6:
      { // Cut/Stop Voice
        const auto buf = m_channels[c].instrument;
        m_channels[c].instrument = 0;
        playnote( c );
        m_channels[c].instrument = buf;
        m_channels[c].restHoldDelay = fxop;
        continue; // no note follows this event
      }
      case 7: // Vibrato
        m_channels[c].vibratoSpeed = fxop & 0xff;
        m_channels[c].vibratoDepth = fxop >> 8;
        m_channels[c].trigger = fxop >> 9;
        m_channels[c].fxflag |= 1;
        break;
      case 8: // v0: Duration
        if( !m_version )
        {
          m_channels[c].irhcnt = fxop;
        }
        break;
      case 9: // New Level
        m_channels[c].volume = fxop & 63;
        if( m_channels[c].volume + m_channels[c].carrierVolume < 63 )
        { // apply channel volume
          m_channels[c].volume += m_channels[c].carrierVolume;
        }
        else
        {
          m_channels[c].volume = 63;
        }
        setvolume( c );
        break;
      case 0xb: // v4: Set SpFX
        if( m_version == 4 )
        {
          m_channels[c].ispfx = fxop;
        }
        break;
      case 0xc: // Set Instrument
        m_channels[c].ispfx = 0xffff;
        m_channels[c].spfx = 0xffff;
        m_channels[c].instrument = fxop;
        m_channels[c].modulatorVolume = m_instruments[fxop].data[7] & 63;
        if( m_version < 3 && m_version && m_instruments[fxop].finetune ) // Set LevelPuls
        {
          m_channels[c].ilevpuls = m_instruments[fxop].finetune - 1;
        }
        else
        {
          m_channels[c].ilevpuls = 0xff;
          m_channels[c].levpuls = 0xff;
        }
        break;
      case 0xd: // Slide up
        m_channels[c].noteSlideSpeed = fxop;
        m_channels[c].fxflag |= 2;
        break;
      case 0xe: // Slide down
        m_channels[c].noteSlideSpeed = -fxop;
        m_channels[c].fxflag |= 2;
        break;
      }
      goto readseq; // event is incomplete, note follows
    }
  }

  int trackend = 0;
  for( auto& channel: m_channels )
  {
    if( channel.seqend )
    {
      trackend++;
    }
  }
  if( trackend == 9 )
  {
    m_songEnd = true;
  }

  return !m_songEnd;
}

void D00Player::rewind(const boost::optional<size_t>& subsong)
{
#pragma pack(push, 1)
  struct TrackPointer
  {
    uint16_t ptr[9];
    uint8_t volume[9], dummy[5];
  };
#pragma pack(pop)

  if( m_version > 1 )
  { // do nothing if subsong > number of subsongs
    if( subsong.get_value_or( m_currentSubSong ) >= m_header->subsongs )
    {
      return;
    }
  }
  else if( subsong.get_value_or( m_currentSubSong ) >= m_header1->subsongs )
  {
    return;
  }

  memset( m_channels, 0, sizeof(m_channels) );
  const TrackPointer* tpoin;
  if( m_version > 1 )
  {
    tpoin = reinterpret_cast<const TrackPointer*>(m_fileData.data() + LE_WORD( &m_header->trackPointerOfs ));
  }
  else
  {
    tpoin = reinterpret_cast<const TrackPointer*>(m_fileData.data() + LE_WORD( &m_header1->trackPointerOfs ));
  }
  for( int i = 0; i < 9; i++ )
  {
    if( LE_WORD( &tpoin[subsong.get_value_or( m_currentSubSong )].ptr[i] ) )
    { // track enabled
      m_channels[i].speed = LE_WORD(
        reinterpret_cast<const uint16_t*>(m_fileData.data()
          + LE_WORD( &tpoin[subsong.get_value_or( m_currentSubSong )].ptr[i] )) );
      m_channels[i].patternData = reinterpret_cast<const uint16_t*>(m_fileData.data()
        + LE_WORD( &tpoin[subsong.get_value_or( m_currentSubSong )].ptr[i] ) +
        2);
    }
    else
    { // track disabled
      m_channels[i].speed = 0;
      m_channels[i].patternData = nullptr;
    }
    m_channels[i].ispfx = 0xffff;
    m_channels[i].spfx = 0xffff; // no SpFX
    m_channels[i].ilevpuls = 0xff;
    m_channels[i].levpuls = 0xff; // no LevelPuls
    m_channels[i].carrierVolume =
      tpoin[subsong.get_value_or( m_currentSubSong )].volume[i] & 0x7f; // our player may savely ignore bit 7
    m_channels[i].volume = m_channels[i].carrierVolume; // initialize volume
  }
  m_songEnd = false;
  getOpl()->writeReg( 1, 32 ); // reset OPL chip
  m_currentSubSong = subsong.get_value_or( m_currentSubSong );
}

std::string D00Player::type() const
{
  char tmpstr[40];

  sprintf( tmpstr, "EdLib packed (version %d)", m_version > 1 ? m_header->version : m_header1->version );
  return tmpstr;
}

size_t D00Player::framesUntilUpdate() const
{
  if( m_version > 1 )
  {
    return SampleRate / m_header->speed;
  }
  else
  {
    return SampleRate / m_header1->speed;
  }
}

size_t D00Player::subSongCount() const
{
  if( m_version <= 1 )
  { // return number of subsongs
    return m_header1->subsongs;
  }
  else
  {
    return m_header->subsongs;
  }
}

/*** private methods *************************************/

void D00Player::setvolume(uint8_t chan)
{
  const auto op = s_opTable[chan];
  const auto insnr = m_channels[chan].instrument;

  getOpl()->writeReg( 0x43 + op,
                      static_cast<uint8_t>(63
                        - ((63 - (m_instruments[insnr].data[2] & 63)) / 63.0) * (63 - m_channels[chan].volume)) +
                        (m_instruments[insnr].data[2] & 192) );
  if( m_instruments[insnr].data[10] & 1 )
  {
    getOpl()->writeReg( 0x40 + op,
                        static_cast<uint8_t>(63
                          - ((63 - m_channels[chan].modulatorVolume) / 63.0) * (63 - m_channels[chan].volume)) +
                          (m_instruments[insnr].data[7] & 192) );
  }
  else
  {
    getOpl()->writeReg( 0x40 + op, m_channels[chan].modulatorVolume + (m_instruments[insnr].data[7] & 192) );
  }
}

void D00Player::setfreq(uint8_t chan)
{
  auto freq = m_channels[chan].frequency;

  if( m_version == 4 )
  { // v4: apply instrument finetune
    freq += m_instruments[m_channels[chan].instrument].finetune;
  }

  freq += m_channels[chan].noteSlideValue;
  getOpl()->writeReg( 0xa0 + chan, freq & 255 );
  if( m_channels[chan].keyOn )
  {
    getOpl()->writeReg( 0xb0 + chan, ((freq >> 8) & 31) | 0x20 );
  }
  else
  {
    getOpl()->writeReg( 0xb0 + chan, (freq >> 8) & 31 );
  }
}

void D00Player::setinst(uint8_t chan)
{
  const auto op = s_opTable[chan];
  const auto insnr = m_channels[chan].instrument;

  // set instrument data
  getOpl()->writeReg( 0x63 + op, m_instruments[insnr].data[0] );
  getOpl()->writeReg( 0x83 + op, m_instruments[insnr].data[1] );
  getOpl()->writeReg( 0x23 + op, m_instruments[insnr].data[3] );
  getOpl()->writeReg( 0xe3 + op, m_instruments[insnr].data[4] );
  getOpl()->writeReg( 0x60 + op, m_instruments[insnr].data[5] );
  getOpl()->writeReg( 0x80 + op, m_instruments[insnr].data[6] );
  getOpl()->writeReg( 0x20 + op, m_instruments[insnr].data[8] );
  getOpl()->writeReg( 0xe0 + op, m_instruments[insnr].data[9] );
  if( m_version )
  {
    getOpl()->writeReg( 0xc0 + chan, m_instruments[insnr].data[10] );
  }
  else
  {
    getOpl()->writeReg( 0xc0 + chan, (m_instruments[insnr].data[10] << 1) + (m_instruments[insnr].finetune & 1) );
  }
}

void D00Player::playnote(uint8_t chan)
{
  // set misc vars & play
  getOpl()->writeReg( 0xb0 + chan, 0 ); // stop old note
  setinst( chan );
  m_channels[chan].keyOn = true;
  setfreq( chan );
  setvolume( chan );
}

void D00Player::vibrato(uint8_t chan)
{
  if( !m_channels[chan].vibratoDepth )
  {
    return;
  }

  if( m_channels[chan].trigger )
  {
    m_channels[chan].trigger--;
  }
  else
  {
    m_channels[chan].trigger = m_channels[chan].vibratoDepth;
    m_channels[chan].vibratoSpeed = -m_channels[chan].vibratoSpeed;
  }
  m_channels[chan].frequency += m_channels[chan].vibratoSpeed;
  setfreq( chan );
}
