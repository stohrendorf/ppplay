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
 * u6m.cpp - Ultima 6 Music Player by Marc Winterrowd.
 * This code extends the Adlib Winamp plug-in by Simon Peter <dn.tlp@gmx.net>
 */

#include "u6m.h"

#include "stream/filestream.h"

Player* U6mPlayer::factory()
{
  return new U6mPlayer();
}

bool U6mPlayer::load(const std::string& filename)
{
  // file validation section
  // this section only checks a few *necessary* conditions

  FileStream f( filename );
  if( !f )
  {
    return false;
  }
  const auto filesize = f.size();

  if( filesize < 6 )
  {
    return false;
  }

  // check if the file has a valid pseudo-header
  uint8_t pseudo_header[6];
  f.read( pseudo_header, 6 );
  std::streamsize decompressed_filesize = pseudo_header[0] + (pseudo_header[1] << 8);

  if( !(pseudo_header[2] == 0 && pseudo_header[3] == 0 &&
    (pseudo_header[4] + ((pseudo_header[5] & 0x1) << 8) == 0x100) &&
    decompressed_filesize > (filesize - 4)) )
  {
    return false;
  }

  // load section
  m_songData.resize( decompressed_filesize );
  std::vector<uint8_t> compressed_song_data;
  compressed_song_data.resize( filesize - 3u );

  f.seek( 4 );
  f.read( compressed_song_data.data(), filesize - 4 );

  // attempt to decompress the song data
  // if unsuccessful, deallocate song_data[] on the spot, and return(false)
  DataBlock source, destination;
  source.assign( compressed_song_data.begin(), compressed_song_data.begin() + filesize - 4 );

  if( !lzw_decompress( source, m_songData ) )
  {
    return false;
  }

  rewind( size_t( 0 ) );
  return true;
}

bool U6mPlayer::update()
{
  if( !m_driverActive )
  {
    m_driverActive = true;
    dec_clip( m_readDelay );
    if( m_readDelay == 0 )
    {
      command_loop();
    }

    // on all Adlib channels: freq slide/vibrato, mute factor slide
    for( int i = 0; i < 9; i++ )
    {
      if( m_channelFreqSignedDelta[i] != 0 )
        // frequency slide + mute factor slide
      {
        // freq slide
        freq_slide( i );

        // mute factor slide
        if( m_carrierMfSignedDelta[i] != 0 )
        {
          mf_slide( i );
        }
      }
      else
        // vibrato + mute factor slide
      {
        // vibrato
        if( (m_vbMultiplier[i] != 0) && ((m_channelFreq[i].hi & 0x20) == 0x20) )
        {
          vibrato( i );
        }

        // mute factor slide
        if( m_carrierMfSignedDelta[i] != 0 )
        {
          mf_slide( i );
        }
      }
    }

    m_driverActive = false;
  }

  return !m_songEnd;
}

void U6mPlayer::rewind(const boost::optional<size_t>&)
{
  m_songEnd = false;

  // set the driver's internal variables
  Nybbles freq_word = { 0, 0 };

  m_driverActive = false;
  m_songPos = 0;
  m_loopPosition = 0; // position of the loop point
  m_readDelay = 0; // delay (in timer ticks) before further song data is read

  for( int i = 0; i < 9; i++ )
  {
    // frequency
    m_channelFreqSignedDelta[i] = 0;
    m_channelFreq[i] = freq_word; // Adlib freq settings for each channel

    // vibrato ("vb")
    m_vbCurrentValue[i] = 0;
    m_vbDoubleAmplitude[i] = 0;
    m_vbMultiplier[i] = 0;
    m_vbDirectionFlag[i] = false;

    // mute factor ("mf") == ~(volume)
    m_carrierAttenuation[i] = 0;
    m_carrierMfSignedDelta[i] = 0;
    m_carrierMfModDelayBackup[i] = 0;
    m_carrierMfModDelay[i] = 0;
  }

  while( !m_subsongStack.empty() )
  { // empty subsong stack
    m_subsongStack.pop();
  }

  getOpl()->writeReg( 1, 32 ); // go to OPL2 mode
}

size_t U6mPlayer::framesUntilUpdate() const
{
  return SampleRate / 60; // the Ultima 6 music driver expects to be called at 60 Hz
}

