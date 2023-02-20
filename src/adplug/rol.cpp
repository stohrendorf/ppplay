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
 * rol.h - ROL Player by OPLx <oplx@yahoo.com>
 *
 * Visit:  http://tenacity.hispeed.com/aomit/oplx/
 */
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "stream/filestream.h"

#include "rol.h"

namespace
{
constexpr int kSizeofDataRecord = 30;
constexpr int kMaxTickBeat = 60;
constexpr int kSilenceNote = -12;
constexpr int kNumMelodicVoices = 9;
constexpr int kNumPercussiveVoices = 11;
constexpr int kBassDrumChannel = 6;
constexpr int kSnareDrumChannel = 7;
constexpr int kTomtomChannel = 8;
constexpr int kTomtomFreq = 2; //4;
constexpr int kSnareDrumFreq = 2; //kTomtomFreq + 7;
constexpr float kPitchFactor = 400.0f;

constexpr uint16_t kNoteTable[12] = {
  340, // C
  363, // C#
  385, // D
  408, // D#
  432, // E
  458, // F
  485, // F#
  514, // G
  544, // G#
  577, // A
  611, // A#
  647 // B
};

constexpr uint8_t drum_table[4] = { 0x14, 0x12, 0x15, 0x11 };
}

/*** public methods **************************************/

Player* RolPlayer::factory()
{
  return new RolPlayer();
}

//---------------------------------------------------------
bool RolPlayer::load(const std::string& filename)
{
  FileStream f( filename );
  if( !f )
  {
    return false;
  }

  std::string bnk_filename = (boost::filesystem::path( filename ).remove_filename() / "standard.bnk").string();

  f >> m_rolHeader.version_major;
  f >> m_rolHeader.version_minor;

  // Version check
  if( m_rolHeader.version_major != 0 || m_rolHeader.version_minor != 4 )
  {
    return false;
  }

  f.seekrel( 40 );

  f >> m_rolHeader.ticks_per_beat;
  f >> m_rolHeader.beats_per_measure;
  f >> m_rolHeader.edit_scale_y;
  f >> m_rolHeader.edit_scale_x;

  f.seekrel( 1 );

  f >> m_rolHeader.mode;

  f.seekrel( 90 + 38 + 15 );

  f >> m_rolHeader.basic_tempo;

  load_tempo_events( f );

  m_timeOfLastNote = 0;

  if( !load_voice_data( f, bnk_filename ) )
  {
    return false;
  }

  rewind( size_t( 0 ) );
  return true;
}

//---------------------------------------------------------
bool RolPlayer::update()
{
  if( m_nextTempoEvent < m_tempoEvents.size() &&
    m_tempoEvents[m_nextTempoEvent].time == m_currTick )
  {
    SetRefresh( m_tempoEvents[m_nextTempoEvent].multiplier );
    ++m_nextTempoEvent;
  }

  int voice = 0;
  for( auto& voiceData: m_voiceData )
  {
    UpdateVoice( voice, voiceData );
    ++voice;
  }

  ++m_currTick;

  return m_currTick <= m_timeOfLastNote;
}

//---------------------------------------------------------
void RolPlayer::rewind(const boost::optional<size_t>&)
{
  for( auto& voice: m_voiceData )
  {
    voice.Reset();
  }

  memset( m_bxRegister, 0, sizeof(m_bxRegister) );
  memset( m_volumeCache, 0, sizeof(m_volumeCache) );

  m_bdRegister = 0;

  getOpl()->writeReg( 1, 0x20 ); // Enable waveform select (bit 5)

  if( m_rolHeader.mode == 0 )
  {
    getOpl()->writeReg( 0xbd, 0x20 ); // select rhythm mode (bit 5)
    m_bdRegister = 0x20;

    SetFreq( kTomtomChannel, 24 );
    SetFreq( kSnareDrumChannel, 31 );
  }

  m_nextTempoEvent = 0;
  m_currTick = 0;

  SetRefresh( 1.0f );
}

