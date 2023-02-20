#include "almidi.h"

#include "stuff/numberutils.h"

#include <cstring>

namespace ppp
{
constexpr auto NUM_MIDI_CHANNELS = 16;

#define EMIDI_END_LOOP_VALUE    127
#define EMIDI_ALL_CARDS         127
#define EMIDI_INCLUDE_TRACK     110
#define EMIDI_EXCLUDE_TRACK     111
#define EMIDI_PROGRAM_CHANGE    112
#define EMIDI_VOLUME_CHANGE     113
#define EMIDI_CONTEXT_START     114
#define EMIDI_CONTEXT_END       115
#define EMIDI_LOOP_START        116
#define EMIDI_LOOP_END          117
#define EMIDI_SONG_LOOP_START   118
#define EMIDI_SONG_LOOP_END     119

#define EMIDI_Adlib             7

#define EMIDI_AffectsCurrentCard(c, type) \
    ( ( ( c ) == EMIDI_ALL_CARDS ) || ( ( c ) == ( type ) ) )

namespace
{
constexpr std::array<int, 16> commandLengths = { {
                                                   0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 2, 0
                                                 } };
}

struct EMidi::SongContext
{
  std::size_t pos = 0;
  static constexpr std::size_t Unlooped = std::numeric_limits<std::size_t>::max();
  std::size_t loopstart = Unlooped;
  static constexpr short InfiniteLoop = -1;
  short loopcount = 0;
  short runningStatus = -1;
  EMidi::Timing timing{};
  long delay = 0;
  bool active = false;
};

struct EMidi::Track
{
  Track()
    : contexts()
  {
  }

  std::size_t dataPos = 0;
  std::vector<uint8_t> data{};

  uint8_t nextByte()
  {
    BOOST_ASSERT( dataPos < data.size() );
    return data[dataPos++];
  }

  uint8_t currentByte()
  {
    BOOST_ASSERT( dataPos < data.size() );
    return data[dataPos];
  }

  uint32_t readDelta()
  {
    uint32_t value = 0;

    uint8_t c;
    do
    {
      c = nextByte();
      value = (value << 7) | (c & 0x7f);
    } while( c & 0x80 );

    return value;
  }

  void sysEx()
  {
    dataPos += readDelta();
  }

  long delay = 0;
  bool active = false;
  short runningStatus = -1;

  short currentContext = 0;
  std::array<SongContext, 7> contexts;