// ============================================================================================
//
//
//    Functions called by load()
//
//
// ============================================================================================

// decompress from memory to memory
bool U6mPlayer::lzw_decompress(const U6mPlayer::DataBlock& source, U6mPlayer::DataBlock& dest)
{
  bool end_marker_reached = false;
  int codeword_size = 9;
  long bits_read = 0;
  uint32_t next_free_codeword = 0x102;
  size_t dictionary_size = 0x200;
  MyDict dictionary;
  std::stack<uint8_t> root_stack;

  size_t bytes_written = 0;

  uint32_t pW = 0;
  uint8_t C;

  while( !end_marker_reached )
  {
    auto cW = get_next_codeword( bits_read, source.data(), codeword_size );
    switch( cW )
    {
      // re-init the dictionary
    case 0x100:
      codeword_size = 9;
      next_free_codeword = 0x102;
      dictionary_size = 0x200;
      dictionary.reset();
      cW = get_next_codeword( bits_read, source.data(), codeword_size );
      if( !safeOutputRoot( cW, dest, bytes_written ) )
      {
        return false;
      }
      break;
      // end of compressed file has been reached
    case 0x101:
      end_marker_reached = true;
      break;
      // (cW <> 0x100) && (cW <> 0x101)
    default:
      if( cW < next_free_codeword ) // codeword is already in the dictionary
      {
        // create the string associated with cW (on the stack)
        get_string( cW, dictionary, root_stack );
        C = root_stack.top();
        // output the string represented by cW
        while( !root_stack.empty() )
        {
          if( !safeOutputRoot( root_stack.top(), dest, bytes_written ) )
          {
            return false;
          }
          root_stack.pop();
        }
        // add pW+C to the dictionary
        dictionary.add( C, pW );

        next_free_codeword++;
        if( next_free_codeword >= dictionary_size )
        {
          if( codeword_size < MyDict::MaxCodewordLength )
          {
            codeword_size += 1;
            dictionary_size *= 2;
          }
        }
      }
      else
      { // codeword is not yet defined
        // create the string associated with pW (on the stack)
        get_string( pW, dictionary, root_stack );
        C = root_stack.top();
        // output the string represented by pW
        while( !root_stack.empty() )
        {
          if( !safeOutputRoot( root_stack.top(), dest, bytes_written ) )
          {
            return false;
          }
          root_stack.pop();
        }
        // output the char C
        if( !safeOutputRoot( C, dest, bytes_written ) )
        {
          return false;
        }

        // the new dictionary entry must correspond to cW
        // if it doesn't, something is wrong with the lzw-compressed data.
        if( cW != next_free_codeword )
        {
          /*                        printf("cW != next_free_codeword!\n");
                  exit(-1); */
          return false;
        }
        // add pW+C to the dictionary
        dictionary.add( C, pW );

        next_free_codeword++;
        if( next_free_codeword >= dictionary_size )
        {
          if( codeword_size < MyDict::MaxCodewordLength )
          {
            codeword_size += 1;
            dictionary_size *= 2;
          }
        }
      };
      break;
    }
    // shift roles - the current cW becomes the new pW
    pW = cW;
  }

  return (true); // indicate successful decompression
}

// --------------------
// Additional functions
// --------------------

// Read the next code word from the source buffer
uint32_t U6mPlayer::get_next_codeword(long& bits_read, const uint8_t* source, int codeword_size)
{
  const auto b0 = source[bits_read / 8];
  const auto b1 = source[bits_read / 8 + 1];
  const auto b2 = source[bits_read / 8 + 2];

  uint32_t codeword = ((b2 << 16) | (b1 << 8) | b0) >> (bits_read % 8);
  switch( codeword_size )
  {
  case 0x9:
    codeword &= 0x1ff;
    break;
  case 0xa:
    codeword &= 0x3ff;
    break;
  case 0xb:
    codeword &= 0x7ff;
    break;
  case 0xc:
    codeword &= 0xfff;
    break;
  default:
    codeword = std::numeric_limits<uint32_t>::max(); // indicates that an error has occurred
    break;
  }

  bits_read += codeword_size;
  return codeword;
}

// output a root to memory
void U6mPlayer::output_root(uint8_t root, uint8_t* destination, size_t& position)
{
  destination[position++] = root;
}