//---------------------------------------------------------
void RolPlayer::SetRefresh(float multiplier)
{
  const float tickBeat = std::min<float>( kMaxTickBeat, m_rolHeader.ticks_per_beat );

  m_refresh = (tickBeat * m_rolHeader.basic_tempo * multiplier) / 60.0f;
}

//---------------------------------------------------------
size_t RolPlayer::framesUntilUpdate() const
{
  return static_cast<size_t>(SampleRate / m_refresh);
}

//---------------------------------------------------------
void RolPlayer::UpdateVoice(int voice, VoiceData& voiceData)
{
  if( voiceData.note_events.empty() || voiceData.mEventStatus & VoiceData::EventStatus::NoteEnd )
  {
    return; // no note data to process, don't bother doing anything.
  }

  if( !(voiceData.mEventStatus & VoiceData::EventStatus::InstrEnd) &&
    voiceData.instrument_events[voiceData.next_instrument_event].time == m_currTick )
  {
    if( voiceData.next_instrument_event < voiceData.instrument_events.size() )
    {
      send_ins_data_to_chip( voice,
                             voiceData.instrument_events[voiceData.next_instrument_event].ins_index );
      ++voiceData.next_instrument_event;
    }
    else
    {
      voiceData.mEventStatus |= VoiceData::EventStatus::InstrEnd;
    }
  }

  if( !(voiceData.mEventStatus & VoiceData::EventStatus::VolumeEnd) &&
    voiceData.volume_events[voiceData.next_volume_event].time == m_currTick )
  {
    const VolumeEvent& volumeEvent = voiceData.volume_events[voiceData.next_volume_event];

    if( voiceData.next_volume_event < voiceData.volume_events.size() )
    {
      const auto volume = static_cast<int>(63.0f * (1.0f - volumeEvent.multiplier));

      SetVolume( voice, volume );

      ++voiceData.next_volume_event; // move to next volume event
    }
    else
    {
      voiceData.mEventStatus |= VoiceData::EventStatus::VolumeEnd;
    }
  }

  if( voiceData.mForceNote ||
    voiceData.current_note_duration > voiceData.mNoteDuration - 1 )
  {
    if( m_currTick != 0 )
    {
      ++voiceData.current_note;
    }

    if( voiceData.current_note < voiceData.note_events.size() )
    {
      const NoteEvent& noteEvent = voiceData.note_events[voiceData.current_note];

      SetNote( voice, noteEvent.number );
      voiceData.current_note_duration = 0;
      voiceData.mNoteDuration = noteEvent.duration;
      voiceData.mForceNote = false;
    }
    else
    {
      SetNote( voice, kSilenceNote );
      voiceData.mEventStatus |= VoiceData::EventStatus::NoteEnd;
      return;
    }
  }

  if( !(voiceData.mEventStatus & VoiceData::EventStatus::PitchEnd) &&
    voiceData.pitch_events[voiceData.next_pitch_event].time == m_currTick )
  {
    if( voiceData.next_pitch_event < voiceData.pitch_events.size() )
    {
      SetPitch( voice, voiceData.pitch_events[voiceData.next_pitch_event].variation );
      ++voiceData.next_pitch_event;
    }
    else
    {
      voiceData.mEventStatus |= VoiceData::EventStatus::PitchEnd;
    }
  }

  ++voiceData.current_note_duration;
}

//---------------------------------------------------------
void RolPlayer::SetNote(int voice, int note)
{
  if( voice < kBassDrumChannel || m_rolHeader.mode )
  {
    SetNoteMelodic( voice, note );
  }
  else
  {
    SetNotePercussive( voice, note );
  }
}