  bool includeTrack = false;
  bool programChange = false;
  bool volumeChange = false;
};

void EMidi::resetTracks()
{
  m_timing = Timing();
  m_timing.ticksPerBeat = m_division;

  m_positionInTicks = 0;
  m_activeTracks = 0;
  m_context = 0;

  for( Track& track: m_tracks )
  {
    track.dataPos = 0;
    track.delay = track.readDelta();
    track.active = track.includeTrack;
    track.runningStatus = -1;
    track.currentContext = 0;
    track.contexts[0].loopstart = SongContext::Unlooped;
    track.contexts[0].loopcount = 0;

    if( track.active )
    {
      m_activeTracks++;
    }
  }
}

void EMidi::advanceTick()
{
  m_positionInTicks++;

  m_timing.tick++;
  while( m_timing.tick > m_timing.ticksPerBeat )
  {
    m_timing.tick -= m_timing.ticksPerBeat;
    m_timing.beat++;
  }
  while( m_timing.beat > m_timing.beatsPerMeasure )
  {
    m_timing.beat -= m_timing.beatsPerMeasure;
    m_timing.measure++;
  }
}

void EMidi::metaEvent(EMidi::Track& track)
{
  auto command = track.nextByte();
  auto length = track.readDelta();

#define MIDI_END_OF_TRACK          0x2F
#define MIDI_TEMPO_CHANGE          0x51
#define MIDI_TIME_SIGNATURE        0x58

  switch( command )
  {
  case MIDI_END_OF_TRACK:
    track.active = false;
    m_activeTracks--;
    break;

  case MIDI_TEMPO_CHANGE:
  {
    BOOST_ASSERT( length >= 3 );
    uint32_t num = track.nextByte() << 16;
    num |= track.nextByte() << 8;
    num |= track.nextByte();
    length -= 3;
    setTempo( 60000000 / num );
    break;
  }

  case MIDI_TIME_SIGNATURE:
    BOOST_ASSERT( length >= 2 );
    if( m_timing.tick > 0 || m_timing.beat > 1 )
    {
      m_timing.measure++;
    }

    m_timing.tick = 0;
    m_timing.beat = 1;

    m_timing.beatsPerMeasure = track.nextByte();
    m_timing.timeBase = 1 << track.nextByte();
    length -= 2;
    m_timing.ticksPerBeat = (m_division * 4) / m_timing.timeBase;
    break;
  case 0x20: // MIDI channel prefix assignment
    BOOST_ASSERT( length == 1 );
    track.nextByte();
    --length;
    break;
  case 0x02: // Copyright notice
    std::cout << "Copyright: ";
    while( length-- )
    {
      std::cout << char( track.nextByte() );
    }
    std::cout << "\n";
    length = 0;
    break;
  case 0x01: // Text event
    std::cout << "Comment: ";
    while( length-- )
    {
      std::cout << char( track.nextByte() );
    }
    std::cout << "\n";
    length = 0;
    break;
  case 0x03: // Sequence/track name
    std::cout << "Track name: ";
    while( length-- )
    {
      std::cout << char( track.nextByte() );
    }
    std::cout << "\n";
    length = 0;
    break;
  case 0x04: // Instrument name
    std::cout << "Instrument name: ";
    while( length-- )
    {
      std::cout << char( track.nextByte() );
    }
    std::cout << "\n";
    length = 0;
    break;
  case 0x05: // lyric text
    std::cout << "Lyric: ";
    while( length-- )
    {
      std::cout << char( track.nextByte() );
    }
    length = 0;
    std::cout << "\n";
    break;
  case 0x06: // marker text
    std::cout << "Marker: ";
    while( length-- )
    {
      std::cout << char( track.nextByte() );
    }
    length = 0;
    std::cout << "\n";
    break;
  case 0x08: // title OR muse score program name
    std::cout << "Title: ";
    while( length-- )
    {
      std::cout << char( track.nextByte() );
    }
    length = 0;
    std::cout << "\n";
    break;
  case 0x00: // Sequence number
  case 0x07: // Cue point
  case 0x09: // muse score device name
  case 0x21: // MIDI port
  case 0x54: // SMPTE offset
  case 0x59: // Key signature
  case 0x7f: // Sequencer specific event
    break;
  default:
    throw std::runtime_error( "Unhandled meta event" );
  }

  track.dataPos += length;
}

bool EMidi::interpretControllerInfo(EMidi::Track& track, bool timeSet, int channel, uint8_t c1, uint8_t c2)
{
  BOOST_ASSERT( channel >= 0 && channel < 16 );
  Track* trackptr;
  size_t tracknum;
  int loopcount;

#define MIDI_DETUNE                94
#define MIDI_RHYTHM_CHANNEL        9
#define MIDI_PAN                   10

#define MIDI_VOLUME                0x7
#define MIDI_NOTE_OFF              0x8
#define MIDI_NOTE_ON               0x9
#define MIDI_POLYPHONIC_PRESSURE   0xA
#define MIDI_CONTROL_CHANGE        0xB
#define MIDI_PROGRAM_CHANGE        0xC
#define MIDI_CHANNEL_PRESSURE      0xD
#define MIDI_PITCH_BEND            0xE
#define MIDI_SPECIAL               0xF

#define MIDI_SYSEX                 0xF0
#define MIDI_SYSEX_CONTINUE        0xF7
#define MIDI_META_EVENT            0xFF
#define MIDI_RESET_ALL_CONTROLLERS 0x79
#define MIDI_ALL_NOTES_OFF         0x7b
#define MIDI_MONO_MODE_ON          0x7E
#define MIDI_RUNNING_STATUS        0x80
#define MIDI_SYSTEM_RESET          0xFF

  switch( c1 )
  {
  case MIDI_MONO_MODE_ON:
    track.dataPos++;
    break;

  case MIDI_VOLUME:
    if( !track.volumeChange )
    {
      setChannelVolume( channel, c2 );
    }
    break;

  case EMIDI_INCLUDE_TRACK:
  case EMIDI_EXCLUDE_TRACK:
    break;

  case EMIDI_PROGRAM_CHANGE:
    if( track.programChange )
    {
      m_chips.programChange( channel, c2 & 0x7f );
    }
    break;

  case EMIDI_VOLUME_CHANGE:
    if( track.volumeChange )
    {
      setChannelVolume( channel, c2 );
    }
    break;

  case EMIDI_CONTEXT_START:
    break;

  case EMIDI_CONTEXT_END:
    if( track.currentContext == m_context || m_context < 0 || track.contexts[m_context].pos == 0 )
    {
      break;
    }

    track.currentContext = m_context;
    track.contexts[0].loopstart = track.contexts[m_context].loopstart;
    track.contexts[0].loopcount = track.contexts[m_context].loopcount;
    track.dataPos = track.contexts[m_context].pos;
    track.runningStatus = track.contexts[m_context].runningStatus;

    if( timeSet )
    {
      break;
    }

    m_timing = track.contexts[m_context].timing;
    timeSet = true;
    break;

  case EMIDI_LOOP_START:
  case EMIDI_SONG_LOOP_START:
    if( c2 == 0 )
    {
      loopcount = SongContext::InfiniteLoop;
    }
    else
    {
      loopcount = c2;
    }

    if( c1 == EMIDI_SONG_LOOP_START )
    {
      trackptr = m_tracks.data();
      tracknum = m_tracks.size();
    }
    else
    {
      trackptr = &track;
      tracknum = 1;
    }

    while( tracknum > 0 )
    {
      trackptr->contexts[0].loopcount = loopcount;
      trackptr->contexts[0].pos = trackptr->dataPos;
      trackptr->contexts[0].loopstart = trackptr->dataPos;
      trackptr->contexts[0].runningStatus = trackptr->runningStatus;
      trackptr->contexts[0].active = trackptr->active;
      trackptr->contexts[0].delay = trackptr->delay;
      trackptr->contexts[0].timing = m_timing;
      trackptr++;
      tracknum--;
    }
    break;

  case EMIDI_LOOP_END:
  case EMIDI_SONG_LOOP_END:
    if( c2 != EMIDI_END_LOOP_VALUE || track.contexts[0].loopstart == SongContext::Unlooped
      || track.contexts[0].loopcount == 0 )
    {
      break;
    }

    if( c1 == EMIDI_SONG_LOOP_END )
    {
      trackptr = m_tracks.data();
      tracknum = m_tracks.size();
      m_activeTracks = 0;
    }
    else
    {
      trackptr = &track;
      tracknum = 1;
      m_activeTracks--;
    }

    while( tracknum > 0 )
    {
      if( trackptr->contexts[0].loopcount != SongContext::InfiniteLoop )
      {
        trackptr->contexts[0].loopcount--;
      }

      BOOST_ASSERT( trackptr->contexts[0].loopstart != SongContext::Unlooped );
      trackptr->dataPos = trackptr->contexts[0].loopstart;
      trackptr->runningStatus = trackptr->contexts[0].runningStatus;
      trackptr->delay = trackptr->contexts[0].delay;
      trackptr->active = trackptr->contexts[0].active;
      if( trackptr->active )
      {
        m_activeTracks++;
      }

      if( !timeSet )
      {
        m_timing = trackptr->contexts[0].timing;
        timeSet = true;
      }

      trackptr++;
      tracknum--;
    }
    break;
  case MIDI_DETUNE:
    m_chips.setChannelDetune( channel, c2 );
    break;
  case MIDI_ALL_NOTES_OFF:
    m_chips.allNotesOff( channel );
    break;
  case MIDI_RESET_ALL_CONTROLLERS:
    m_chips.controlChange( channel, MultiChips::ControlData::ResetAllControllers );
    break;
  case 100: // Registered Parameter Number (LSB)
    m_chips.controlChange( channel, MultiChips::ControlData::RpnLsb, c2 );
    break;
  case 101: // Registered Parameter Number (MSB)
    m_chips.controlChange( channel, MultiChips::ControlData::RpnMsb, c2 );
    break;
  case 6: // Data Entry
    m_chips.controlChange( channel, MultiChips::ControlData::DataentryMsb, c2 );
    break;
  case 38: // Data Entry
    m_chips.controlChange( channel, MultiChips::ControlData::DataentryLsb, c2 );
    break;
  case 10: // Pan
    m_chips.controlChange( channel, MultiChips::ControlData::Pan, c2 );
    break;
  case 1: // Modulation wheel
  case 8: // Balance
  case 91: // Effects 1 (Reverb Send Level)
  case 93: // Effects 3 (Chorus Send Level)
  case 0: // Bank select
  case 32: // Bank select
    break;
  default:
    // throw std::runtime_error("Unhandled controller event");
    std::cout << "Unhandled controller event #" << int( c1 ) << std::endl;
  }

  return timeSet;
}

bool EMidi::serviceRoutine()
{
  switch( m_format )
  {
  case Format::PlainMidi:
    return serviceRoutineMidi();
  case Format::IdMus:
    return serviceRoutineMus();
  default:
    return false;
  }
}

bool EMidi::serviceRoutineMidi()
{
  bool timeSet = false;

  while( true )
  {
    bool repeat = false;

    for( Track& track: m_tracks )
    {
      while( track.active && track.delay == 0 )
      {
        if( track.dataPos >= track.data.size() )
        {
          track.active = false;
          break;
        }
        auto event = track.nextByte();

        if( event & MIDI_RUNNING_STATUS )
        {
          track.runningStatus = event;
        }
        else
        {
          BOOST_ASSERT( track.runningStatus != -1 );
          event = track.runningStatus;
          track.dataPos--;
        }

        if( ((event >> 4) & 0x0f) == MIDI_SPECIAL )
        {
          switch( event )
          {
          case MIDI_SYSEX:
          case MIDI_SYSEX_CONTINUE:
            track.sysEx();
            break;

          case MIDI_META_EVENT:
            metaEvent( track );
            break;

          default:
            throw std::runtime_error( "Unhandled special command" );
          }

          if( track.active )
          {
            track.delay = track.readDelta();
          }

          continue;
        }

        auto channel = event & 0x0f;
        auto command = (event >> 4) & 0x0f;

        BOOST_ASSERT( command >= 8 );
        uint8_t c1 = 0, c2 = 0;
        if( commandLengths[command] > 0 )
        {
          c1 = track.nextByte();
          if( commandLengths[command] > 1 )
          {
            c2 = track.nextByte();
          }
        }

        switch( command )
        {
        case MIDI_NOTE_OFF:
          m_chips.noteOff( channel, c1 );
          break;

        case MIDI_NOTE_ON:
          m_chips.noteOn( channel, c1, c2 );
          break;

        case MIDI_CONTROL_CHANGE:
          timeSet = interpretControllerInfo( track, timeSet, channel, c1, c2 );
          break;

        case MIDI_PROGRAM_CHANGE:
          if( !track.programChange )
          {
            m_chips.programChange( channel, c1 & 0x7f );
          }
          break;

        case MIDI_PITCH_BEND:
          m_chips.pitchBend( channel, c1, c2 );
          break;

        case MIDI_POLYPHONIC_PRESSURE:
        case MIDI_CHANNEL_PRESSURE:
          break;

        default:
          throw std::runtime_error( "Unhandled command" );
        }

        track.delay = track.readDelta();
      }

      track.delay--;

      if( m_activeTracks == 0 )
      {
        resetTracks();
        if( m_loop )
        {
          repeat = true;
          break;
        }
        else
        {
          return false;
        }
      }
    }

    if( !repeat )
    {
      break;
    }
  }

  advanceTick();
  return true;
}

bool EMidi::serviceRoutineMus()
{
  // This loop isn't endless; it's only used as a "goto-less goto"
  // if looping is enabled, as there's a "break" at the very end.
  while( true )
  {
    bool pitchIsUsed = false;
    while( m_tracks[0].active && m_tracks[0].delay == 0 )
    {
      if( m_tracks[0].dataPos >= m_tracks[0].data.size() )
      {
        m_tracks[0].active = false;
        break;
      }

      auto event = m_tracks[0].nextByte();

      auto channel = event & 0x0f;
      // swap channels 9 and 15 to use MIDI's percussion channel handling
      if( channel == 9 )
      {
        channel = 15;
      }
      else if( channel == 15 )
      {
        channel = 9;
      }

      const auto eventType = (event >> 4) & 7;
      const bool isLast = (event & 0x80) != 0;

      switch( eventType )
      {
      case 0: // note off
        m_chips.noteOff( channel, m_tracks[0].nextByte() & 0x7f );
        break;

      case 1:
      { // note on
        auto tmp = m_tracks[0].nextByte();
        auto vol = m_channelVolume[channel];
        if( tmp & 0x80 )
        {
          vol = m_channelVolume[channel] = (m_tracks[0].nextByte() & 0x7f);
        }
        m_chips.noteOn( channel, tmp & 0x7f, vol );
        break;
      }

      case 2:
      { // pitch wheel
        auto data = m_tracks[0].nextByte();
        m_chips.pitchBend( channel, data << 7, data >> 1 );
        pitchIsUsed = true;
        break;
      }

      case 3:
      { // sysevent
        auto sys = m_tracks[0].nextByte() & 0x7f;
        switch( sys )
        {
        case 11:
          m_chips.allNotesOff( channel );
          break;
        case 14:
          interpretControllerInfo( m_tracks[0], true, channel, 121, 0 );
          break;
        }

        break;
      }

      case 4:
      { // control change
        auto c1 = m_tracks[0].nextByte();
        auto c2 = m_tracks[0].nextByte();
        switch( c1 & 0x7f )
        {
        case 0: // patch change:
          m_chips.programChange( channel, c2 & 0x7f );
          break;
        case 3: // volume
          interpretControllerInfo( m_tracks[0], true, channel, 7, c2 & 0x7f );
          break;
        case 4: // pan
          break;
        }

        break;
      }
      case 6: // end of score
        m_tracks[0].active = false;
        --m_activeTracks;
        break;
      default:
        throw std::runtime_error( "Unhandled event type" );
      }

      if( !pitchIsUsed )
      {
        m_chips.pitchBend( channel, 0, 64 );
      } // reset pitch wheel

      if( isLast )
      {
        // read timing information...
        m_tracks[0].delay = m_tracks[0].readDelta();
      }
    }

    m_tracks[0].delay--;

    if( m_activeTracks == 0 )
    {
      resetTracks();
      if( m_loop )
      {
        continue;
      }
      else
      {
        return false;
      }
    }

    break; // if not looped, only once for our only track
  }

  advanceTick();
  return m_activeTracks > 0;
}

void EMidi::allNotesOff()
{
  for( size_t channel = 0; channel < NUM_MIDI_CHANNELS; channel++ )
  {
    m_chips.allNotesOff( channel );
  }
}

void EMidi::setChannelVolume(int channel, int volume)
{
  m_channelVolume[channel] = volume;

  volume *= m_totalVolume;
  volume /= 255;

  m_chips.setChannelVolume( channel, volume );
}

void EMidi::sendChannelVolumes()
{
  for( size_t channel = 0; channel < NUM_MIDI_CHANNELS; channel++ )
  {
    m_chips.setChannelVolume( channel, m_channelVolume[channel] );
  }
}

void EMidi::reset()
{
  allNotesOff();

  for( size_t channel = 0; channel < NUM_MIDI_CHANNELS; channel++ )
  {
    m_chips.controlChange( channel, MultiChips::ControlData::ResetAllControllers );
    m_chips.controlChange( channel, MultiChips::ControlData::RpnMsb, 0 );
    m_chips.controlChange( channel, MultiChips::ControlData::RpnLsb, 0 );
    m_chips.controlChange( channel, MultiChips::ControlData::DataentryMsb, 2 );
    m_chips.controlChange( channel, MultiChips::ControlData::DataentryLsb, 0 );
    m_chips.controlChange( channel, MultiChips::ControlData::Pan, 64 );
    static constexpr auto GenMidiDefaultVolume = 90;
    m_channelVolume[channel] = GenMidiDefaultVolume;
  }

  sendChannelVolumes();
}

EMidi::EMidi(Stream& stream, size_t chipCount, bool stereo)
  : m_chips( chipCount, stereo )
  , m_tracks()
{
  if( !tryLoadMidi( stream ) && !tryLoadMus( stream ) )
  {
    throw std::runtime_error( "Failed to load MID/MUS stream" );
  } // TODO
}

EMidi::~EMidi() = default;

bool EMidi::tryLoadMidi(Stream& stream)
{
  stream.seek( 0 );

  m_loop = false;

  char signature[4];
  stream.read( signature, 4 );
  if( std::strncmp( signature, "MThd", 4 ) != 0 )
  {
    return false;
  }

  uint32_t headersize;
  stream >> headersize;
  ppp::swapEndian( &headersize );

  auto headerPos = stream.pos();

  uint16_t format;
  stream >> format;
  ppp::swapEndian( &format );

  uint16_t numTracks;
  stream >> numTracks;
  ppp::swapEndian( &numTracks );

  stream >> m_division;
  ppp::swapEndian( &m_division );

  if( m_division < 0 )
  {
    // If a SMPTE time division is given, just set to 96 so no errors occur
    m_division = 96;
  }

  if( format > 1 )
  {
    return false;
  }

  stream.seek( headerPos + headersize );

  if( numTracks == 0 )
  {
    return false;
  }

  m_tracks.resize( numTracks );
  for( Track& track: m_tracks )
  {
    stream.read( signature, 4 );
    if( std::strncmp( signature, "MTrk", 4 ) != 0 )
    {
      m_tracks.clear();

      return false;
    }

    uint32_t tracklength;
    stream >> tracklength;
    ppp::swapEndian( &tracklength );

    track.data.resize( tracklength );
    stream.read( track.data.data(), track.data.size() );
  }

  initEmidi();

  resetTracks();

  reset();

  setTempo( 120 );

  m_format = Format::PlainMidi;
  m_chips.useAdlibVolumes( false );
  return true;
}

#pragma pack(push, 1)
struct MusHeader
{
  char id[4];
  uint16_t scoreLen;
  //! @brief Absolute file position
  uint16_t scoreStart;
  uint16_t primaryChannels;
  uint16_t secondaryChannels;
  uint16_t instrumentCount;
  uint16_t dummy;
};
#pragma pack(pop)

bool EMidi::tryLoadMus(Stream& stream)
{
  stream.seek( 0 );

  m_loop = false;

  MusHeader header;
  stream >> header;

  if( std::strncmp( header.id, "MUS\x1a", 4 ) != 0 )
  {
    return false;
  }

  m_division = 60; // TODO check
  m_tracks.resize( 1 );

  stream.seek( header.scoreStart );
  m_tracks[0].data.resize( header.scoreLen );
  stream.read( m_tracks[0].data.data(), m_tracks[0].data.size() );

  //initEmidi();

  m_tracks[0].includeTrack = true;
  resetTracks();

  reset();

  setTempo( 140 );

  m_format = Format::IdMus;
  m_chips.useAdlibVolumes( true );
  return true;
}

void EMidi::setTempo(int tempo)
{
  m_ticksPerSecond = (tempo * m_division) / 60;
}

void EMidi::initEmidi()
{
  resetTracks();

  for( Track& track: m_tracks )
  {
    m_timing = Timing();
    m_timing.ticksPerBeat = m_division;

    m_positionInTicks = 0;
    m_activeTracks = 0;
    m_context = -1;

    track.runningStatus = -1;
    track.active = true;

    track.programChange = false;
    track.volumeChange = false;
    track.includeTrack = true;

    // std::memset( track.context, 0, sizeof( track.context ) );

    while( track.delay > 0 )
    {
      advanceTick();
      track.delay--;
    }

    bool includeFound = false;
    while( track.active )
    {
      if( track.dataPos >= track.data.size() )
      {
        track.active = false;
        break;
      }
      auto event = track.nextByte();

      if( ((event >> 4) & 0x0f) == MIDI_SPECIAL )
      {
        switch( event )
        {
        case MIDI_SYSEX:
        case MIDI_SYSEX_CONTINUE:
          track.sysEx();
          break;

        case MIDI_META_EVENT:
          metaEvent( track );
          break;
        default:
          throw std::runtime_error( "Unhandled event" );
        }

        if( track.active )
        {
          track.delay = track.readDelta();
          while( track.delay > 0 )
          {
            advanceTick();
            track.delay--;
          }
        }

        continue;
      }

      if( event & MIDI_RUNNING_STATUS )
      {
        track.runningStatus = event;
      }
      else
      {
        BOOST_ASSERT( track.runningStatus != -1 );
        event = track.runningStatus;
        track.dataPos--;
      }

      auto command = (event >> 4) & 0x0f;
      BOOST_ASSERT( command >= 8 );
      int length = commandLengths[command];

      if( command == MIDI_CONTROL_CHANGE )
      {
        if( track.currentByte() == MIDI_MONO_MODE_ON )
        {
          length++;
        }
        uint8_t c1 = 0, c2 = 0;
        if( length > 0 )
        {
          c1 = track.nextByte();
          --length;
        }
        if( length > 0 )
        {
          c2 = track.nextByte();
          --length;
        }

        switch( c1 )
        {
        case EMIDI_LOOP_START:
        case EMIDI_SONG_LOOP_START:
          if( c2 == 0 )
          {
            track.contexts[0].loopcount = SongContext::InfiniteLoop;
          }
          else
          {
            track.contexts[0].loopcount = c2;
          }

          track.contexts[0].pos = track.dataPos;
          track.contexts[0].loopstart = track.dataPos;
          track.contexts[0].runningStatus = track.runningStatus;
          track.contexts[0].timing = m_timing;
          break;

        case EMIDI_LOOP_END:
        case EMIDI_SONG_LOOP_END:
          if( c2 == EMIDI_END_LOOP_VALUE )
          {
            track.contexts[0].loopstart = SongContext::Unlooped;
            track.contexts[0].loopcount = 0;
          }
          break;

        case EMIDI_INCLUDE_TRACK:
          if( EMIDI_AffectsCurrentCard( c2, EMIDI_Adlib ) )
          {
            //printf( "Include track %d on card %d\n", tracknum, c2 );
            includeFound = true;
            track.includeTrack = true;
          }
          else if( !includeFound )
          {
            //printf( "Track excluded %d on card %d\n", tracknum, c2 );
            includeFound = true;
            track.includeTrack = false;
          }
          break;

        case EMIDI_EXCLUDE_TRACK:
          if( EMIDI_AffectsCurrentCard( c2, EMIDI_Adlib ) )
          {
            //printf( "Exclude track %d on card %d\n", tracknum, c2 );
            track.includeTrack = false;
          }
          break;

        case EMIDI_PROGRAM_CHANGE:
          track.programChange = true;
          break;

        case EMIDI_VOLUME_CHANGE:
          track.volumeChange = true;
          break;

        case EMIDI_CONTEXT_START:
          if( c2 > 0 && c2 < track.contexts.size() )
          {
            track.contexts[c2].pos = track.dataPos;
            track.contexts[c2].loopstart = track.contexts[0].loopstart;
            track.contexts[c2].loopcount = track.contexts[0].loopcount;
            track.contexts[c2].runningStatus = track.runningStatus;
            track.contexts[c2].timing = m_timing;
          }
          break;

        case EMIDI_CONTEXT_END:
          break;
        }
      }

      BOOST_ASSERT( length >= 0 );
      track.dataPos += length;
      track.delay = track.readDelta();

      while( track.delay > 0 )
      {
        advanceTick();
        track.delay--;
      }
    }
  }

  resetTracks();
}
}