// output the string represented by a codeword
void U6mPlayer::get_string(uint32_t codeword, U6mPlayer::MyDict& dictionary,
                           std::stack<uint8_t>& root_stack)
{
  auto current_codeword = codeword;

  while( current_codeword > 0xff )
  {
    auto root = dictionary.get_root( current_codeword );
    current_codeword = dictionary.get_codeword( current_codeword );
    root_stack.push( root );
  }

  // push the root at the leaf
  root_stack.push( static_cast<uint8_t>(current_codeword) );
}

// ============================================================================================
//
//
//    Functions called by update()
//
//
// ============================================================================================

// This function reads the song data and executes the embedded commands.
void U6mPlayer::command_loop()
{
  bool repeat_loop = true; //

  do
  {
    // extract low and high command nibbles
    const auto command_byte = nextByte(); // implicitly increments song_pos
    const uint8_t command_nibble_hi = command_byte >> 4;
    const uint8_t command_nibble_lo = command_byte & 0xf;

    switch( command_nibble_hi )
    {
    case 0x0:
      stopNote( command_nibble_lo );
      break;
    case 0x1:
      restartNote( command_nibble_lo );
      break;
    case 0x2:
      playNote( command_nibble_lo );
      break;
    case 0x3:
      setCarrierAttenuation( command_nibble_lo );
      break;
    case 0x4:
      setModulatorAttenuation( command_nibble_lo );
      break;
    case 0x5:
      setPortamentoSpeed( command_nibble_lo );
      break;
    case 0x6:
      setVibratoParameters( command_nibble_lo );
      break;
    case 0x7:
      setInstrument( command_nibble_lo );
      break;
    case 0x8:
      switch( command_nibble_lo )
      {
      case 1:
        playSubsong();
        break;
      case 2:
        delay();
        repeat_loop = false;
        break;
      case 3:
        readInstrumentData();
        break;
      case 5:
        setVolSlideUpSpeed();
        break;
      case 6:
        setVolSlideDownSpeed();
        break;
      default:
        break; // maybe generate an error?
      }
      break;
    case 0xE:
      setLoopStart();
      break;
    case 0xF:
      returnFromSubsong();
      break;
    default:
      break; // maybe generate an error?
    }
  } while( repeat_loop );
}

// --------------------------------------------------------
//    The commands supported by the U6 music file format
// --------------------------------------------------------

// ----------------------------------------
// Set octave and frequency, note off
// Format: 0c nn
// c = channel, nn = packed Adlib frequency
// ----------------------------------------
void U6mPlayer::stopNote(int channel)
{
  const auto freq_byte = nextByte();
  const auto freq_word = expand_freq_byte( freq_byte );
  set_adlib_freq( channel, freq_word );
}

// ---------------------------------------------------
// Set octave and frequency, old note off, new note on
// Format: 1c nn
// c = channel, nn = packed Adlib frequency
// ---------------------------------------------------
void U6mPlayer::restartNote(int channel)
{
  m_vbDirectionFlag[channel] = false;
  m_vbCurrentValue[channel] = 0;

  const auto freq_byte = nextByte();
  auto freq_word = expand_freq_byte( freq_byte );
  set_adlib_freq( channel, freq_word );

  freq_word.hi |= 0x20; // note on
  set_adlib_freq( channel, freq_word );
}

// ----------------------------------------
// Set octave and frequency, note on
// Format: 2c nn
// c = channel, nn = packed Adlib frequency
// ----------------------------------------
void U6mPlayer::playNote(int channel)
{
  const auto freq_byte = nextByte();
  auto freq_word = expand_freq_byte( freq_byte );
  freq_word.hi |= 0x20; // note on
  set_adlib_freq( channel, freq_word );
}

// --------------------------------------
// Set "carrier mute factor"==not(volume)
// Format: 3c nn
// c = channel, nn = mute factor
// --------------------------------------
void U6mPlayer::setCarrierAttenuation(int channel)
{
  m_carrierMfSignedDelta[channel] = 0;
  const auto mf_byte = nextByte();
  setCarrierAttenuation( channel, mf_byte );
}