//---------------------------------------------------------
void RolPlayer::SetNotePercussive(int voice, int note)
{
  const int bit_pos = 4 - voice + kBassDrumChannel;

  m_bdRegister &= ~(1 << bit_pos);
  getOpl()->writeReg( 0xbd, m_bdRegister );

  if( note != kSilenceNote )
  {
    switch( voice )
    {
    case kTomtomChannel:
      SetFreq( kSnareDrumChannel, note + 7 );
    case kBassDrumChannel:
      SetFreq( voice, note );
      break;
    }

    m_bdRegister |= 1 << bit_pos;
    getOpl()->writeReg( 0xbd, m_bdRegister );
  }
}

//---------------------------------------------------------
void RolPlayer::SetNoteMelodic(int voice, int note)
{
  getOpl()->writeReg( 0xb0 + voice, m_bxRegister[voice] & ~0x20 );

  if( note != kSilenceNote )
  {
    SetFreq( voice, note, true );
  }
}

//---------------------------------------------------------
void RolPlayer::SetPitch(int voice, float variation)
{
  m_pitchCache[voice] = variation;
  m_freqCache[voice] += static_cast<uint16_t>((m_freqCache[voice] * (variation - 1.0f)) / kPitchFactor);

  getOpl()->writeReg( 0xa0 + voice, m_freqCache[voice] & 0xff );
}

//---------------------------------------------------------
void RolPlayer::SetFreq(int voice, int note, bool keyOn)
{
  uint16_t freq = kNoteTable[note % 12] + ((note / 12) << 10);
  freq += static_cast<uint16_t>((freq * (m_pitchCache[voice] - 1.0f)) / kPitchFactor);

  m_freqCache[voice] = freq;
  m_bxRegister[voice] = ((freq >> 8) & 0x1f);

  getOpl()->writeReg( 0xa0 + voice, freq & 0xff );
  getOpl()->writeReg( 0xb0 + voice, m_bxRegister[voice] | (keyOn ? 0x20 : 0x0) );
}

//---------------------------------------------------------
void RolPlayer::SetVolume(int voice, int volume)
{
  m_volumeCache[voice] = (m_volumeCache[voice] & 0xc0) | volume;

  const int op_offset = (voice < kSnareDrumChannel || m_rolHeader.mode)
                        ? s_opTable[voice] + 3
                        : drum_table[voice - kSnareDrumChannel];

  getOpl()->writeReg( 0x40 + op_offset, m_volumeCache[voice] );
}

//---------------------------------------------------------
void RolPlayer::send_ins_data_to_chip(int voice, int ins_index)
{
  const RolInstrument& instrument = m_instrumentList[ins_index].instrument;

  send_operator( voice, instrument.modulator, instrument.carrier );
}

//---------------------------------------------------------
void RolPlayer::send_operator(int voice, const OPL2Op& modulator,
                              OPL2Op const& carrier)
{
  if( voice < kSnareDrumChannel || m_rolHeader.mode )
  {
    const int op_offset = s_opTable[voice];

    getOpl()->writeReg( 0x20 + op_offset, modulator.ammulti );
    getOpl()->writeReg( 0x40 + op_offset, modulator.ksltl );
    getOpl()->writeReg( 0x60 + op_offset, modulator.ardr );
    getOpl()->writeReg( 0x80 + op_offset, modulator.slrr );
    getOpl()->writeReg( 0xc0 + voice, modulator.fbc );
    getOpl()->writeReg( 0xe0 + op_offset, modulator.waveform );

    m_volumeCache[voice] = (carrier.ksltl & 0xc0) | (m_volumeCache[voice] & 0x3f);

    getOpl()->writeReg( 0x23 + op_offset, carrier.ammulti );
    getOpl()->writeReg( 0x43 + op_offset, m_volumeCache[voice] );
    getOpl()->writeReg( 0x63 + op_offset, carrier.ardr );
    getOpl()->writeReg( 0x83 + op_offset, carrier.slrr );
    //        opl->writeReg( 0xc3+voice    , carrier.fbc      ); <- don't bother
    // writing this.
    getOpl()->writeReg( 0xe3 + op_offset, carrier.waveform );
  }
  else
  {
    const int op_offset = drum_table[voice - kSnareDrumChannel];

    m_volumeCache[voice] = (modulator.ksltl & 0xc0) | (m_volumeCache[voice] & 0x3f);

    getOpl()->writeReg( 0x20 + op_offset, modulator.ammulti );
    getOpl()->writeReg( 0x40 + op_offset, m_volumeCache[voice] );
    getOpl()->writeReg( 0x60 + op_offset, modulator.ardr );
    getOpl()->writeReg( 0x80 + op_offset, modulator.slrr );
    getOpl()->writeReg( 0xc0 + voice, modulator.fbc );
    getOpl()->writeReg( 0xe0 + op_offset, modulator.waveform );
  }
}

