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

#include <boost/exception/all.hpp>

#include "xmmodule.h"
#include "xmchannel.h"
#include "xmcell.h"
#include "xmpattern.h"
#include "xminstrument.h"

#include "stream/stream.h"
#include "genmod/channelstate.h"
#include "genmod/orderentry.h"

#include <boost/algorithm/string.hpp>

#include <cmath>

namespace ppp
{
namespace xm
{
namespace
{
#pragma pack(push, 1)
struct XmHeader
{
  char id[17]; // "Extended Module: "
  char title[20];
  uint8_t endOfFile; // 0x1a
  char trackerName[20];
  uint16_t version;
  // offsetof(headerSize) == 60
  uint32_t headerSize;
  uint16_t songLength;
  uint16_t restartPos;
  uint16_t numChannels;
  uint16_t numPatterns;
  uint16_t numInstruments;
  uint16_t flags;
  uint16_t defaultSpeed;
  uint16_t defaultTempo;
  //uint8_t orders[256];
};
#pragma pack(pop)

constexpr std::array<const uint16_t, 12 * 8> g_PeriodTable = { {
                                                                 907, 900, 894, 887, 881, 875, 868, 862, 856, 850, 844,
                                                                 838, 832, 826, 820, 814,
                                                                 808, 802, 796, 791, 785, 779, 774, 768, 762, 757, 752,
                                                                 746, 741, 736, 730, 725,
                                                                 720, 715, 709, 704, 699, 694, 689, 684, 678, 675, 670,
                                                                 665, 660, 655, 651, 646,
                                                                 640, 636, 632, 628, 623, 619, 614, 610, 604, 601, 597,
                                                                 592, 588, 584, 580, 575,
                                                                 570, 567, 563, 559, 555, 551, 547, 543, 538, 535, 532,
                                                                 528, 524, 520, 516, 513,
                                                                 508, 505, 502, 498, 494, 491, 487, 484, 480, 477, 474,
                                                                 470, 467, 463, 460, 457
                                                               }
};
} // anonymous namespace

XmModule::XmModule(int maxRpt, Sample::Interpolation inter)
  :
  AbstractModule( maxRpt, inter ), m_amiga( false ), m_patterns(), m_instruments(), m_channels(), m_noteToPeriod()
  , m_jumpRow( ~0 ), m_jumpOrder( ~0 ), m_isPatLoop(
  false ), m_doPatJump( false ), m_restartPos( 0 ), m_currentPatternDelay( 0 ), m_requestedPatternDelay( 0 )
{
}

XmModule::~XmModule() = default;

bool XmModule::load(Stream* stream)
{
  noConstMetaInfo().filename = stream->name();
  XmHeader hdr;
  *stream >> hdr;
  if( !std::equal( hdr.id, hdr.id + 17, "Extended Module: " ) || hdr.endOfFile != 0x1a /*|| hdr.numChannels > 32*/)
  {
    logger()->warn( L4CXX_LOCATION, "XM Header invalid" );
    return false;
  }
  if( hdr.version != 0x0104 )
  {
    logger()->warn( L4CXX_LOCATION, "Unsupported XM Version %#x", hdr.version );
    return false;
  }
  for( int i = 0; i < (hdr.songLength & 0xff); i++ )
  {
    uint8_t tmp;
    *stream >> tmp;
    addOrder( std::make_unique<OrderEntry>( tmp ) );
  }
  stream->seek( hdr.headerSize + offsetof( XmHeader, headerSize ) );
  noConstMetaInfo().title = boost::algorithm::trim_copy( stringncpy( hdr.title, 20 ) );
  m_restartPos = hdr.restartPos;
  {
    std::string tmp = boost::algorithm::trim_copy( stringncpy( hdr.trackerName, 20 ) );
    if( !tmp.empty() )
    {
      noConstMetaInfo().trackerInfo = tmp;
    }
    else
    {
      noConstMetaInfo().trackerInfo = "<unknown>";
    }
  }
  setTempo( hdr.defaultTempo & 0xff );
  setSpeed( hdr.defaultSpeed & 0xff );
  state().globalVolume = 0x40;
  m_amiga = (hdr.flags & 1) == 0;
  m_channels.clear();
  for( int i = 0; i < hdr.numChannels; i++ )
  {
    m_channels.emplace_back( std::make_unique<XmChannel>( this ) );
  }
  for( uint_fast16_t i = 0; i < hdr.numPatterns; i++ )
  {
    m_patterns.emplace_back( std::make_unique<XmPattern>( hdr.numChannels ) );
    if( !m_patterns.back()->load( stream ) )
    {
      logger()->error( L4CXX_LOCATION, "Pattern loading error" );
      return false;
    }
  }
  while( m_patterns.size() < 256 )
  {
    m_patterns.emplace_back( XmPattern::createDefaultPattern( hdr.numChannels ) );
  }
  for( uint_fast16_t i = 0; i < hdr.numInstruments; i++ )
  {
    m_instruments.emplace_back( std::make_unique<XmInstrument>() );
    if( !m_instruments.back()->load( stream ) )
    {
      logger()->error( L4CXX_LOCATION, "Instrument loading error" );
      return false;
    }
  }
  if( m_amiga )
  {
    logger()->debug( L4CXX_LOCATION, "Initializing Amiga period table" );
    uint16_t destOfs = 0;
    for( int octave = 10; octave > 0; octave-- )
    {
      const auto octaveMask = (1 << (10 - octave)) - 1;
      for( uint16_t amigaVal: g_PeriodTable )
      {
        auto tmp = (amigaVal * 64 + octaveMask) >> (11 - octave);
        m_noteToPeriod.at( destOfs ) = m_noteToPeriod.at( destOfs + 1 ) = tmp;
        destOfs += 2;
      }
    }
    for( size_t i = 0; i < (m_noteToPeriod.size() - 1) / 2; i++ )
    {
      m_noteToPeriod.at( i * 2 + 1 ) = (m_noteToPeriod.at( i * 2 + 0 ) + m_noteToPeriod.at( i * 2 + 2 )) / 2;
    }
  }
  else
  {
    logger()->debug( L4CXX_LOCATION, "Initializing linear period table" );
    uint16_t val = 121 * 16;
    for( auto& i: m_noteToPeriod )
    {
      i = val * 4;
      val--;
    }
  }
  state().pattern = orderAt( 0 )->index();
  return true;
}

size_t XmModule::internal_buildTick(const AudioFrameBufferPtr& buffer)
{
  if( state().order >= orderCount() )
  {
    logger()->debug( L4CXX_LOCATION, "End of order list reached" );
    return 0;
  }
  if( buffer )
  {
    MixerFrameBufferPtr mixerBuffer = std::make_shared<MixerFrameBuffer>( tickBufferLength() );
    const auto& currPat = m_patterns.at( state().pattern );
    for( uint8_t currTrack = 0; currTrack < channelCount(); currTrack++ )
    {
      const auto& chan = m_channels[currTrack];
      BOOST_ASSERT( chan != nullptr );
      const XmCell& cell = currPat->at( currTrack, state().row );
      chan->update( cell, false );
      chan->mixTick( mixerBuffer );
    }
    buffer->resize( mixerBuffer->size() );
    MixerSampleFrame* mixerBufferPtr = &mixerBuffer->front();
    BasicSampleFrame* bufPtr = &buffer->front();
    for( size_t i = 0; i < mixerBuffer->size(); i++ )
    {  // postprocess...
      *bufPtr = mixerBufferPtr->rightShiftClip( 2 );
      bufPtr++;
      mixerBufferPtr++;
    }
  }
  else
  {
    const auto& currPat = m_patterns.at( state().pattern );
    for( uint8_t currTrack = 0; currTrack < channelCount(); currTrack++ )
    {
      const auto& chan = m_channels[currTrack];
      BOOST_ASSERT( chan != nullptr );
      const XmCell& cell = currPat->at( currTrack, state().row );
      chan->update( cell, true );
      chan->mixTick( nullptr );
    }
  }
  nextTick();
  if( !adjustPosition() )
  {
    logger()->debug( L4CXX_LOCATION, "adjustPosition(%s) failed", !buffer );
    return 0;
  }
  state().playedFrames += tickBufferLength();
  return tickBufferLength();
}

bool XmModule::adjustPosition()
{
  bool orderChanged = false;
  bool rowChanged = false;
  if( state().tick == 0 )
  {
    if( m_requestedPatternDelay != 0 )
    {
      m_currentPatternDelay = m_requestedPatternDelay;
      m_requestedPatternDelay = 0;
    }
    if( m_currentPatternDelay != 0 )
    {
      m_currentPatternDelay--;
    }
    if( m_isPatLoop || m_doPatJump )
    {
      if( m_isPatLoop )
      {
        m_isPatLoop = false;
        setRow( m_jumpRow );
        rowChanged = true;
      }
      if( m_doPatJump )
      {
        setRow( m_jumpRow );
        rowChanged = true;
        m_jumpRow = 0;
        m_doPatJump = false;
        m_jumpOrder++;
        if( m_jumpOrder >= orderCount() )
        {
          m_jumpOrder = m_restartPos;
        }
        setOrder( m_jumpOrder );
        orderChanged = true;
      }
    }
    else
    {
      if( !isRunningPatDelay() )
      {
        const auto& currPat = m_patterns.at( state().pattern );
        setRow( (state().row + 1) % currPat->height() );
        rowChanged = true;
        if( state().row == 0 )
        {
          setOrder( state().order + 1 );
          orderChanged = true;
        }
      }
    }
    m_jumpOrder = m_jumpRow = 0;
    m_doPatJump = m_isPatLoop = false;
  }
  if( state().order >= orderCount() )
  {
    logger()->debug( L4CXX_LOCATION, "End of order list" );
    return false;
  }
  if( orderChanged )
  {
    state().pattern = orderAt( state().order )->index();
    setOrder( state().order );
  }
  if( orderAt( state().order )->playbackCount() >= maxRepeat() )
  {
    logger()->debug( L4CXX_LOCATION, "Maximum playback count reached" );
    return false;
  }
  if( rowChanged )
  {
    if( !orderAt( state().order )->increaseRowPlayback( state().row ) )
    {
      logger()->info( L4CXX_LOCATION, "Row playback counter reached limit" );
      setOrder( orderCount() );
      return false;
    }
  }
  return true;
}

ChannelState XmModule::internal_channelStatus(size_t idx) const
{
  return m_channels.at( idx )->status();
}

int XmModule::internal_channelCount() const
{
  return m_channels.size();
}

const std::unique_ptr<XmInstrument>& XmModule::getInstrument(int idx) const
{
  if( !between<int>( idx, 1, m_instruments.size() ) )
  {
    static const std::unique_ptr<XmInstrument> none;
    return none;
  }
  return m_instruments[idx - 1];
}

uint16_t XmModule::noteToPeriod(uint8_t note, int8_t finetune) const
{
  uint16_t tuned = (note << 4) + (finetune >> 3) + 16;
  if( tuned >= m_noteToPeriod.size() )
  {
    return 0;
  }
  return m_noteToPeriod[tuned];
}

uint32_t XmModule::periodToFrequency(uint16_t period) const
{
  const float pbFrq = frequency();
  static const float adjFac = pow( 2, -7.0f / 12 );
  if( m_amiga )
  {
    /*
    Period = (PeriodTab[((Note%12)*8 + FineTune/16]*(1-Frac(FineTune/16)) +
         PeriodTab[(Note%12)*8 + FineTune/16+1]*(Frac(FineTune/16)))
        *16/2^(Note/12);
     FRQ = 8363*1712/Period
     */
    return ((8363ul * 1712) << 16) / pbFrq / period * 12 * 6 * 4;
  }
  else
  {
    /*
    Period = 10*12*16*4 - Note*16*4 - FineTune/2;
    Frequency = 8363*2^((6*12*16*4 - Period) / (12*16*4));
    */
    constexpr int N = 12 * 16 * 4;
    const auto res = 8363 / pbFrq * (1 << 16) * std::pow( 2, static_cast<float>(12 * N - period) / N + 6 );
    return static_cast<uint32_t>(res * adjFac);
  }
}

uint16_t XmModule::periodToFineNoteIndex(uint16_t period, int8_t finetune, uint8_t deltaNote) const
{
  const int tuned = clip( finetune / 8 + 16, 8, 24 );
  uint16_t ofsLo = 0;
  uint16_t ofsHi = 1536;
  for( int i = 0; i < 8; i++ )
  {
    uint16_t ofsMid = (ofsLo + ofsHi) / 2;
    ofsMid &= 0xfff0;
    if( period < m_noteToPeriod.at( ofsMid + tuned - 8 ) )
    {
      ofsLo = ofsMid;
    }
    else
    {
      ofsHi = ofsMid;
    }
  }
  int ofs = ofsLo + tuned + (deltaNote << 1);
  if( ofs >= 1550 )
  {
    ofs = 1551;
  }
  else if( ofs < 0 )
  {
    ofs = 0;
  }
  return ofs;
}

uint16_t XmModule::glissando(uint16_t period, int8_t finetune, uint8_t deltaNote) const
{
  return m_noteToPeriod.at( periodToFineNoteIndex( period, finetune, deltaNote ) );
}

void XmModule::doPatternBreak(int16_t next)
{
  if( next <= 0x3f )
  {
    m_jumpRow = next;
  }
  else
  {
    m_jumpRow = 0;
  }
  doJumpPos( state().order );
  m_doPatJump = true;
}

void XmModule::doJumpPos(int16_t next)
{
  m_jumpOrder = next;
}

void XmModule::doPatLoop(int16_t next)
{
  m_jumpRow = next;
  m_isPatLoop = true;
}

AbstractArchive& XmModule::serialize(AbstractArchive* data)
{
  AbstractModule::serialize( data );
  for( const auto& chan: m_channels )
  {
    if( !chan )
    {
      continue;
    }
    data->archive( chan.get() );
  }
  *data
    % m_jumpRow
    % m_jumpOrder
    % m_isPatLoop
    % m_doPatJump
    % m_restartPos
    % m_requestedPatternDelay
    % m_currentPatternDelay;
  return *data;
}

std::shared_ptr<AbstractModule> XmModule::factory(Stream* stream,
                                                  uint32_t frequency,
                                                  int maxRpt,
                                                  Sample::Interpolation inter)
{
  auto result = std::make_shared<XmModule>( maxRpt, inter );
  if( !result->load( stream ) )
  {
    return nullptr;
  }
  if( !result->initialize( frequency ) )
  {
    return nullptr;
  }
  return result;
}

bool XmModule::isRunningPatDelay() const
{
  return m_currentPatternDelay != 0;
}

void XmModule::doPatDelay(uint8_t counter)
{
  if( isRunningPatDelay() )
  {
    return;
  }
  m_requestedPatternDelay = counter + 1;
}

light4cxx::Logger* XmModule::logger()
{
  return light4cxx::Logger::get( AbstractModule::logger()->name() + ".xm" );
}
}
}

/**
 * @}
 */