// ----------------------------------------
// set "modulator mute factor"==not(volume)
// Format: 4c nn
// c = channel, nn = mute factor
// ----------------------------------------
void U6mPlayer::setModulatorAttenuation(int channel)
{
  uint8_t mf_byte;

  mf_byte = nextByte();
  setModulatorAttenuation( channel, mf_byte );
}

// --------------------------------------------
// Set portamento (pitch slide)
// Format: 5c nn
// c = channel, nn = signed channel pitch delta
// --------------------------------------------
void U6mPlayer::setPortamentoSpeed(int channel)
{
  m_channelFreqSignedDelta[channel] = nextSignedByte();
}

// --------------------------------------------
// Set vibrato parameters
// Format: 6c mn
// c = channel
// m = vibrato double amplitude
// n = vibrato multiplier
// --------------------------------------------
void U6mPlayer::setVibratoParameters(int channel)
{
  const auto vb_parameters = nextByte();
  m_vbDoubleAmplitude[channel] = vb_parameters >> 4; // high nibble
  m_vbMultiplier[channel] = vb_parameters & 0xF; // low nibble
}

// ----------------------------------------
// Assign Adlib instrument to Adlib channel
// Format: 7c nn
// c = channel, nn = instrument number
// ----------------------------------------
void U6mPlayer::setInstrument(int channel)
{
  int instrument_offset = m_instrumentOffsets[nextByte()];
  writeOperatorRegister( channel, false, 0x20, m_songData[instrument_offset + 0] );
  writeOperatorRegister( channel, false, 0x40, m_songData[instrument_offset + 1] );
  writeOperatorRegister( channel, false, 0x60, m_songData[instrument_offset + 2] );
  writeOperatorRegister( channel, false, 0x80, m_songData[instrument_offset + 3] );
  writeOperatorRegister( channel, false, 0xE0, m_songData[instrument_offset + 4] );
  writeOperatorRegister( channel, true, 0x20, m_songData[instrument_offset + 5] );
  writeOperatorRegister( channel, true, 0x40, m_songData[instrument_offset + 6] );
  writeOperatorRegister( channel, true, 0x60, m_songData[instrument_offset + 7] );
  writeOperatorRegister( channel, true, 0x80, m_songData[instrument_offset + 8] );
  writeOperatorRegister( channel, true, 0xE0, m_songData[instrument_offset + 9] );
  getOpl()->writeReg( 0xC0 + channel, m_songData[instrument_offset + 10] );
}

// -------------------------------------------
// Branch to a new subsong
// Format: 81 nn aa bb
// nn == number of times to repeat the subsong
// aa == subsong offset (low byte)
// bb == subsong offset (high byte)
// -------------------------------------------
void U6mPlayer::playSubsong()
{
  SubsongInfo new_ss_info;

  new_ss_info.repetitions = nextByte();
  new_ss_info.start = nextByte();
  new_ss_info.start += nextByte() << 8;
  new_ss_info.continue_pos = m_songPos;

  m_subsongStack.push( new_ss_info );
  m_songPos = new_ss_info.start;
}

// ------------------------------------------------------------
// Stop interpreting commands for this timer tick
// Format: 82 nn
// nn == delay (in timer ticks) until further data will be read
// ------------------------------------------------------------
void U6mPlayer::delay()
{
  m_readDelay = nextByte();
}

// -----------------------------
// Adlib instrument data follows
// Format: 83 nn <11 bytes>
// nn == instrument number
// -----------------------------
void U6mPlayer::readInstrumentData()
{
  uint8_t instrument_number = nextByte();
  m_instrumentOffsets[instrument_number] = m_songPos;
  m_songPos += 11;
}

// ----------------------------------------------
// Set -1 mute factor slide (upward volume slide)
// Format: 85 cn
// c == channel
// n == slide delay
// ----------------------------------------------
void U6mPlayer::setVolSlideUpSpeed()
{
  uint8_t data_byte = nextByte();
  int channel = data_byte >> 4; // high nibble
  uint8_t slide_delay = data_byte & 0xF; // low nibble
  m_carrierMfSignedDelta[channel] = +1;
  m_carrierMfModDelay[channel] = slide_delay + 1;
  m_carrierMfModDelayBackup[channel] = slide_delay + 1;
}