//---------------------------------------------------------
void RolPlayer::load_tempo_events(FileStream& f)
{
  int16_t num_tempo_events;
  f >> num_tempo_events;

  m_tempoEvents.reserve( num_tempo_events );

  for( int i = 0; i < num_tempo_events; ++i )
  {
    TempoEvent event;
    f >> event;
    m_tempoEvents.push_back( event );
  }
}

//---------------------------------------------------------
bool RolPlayer::load_voice_data(FileStream& f, std::string const& bnk_filename)
{
  BnkHeader bnk_header;
  FileStream bnk_file( bnk_filename );

  if( !bnk_file )
  {
    return false;
  }

  load_bnk_info( bnk_file, bnk_header );

  const int numVoices = m_rolHeader.mode ? kNumMelodicVoices : kNumPercussiveVoices;

  m_voiceData.reserve( numVoices );
  for( int i = 0; i < numVoices; ++i )
  {
    VoiceData voice;

    load_note_events( f, voice );
    load_instrument_events( f, voice, bnk_file, bnk_header );
    load_volume_events( f, voice );
    load_pitch_events( f, voice );

    m_voiceData.push_back( voice );
  }

  return true;
}

//---------------------------------------------------------
void RolPlayer::load_note_events(FileStream& f, VoiceData& voice)
{
  f.seekrel( 15 );

  int16_t time_of_last_note;
  f >> time_of_last_note;

  if( time_of_last_note != 0 )
  {
    int total_duration = 0;

    do
    {
      NoteEvent event;
      f >> event;

      event.number += kSilenceNote; // adding -12

      voice.note_events.push_back( event );

      total_duration += event.duration;
    } while( total_duration < time_of_last_note );

    if( time_of_last_note > m_timeOfLastNote )
    {
      m_timeOfLastNote = time_of_last_note;
    }
  }

  f.seekrel( 15 );
}

//---------------------------------------------------------
void RolPlayer::load_instrument_events(FileStream& f, VoiceData& voice,
                                       FileStream& bnk_file,
                                       const BnkHeader& bnk_header)
{
  int16_t number_of_instrument_events;
  f >> number_of_instrument_events;

  voice.instrument_events.reserve( number_of_instrument_events );

  for( int i = 0; i < number_of_instrument_events; ++i )
  {
    InstrumentEvent event;
    f >> event.time;
    f.read( event.name, 9 );

    std::string event_name = event.name;
    event.ins_index = load_rol_instrument( bnk_file, bnk_header, event_name );

    voice.instrument_events.push_back( event );

    f.seekrel( 1 + 2 );
  }

  f.seekrel( 15 );
}

//---------------------------------------------------------
void RolPlayer::load_volume_events(FileStream& f, VoiceData& voice)
{
  int16_t number_of_volume_events;
  f >> number_of_volume_events;

  voice.volume_events.reserve( number_of_volume_events );

  for( int i = 0; i < number_of_volume_events; ++i )
  {
    VolumeEvent event;
    f >> event;
    voice.volume_events.push_back( event );
  }

  f.seekrel( 15 );
}