// ------------------------------------------------
// Set +1 mute factor slide (downward volume slide)
// Format: 86 cn
// c == channel
// n == slide speed
// ------------------------------------------------
void U6mPlayer::setVolSlideDownSpeed()
{
  uint8_t data_byte = nextByte();
  int channel = data_byte >> 4; // high nibble
  uint8_t slide_delay = data_byte & 0xF; // low nibble
  m_carrierMfSignedDelta[channel] = -1;
  m_carrierMfModDelay[channel] = slide_delay + 1;
  m_carrierMfModDelayBackup[channel] = slide_delay + 1;
}

// --------------
// Set loop point
// Format: E?
// --------------
void U6mPlayer::setLoopStart()
{
  m_loopPosition = m_songPos;
}

// ---------------------------
// Return from current subsong
// Format: F?
// ---------------------------
void U6mPlayer::returnFromSubsong()
{
  if( !m_subsongStack.empty() )
  {
    SubsongInfo temp = m_subsongStack.top();
    m_subsongStack.pop();
    temp.repetitions--;
    if( temp.repetitions == 0 )
    {
      m_songPos = temp.continue_pos;
    }
    else
    {
      m_songPos = temp.start;
      m_subsongStack.push( temp );
    }
  }
  else
  {
    m_songPos = m_loopPosition;
    m_songEnd = true;
  }
}

// --------------------
// Additional functions
// --------------------

// This function decrements its argument, without allowing it to become
// negative.
void U6mPlayer::dec_clip(int& param)
{
  param--;
  if( param < 0 )
  {
    param = 0;
  }
}

// Returns the byte at the current song position.
// Side effect: increments song_pos.
uint8_t U6mPlayer::nextByte()
{
  return m_songData[m_songPos++];
}

// Same as nextByte(), except that it returns a signed byte
int8_t U6mPlayer::nextSignedByte()
{
  auto song_byte = m_songData[m_songPos++];
  int signed_value;
  if( song_byte <= 127 )
  {
    signed_value = song_byte;
  }
  else
  {
    signed_value = static_cast<int>(song_byte) - 0x100;
  }
  return static_cast<int8_t>(signed_value);
}

U6mPlayer::Nybbles U6mPlayer::expand_freq_byte(uint8_t freq_byte)
{
  static const Nybbles freq_table[24] = {
    { 0x00, 0x00 },
    { 0x58, 0x01 },
    { 0x82, 0x01 },
    { 0xB0, 0x01 },
    { 0xCC, 0x01 },
    { 0x03, 0x02 },
    { 0x41, 0x02 },
    { 0x86, 0x02 },
    { 0x00, 0x00 },
    { 0x6A, 0x01 },
    { 0x96, 0x01 },
    { 0xC7, 0x01 },
    { 0xE4, 0x01 },
    { 0x1E, 0x02 },
    { 0x5F, 0x02 },
    { 0xA8, 0x02 },
    { 0x00, 0x00 },
    { 0x47, 0x01 },
    { 0x6E, 0x01 },
    { 0x9A, 0x01 },
    { 0xB5, 0x01 },
    { 0xE9, 0x01 },
    { 0x24, 0x02 },
    { 0x66, 0x02 }
  };

  auto packed_freq = freq_byte & 0x1F;
  const auto octave = freq_byte >> 5;

  // range check (not present in the original U6 music driver)
  if( packed_freq >= 24 )
  {
    packed_freq = 0;
  }

  Nybbles freq_word;
  freq_word.hi = freq_table[packed_freq].hi + (octave << 2);
  freq_word.lo = freq_table[packed_freq].lo;

  return freq_word;
}

void U6mPlayer::set_adlib_freq(int channel, U6mPlayer::Nybbles freq_word)
{
  getOpl()->writeReg( 0xA0 + channel, freq_word.lo );
  getOpl()->writeReg( 0xB0 + channel, freq_word.hi );
  // update the Adlib register backups
  m_channelFreq[channel] = freq_word;
}

// this function sets the Adlib frequency, but does not update the register
// backups
void U6mPlayer::set_adlib_freq_no_update(int channel,
                                         U6mPlayer::Nybbles freq_word)
{
  getOpl()->writeReg( 0xA0 + channel, freq_word.lo );
  getOpl()->writeReg( 0xB0 + channel, freq_word.hi );
}