//---------------------------------------------------------
void RolPlayer::load_pitch_events(FileStream& f, VoiceData& voice)
{
  int16_t number_of_pitch_events;
  f >> number_of_pitch_events;

  voice.pitch_events.reserve( number_of_pitch_events );

  for( int i = 0; i < number_of_pitch_events; ++i )
  {
    PitchEvent event;
    f >> event;

    voice.pitch_events.push_back( event );
  }
}

//---------------------------------------------------------
bool RolPlayer::load_bnk_info(FileStream& f, BnkHeader& header)
{
  f >> header.version_major;
  f >> header.version_minor;
  f.read( header.signature, 6 );

  f >> header.number_of_list_entries_used;
  f >> header.total_number_of_list_entries;

  f >> header.abs_offset_of_name_list;
  f >> header.abs_offset_of_data;

  f.seek( header.abs_offset_of_name_list );

  InstrumentNames& ins_name_list = header.ins_name_list;
  ins_name_list.reserve( header.number_of_list_entries_used );

  for( int i = 0; i < header.number_of_list_entries_used; ++i )
  {
    InstrumentName instrument;
    f >> instrument;

    // printf("%s = #%d\n", instrument.name, i );

    ins_name_list.push_back( instrument );
  }

  //std::sort( ins_name_list.begin(), ins_name_list.end(), StringCompare() );

  return true;
}

//---------------------------------------------------------
int RolPlayer::load_rol_instrument(FileStream& f, const BnkHeader& header,
                                   std::string& name)
{
  const int ins_index = get_ins_index( name );

  if( ins_index != -1 )
  {
    return ins_index;
  }

  auto range = std::equal_range( header.ins_name_list.begin(), header.ins_name_list.end(), name, StringCompare() );

  if( range.first != range.second )
  {
    int const seekOffs = header.abs_offset_of_data + (range.first->index * kSizeofDataRecord);
    f.seek( seekOffs );
  }

  UsedList usedIns;
  usedIns.name = name;

  if( range.first != range.second )
  {
    read_rol_instrument( f, usedIns.instrument );
  }
  else
  {
    // set up default instrument data here
    memset( &usedIns.instrument, 0, sizeof( RolInstrument ) );
  }
  m_instrumentList.push_back( usedIns );

  return m_instrumentList.size() - 1;
}

//---------------------------------------------------------
int RolPlayer::get_ins_index(const std::string& name) const
{
  for( size_t i = 0; i < m_instrumentList.size(); ++i )
  {
    if( boost::iequals( m_instrumentList[i].name, name ) )
    {
      return i;
    }
  }

  return -1;
}

//---------------------------------------------------------
void RolPlayer::read_rol_instrument(FileStream& f, RolInstrument& ins)
{
  f >> ins.mode;
  f >> ins.voice_number;

  read_fm_operator( f, ins.modulator );
  read_fm_operator( f, ins.carrier );

  f >> ins.modulator.waveform;
  f >> ins.carrier.waveform;
}

//---------------------------------------------------------
void RolPlayer::read_fm_operator(FileStream& f, OPL2Op& opl2_op)
{
  FMOperator fm_op;
  f >> fm_op;

  opl2_op.ammulti = fm_op.amplitude_vibrato << 7 |
    fm_op.frequency_vibrato << 6 | fm_op.sustaining_sound << 5 |
    fm_op.envelope_scaling << 4 | fm_op.freq_multiplier;
  opl2_op.ksltl = fm_op.key_scale_level << 6 | fm_op.output_level;
  opl2_op.ardr = fm_op.attack_rate << 4 | fm_op.decay_rate;
  opl2_op.slrr = fm_op.sustain_level << 4 | fm_op.release_rate;
  opl2_op.fbc = fm_op.feed_back << 1 | (fm_op.fm_type ^ 1);
}