void U6mPlayer::setCarrierAttenuation(int channel, uint8_t mute_factor)
{
  writeOperatorRegister( channel, true, 0x40, mute_factor );
  m_carrierAttenuation[channel] = mute_factor;
}

void U6mPlayer::setModulatorAttenuation(int channel, uint8_t mute_factor)
{
  writeOperatorRegister( channel, false, 0x40, mute_factor );
}

void U6mPlayer::freq_slide(int channel)
{
  Nybbles freq = m_channelFreq[channel];

  long freq_word =
    freq.lo + (freq.hi << 8) + m_channelFreqSignedDelta[channel];
  if( freq_word < 0 )
  {
    freq_word += 0x10000;
  }
  if( freq_word > 0xFFFF )
  {
    freq_word -= 0x10000;
  }

  freq.lo = freq_word & 0xFF;
  freq.hi = (freq_word >> 8) & 0xFF;
  set_adlib_freq( channel, freq );
}

void U6mPlayer::vibrato(int channel)
{
  Nybbles freq;

  if( m_vbCurrentValue[channel] >= m_vbDoubleAmplitude[channel] )
  {
    m_vbDirectionFlag[channel] = true;
  }
  else if( m_vbCurrentValue[channel] <= 0 )
  {
    m_vbDirectionFlag[channel] = false;
  }

  if( !m_vbDirectionFlag[channel] )
  {
    m_vbCurrentValue[channel]++;
  }
  else
  {
    m_vbCurrentValue[channel]--;
  }

  long freq_word = m_channelFreq[channel].lo + (m_channelFreq[channel].hi << 8);
  freq_word += (m_vbCurrentValue[channel] -
    (m_vbDoubleAmplitude[channel] >> 1)) * m_vbMultiplier[channel];
  if( freq_word < 0 )
  {
    freq_word += 0x10000;
  }
  if( freq_word > 0xFFFF )
  {
    freq_word -= 0x10000;
  }

  freq.lo = freq_word & 0xFF;
  freq.hi = (freq_word >> 8) & 0xFF;
  set_adlib_freq_no_update( channel, freq );
}

void U6mPlayer::mf_slide(int channel)
{
  m_carrierMfModDelay[channel]--;
  if( m_carrierMfModDelay[channel] == 0 )
  {
    m_carrierMfModDelay[channel] = m_carrierMfModDelayBackup[channel];
    int current_mf = m_carrierAttenuation[channel] + m_carrierMfSignedDelta[channel];
    if( current_mf > 0x3F )
    {
      current_mf = 0x3F;
      m_carrierMfSignedDelta[channel] = 0;
    }
    else if( current_mf < 0 )
    {
      current_mf = 0;
      m_carrierMfSignedDelta[channel] = 0;
    }

    setCarrierAttenuation( channel, static_cast<uint8_t>(current_mf) );
  }
}

void U6mPlayer::writeOperatorRegister(int channel,
                                      bool carrier,
                                      uint8_t reg,
                                      uint8_t val)
{
  static const uint8_t adlib_channel_to_carrier_offset[9] = { 0x03, 0x04, 0x05,
                                                              0x0B, 0x0C, 0x0D,
                                                              0x13, 0x14, 0x15 };
  static const uint8_t adlib_channel_to_modulator_offset[9] = {
    0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12
  };

  if( carrier )
  {
    getOpl()->writeReg( reg + adlib_channel_to_carrier_offset[channel], val );
  }
  else
  {
    getOpl()->writeReg( reg + adlib_channel_to_modulator_offset[channel], val );
  }
}

// ============================================================================================
//
//
//    The Dictionary
//
//
// ============================================================================================

// re-initializes the dictionary
void U6mPlayer::MyDict::reset()
{
  m_contains = 2;
}

// Note: If the dictionary is already full, this function does nothing.
void U6mPlayer::MyDict::add(uint8_t root, int codeword)
{
  if( m_contains < m_dictionary.size() )
  {
    m_dictionary[m_contains].root = root;
    m_dictionary[m_contains].codeword = codeword;
    m_contains++;
  }
}

uint8_t U6mPlayer::MyDict::get_root(uint32_t codeword)
{
  return m_dictionary[codeword - 0x100].root;
}

uint32_t U6mPlayer::MyDict::get_codeword(uint32_t codeword)
{
  return m_dictionary[codeword - 0x100].codeword;
}