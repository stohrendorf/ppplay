#include "almidi.h"

#include "stuff/numberutils.h"

#include <algorithm>
#include <cstring>

namespace ppp {

constexpr auto NUM_MIDI_CHANNELS = 16;

#define NUM_VOICES      9

#define EMIDI_INFINITE          -1
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

#define EMIDI_AffectsCurrentCard( c, type ) \
    ( ( ( c ) == EMIDI_ALL_CARDS ) || ( ( c ) == ( type ) ) )


#define EMIDI_NUM_CONTEXTS      7

struct songcontext
{
    std::size_t pos = 0;
    std::size_t loopstart = -1;
    short          loopcount = 0;
    short          RunningStatus = -1;
    short          tick = 0;
    short          beat = 0;
    short          measure = 0;
    short          BeatsPerMeasure = 0;
    short          TicksPerBeat = 0;
    short          TimeBase = 0;
    long           delay = 0;
    bool active = false;
};

namespace {
static constexpr std::array<int,16> commandLengths = {
    0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 2, 0
};
}

struct EMidi::Track
{
    std::size_t dataPos = 0;
    std::vector<uint8_t> data{};

    uint8_t nextByte() {
        BOOST_ASSERT( dataPos < data.size() );
        return data[dataPos++];
    }
    uint8_t currentByte() {
        BOOST_ASSERT( dataPos < data.size() );
        return data[dataPos];
    }

    uint32_t readDelta() {
        uint32_t value = 0;

        uint8_t c = 0;
        do {
            c = nextByte();
            value = ( value << 7 ) | ( c & 0x7f );
        } while( c & 0x80 );

        return value;
    }

    void sysEx() {
        dataPos += readDelta();
    }

    long           delay = 0;
    bool active = false;
    short          RunningStatus = -1;

    short          currentcontext = 0;
    songcontext    context[ EMIDI_NUM_CONTEXTS ];

    bool           EMIDI_IncludeTrack = false;
    bool           EMIDI_ProgramChange = false;
    bool           EMIDI_VolumeChange = false;
};

void EMidi::resetTracks()
{
    m_tick = 0;
    m_beat = 1;
    m_measure = 1;
    m_beatsPerMeasure = 4;
    m_ticksPerBeat = m_division;
    m_timeBase = 4;

    m_positionInTicks = 0;
    m_activeTracks    = 0;
    m_context         = 0;

    auto ptr = m_trackPtr;
    for( int i = 0; i < m_numTracks; i++ )
    {
        ptr->dataPos = 0;
        ptr->delay = ptr->readDelta();
        ptr->active                 = ptr->EMIDI_IncludeTrack;
        ptr->RunningStatus          = -1;
        ptr->currentcontext         = 0;
        ptr->context[ 0 ].loopstart = -1;
        ptr->context[ 0 ].loopcount = 0;

        if ( ptr->active )
            m_activeTracks++;

        ptr++;
    }
}

void EMidi::advanceTick()
{
    m_positionInTicks++;

    m_tick++;
    while( m_tick > m_ticksPerBeat )
    {
        m_tick -= m_ticksPerBeat;
        m_beat++;
    }
    while( m_beat > m_beatsPerMeasure )
    {
        m_beat -= m_beatsPerMeasure;
        m_measure++;
    }
}

void EMidi::metaEvent(EMidi::Track* Track)
{
    auto command = Track->nextByte();
    auto length = Track->readDelta();

#define MIDI_END_OF_TRACK          0x2F
#define MIDI_TEMPO_CHANGE          0x51
#define MIDI_TIME_SIGNATURE        0x58

    switch( command ) {
    case MIDI_END_OF_TRACK :
        Track->active = false;
        m_activeTracks--;
        break;

    case MIDI_TEMPO_CHANGE : {
        BOOST_ASSERT(length>=3);
        uint32_t num = Track->nextByte() << 16;
        num |= Track->nextByte()<<8;
        num |= Track->nextByte();
        length -= 3;
        setTempo( 60000000L / num );
        break;
    }

    case MIDI_TIME_SIGNATURE :
        BOOST_ASSERT(length>=2);
        if ( ( m_tick > 0 ) || ( m_beat > 1 ) ) {
            m_measure++;
        }

        m_tick = 0;
        m_beat = 1;

        m_beatsPerMeasure = Track->nextByte();
        m_timeBase = 1 << Track->nextByte();
        length -= 2;
        m_ticksPerBeat = ( m_division * 4 ) / m_timeBase;
        break;
    default:
        break;
    }

    Track->dataPos += length;
}

bool EMidi::interpretControllerInfo ( EMidi::Track *track, bool TimeSet, int channel, int c1, int c2 )
{
    BOOST_ASSERT( channel>=0 && channel<16 );
    Track *trackptr;
    int tracknum;
    int loopcount;

#define MIDI_VOLUME                7
#define MIDI_PAN                   10
#define MIDI_DETUNE                94
#define MIDI_RHYTHM_CHANNEL        9
#define MIDI_RUNNING_STATUS        0x80

#define MIDI_NOTE_OFF              0x8
#define MIDI_NOTE_ON               0x9
#define MIDI_CONTROL_CHANGE        0xB
#define MIDI_PROGRAM_CHANGE        0xC
#define MIDI_PITCH_BEND            0xE
#define MIDI_SPECIAL               0xF

#define MIDI_SYSEX                 0xF0
#define MIDI_SYSEX_CONTINUE        0xF7
#define MIDI_META_EVENT            0xFF
#define MIDI_RESET_ALL_CONTROLLERS 0x79
#define MIDI_ALL_NOTES_OFF         0x7b
#define MIDI_MONO_MODE_ON          0x7E
#define MIDI_SYSTEM_RESET          0xFF

    switch( c1 ) {
    case MIDI_MONO_MODE_ON :
        track->dataPos++;
        break;

    case MIDI_VOLUME :
        if ( !track->EMIDI_VolumeChange ) {
            setChannelVolume( channel, c2 );
        }
        break;

    case EMIDI_INCLUDE_TRACK :
    case EMIDI_EXCLUDE_TRACK :
        break;

    case EMIDI_PROGRAM_CHANGE :
        if ( track->EMIDI_ProgramChange ) {
            m_chips.programChange( channel, c2 & 0x7f );
        }
        break;

    case EMIDI_VOLUME_CHANGE :
        if ( track->EMIDI_VolumeChange ) {
            setChannelVolume( channel, c2 );
        }
        break;

    case EMIDI_CONTEXT_START :
        break;

    case EMIDI_CONTEXT_END :
        if ( ( track->currentcontext == m_context ) ||
             ( m_context < 0 ) ||
             ( track->context[ m_context ].pos == 0 ) )
        {
            break;
        }

        track->currentcontext = m_context;
        track->context[ 0 ].loopstart = track->context[ m_context ].loopstart;
        track->context[ 0 ].loopcount = track->context[ m_context ].loopcount;
        track->dataPos           = track->context[ m_context ].pos;
        track->RunningStatus = track->context[ m_context ].RunningStatus;

        if ( TimeSet )
            break;

        m_tick             = track->context[ m_context ].tick;
        m_beat             = track->context[ m_context ].beat;
        m_measure          = track->context[ m_context ].measure;
        m_beatsPerMeasure  = track->context[ m_context ].BeatsPerMeasure;
        m_ticksPerBeat     = track->context[ m_context ].TicksPerBeat;
        m_timeBase         = track->context[ m_context ].TimeBase;
        TimeSet = true;
        break;

    case EMIDI_LOOP_START :
    case EMIDI_SONG_LOOP_START :
        if ( c2 == 0 ) {
            loopcount = EMIDI_INFINITE;
        }
        else {
            loopcount = c2;
        }

        if ( c1 == EMIDI_SONG_LOOP_START ) {
            trackptr = m_trackPtr;
            tracknum = m_numTracks;
        }
        else {
            trackptr = track;
            tracknum = 1;
        }

        while( tracknum > 0 ) {
            trackptr->context[ 0 ].loopcount        = loopcount;
            trackptr->context[ 0 ].pos              = trackptr->dataPos;
            trackptr->context[ 0 ].loopstart        = trackptr->dataPos;
            trackptr->context[ 0 ].RunningStatus    = trackptr->RunningStatus;
            trackptr->context[ 0 ].active           = trackptr->active;
            trackptr->context[ 0 ].delay            = trackptr->delay;
            trackptr->context[ 0 ].tick             = m_tick;
            trackptr->context[ 0 ].beat             = m_beat;
            trackptr->context[ 0 ].measure          = m_measure;
            trackptr->context[ 0 ].BeatsPerMeasure  = m_beatsPerMeasure;
            trackptr->context[ 0 ].TicksPerBeat     = m_ticksPerBeat;
            trackptr->context[ 0 ].TimeBase         = m_timeBase;
            trackptr++;
            tracknum--;
        }
        break;

    case EMIDI_LOOP_END :
    case EMIDI_SONG_LOOP_END :
        if ( ( c2 != EMIDI_END_LOOP_VALUE ) ||
             ( track->context[ 0 ].loopstart == -1 ) ||
             ( track->context[ 0 ].loopcount == 0 ) )
        {
            break;
        }

        if ( c1 == EMIDI_SONG_LOOP_END ) {
            trackptr = m_trackPtr;
            tracknum = m_numTracks;
            m_activeTracks = 0;
        }
        else {
            trackptr = track;
            tracknum = 1;
            m_activeTracks--;
        }

        while( tracknum > 0 ) {
            if ( trackptr->context[ 0 ].loopcount != EMIDI_INFINITE ) {
                trackptr->context[ 0 ].loopcount--;
            }

            BOOST_ASSERT( trackptr->context[ 0 ].loopstart != -1 );
            trackptr->dataPos = trackptr->context[ 0 ].loopstart;
            trackptr->RunningStatus = trackptr->context[ 0 ].RunningStatus;
            trackptr->delay         = trackptr->context[ 0 ].delay;
            trackptr->active        = trackptr->context[ 0 ].active;
            if ( trackptr->active )
            {
                m_activeTracks++;
            }

            if ( !TimeSet ) {
                m_tick             = trackptr->context[ 0 ].tick;
                m_beat             = trackptr->context[ 0 ].beat;
                m_measure          = trackptr->context[ 0 ].measure;
                m_beatsPerMeasure  = trackptr->context[ 0 ].BeatsPerMeasure;
                m_ticksPerBeat     = trackptr->context[ 0 ].TicksPerBeat;
                m_timeBase         = trackptr->context[ 0 ].TimeBase;
                TimeSet = true;
            }

            trackptr++;
            tracknum--;
        }
        break;
    case MIDI_PAN:
        // not implemented m_chips.setChannelPan( channel, c2 );
        break;
    case MIDI_DETUNE:
        m_chips.setChannelDetune( channel, c2 );
        break;
    case MIDI_ALL_NOTES_OFF:
        m_chips.allNotesOff(channel);
        break;
    case MIDI_RESET_ALL_CONTROLLERS:
        m_chips.controlChange( channel, DualChips::ControlData::ResetAllControllers );
        break;
    case 100:
        m_chips.controlChange( channel, DualChips::ControlData::RpnMsb, c2 );
        break;
    case 101:
        m_chips.controlChange( channel, DualChips::ControlData::RpnLsb, c2 );
        break;
    case 6:
        m_chips.controlChange( channel, DualChips::ControlData::DataentryMsb, c2 );
        break;
    case 38:
        m_chips.controlChange( channel, DualChips::ControlData::DataentryLsb, c2 );
        break;
    }

    return TimeSet;
}

bool EMidi::serviceRoutine()
{
    switch(m_format) {
    case Format::PlainMidi:
        return serviceRoutineMidi();
    case Format::IdMus:
        return serviceRoutineMus();
    }
}

bool EMidi::serviceRoutineMidi()
{
    bool   TimeSet = false;

    auto Track = m_trackPtr;
    auto tracknum = 0;
    while( tracknum < m_numTracks ) {
        while( Track->active && Track->delay == 0 ) {
            if( Track->dataPos >= Track->data.size()) {
                Track->active = false;
                break;
            }
            auto event = Track->nextByte();

            if ( event & MIDI_RUNNING_STATUS ) {
                Track->RunningStatus = event;
            }
            else {
                BOOST_ASSERT(Track->RunningStatus != -1);
                event = Track->RunningStatus;
                Track->dataPos--;
            }

            if ( ( (event>>4)&0x0f ) == MIDI_SPECIAL ) {
                switch( event ) {
                case MIDI_SYSEX :
                case MIDI_SYSEX_CONTINUE :
                    Track->sysEx();
                    break;

                case MIDI_META_EVENT :
                    metaEvent( Track );
                    break;
                }

                if ( Track->active ) {
                    Track->delay = Track->readDelta();
                }

                continue;
            }

            auto channel = event&0x0f;
            auto command = (event>>4)&0x0f;

            BOOST_ASSERT(command>=8);
            int c1=0, c2=0;
            if ( commandLengths[ command ] > 0 ) {
                c1 = Track->nextByte();
                if ( commandLengths[ command ] > 1 ) {
                    c2 = Track->nextByte();
                }
            }

            switch ( command ) {
            case MIDI_NOTE_OFF :
                m_chips.noteOff( channel, c1 );
                break;

            case MIDI_NOTE_ON :
                m_chips.noteOn( channel, c1, c2 );
                break;

            case MIDI_CONTROL_CHANGE :
                TimeSet = interpretControllerInfo( Track, TimeSet, channel, c1, c2 );
                break;

            case MIDI_PROGRAM_CHANGE :
                if ( !Track->EMIDI_ProgramChange ) {
                    m_chips.programChange( channel, c1 & 0x7f );
                }
                break;

            case MIDI_PITCH_BEND:
                m_chips.pitchBend(channel, c1, c2);
                break;

            default :
                break;
            }

            Track->delay = Track->readDelta();
        }

        Track->delay--;
        Track++;
        tracknum++;

        if ( m_activeTracks == 0 ) {
            resetTracks();
            if ( m_loop ) {
                tracknum = 0;
                Track = m_trackPtr;
            }
            else {
                return false;
            }
        }
    }

    advanceTick();
    return true;
}

bool EMidi::serviceRoutineMus()
{
    // This loop isn't endless; it's only used as a "goto-less goto"
    // if looping is enabled, as there's a "break" at the very end.
    while( true ) {
        while( m_trackPtr[0].active && m_trackPtr[0].delay == 0 ) {
            if( m_trackPtr[0].dataPos >= m_trackPtr[0].data.size()) {
                m_trackPtr[0].active = false;
                break;
            }

            auto event = m_trackPtr[0].nextByte();

            auto channel = event & 0x0f;
            // swap channels 9 and 15 to use MIDI's percussion channel handling
            if( channel==9 )
                channel = 15;
            else if( channel==15 )
                channel = 9;

            const auto eventType = (event>>4)&7;
            const bool isLast = (event & 0x80) != 0;

            switch ( eventType ) {
            case 0: // note off
                m_chips.noteOff( channel, m_trackPtr[0].nextByte() & 0x7f );
                break;

            case 1: { // note on
                auto tmp = m_trackPtr[0].nextByte();
                auto vol = m_channelVolume[channel];
                if( tmp&0x80 ) {
                    vol = m_channelVolume[channel] = (m_trackPtr[0].nextByte() & 0x7f);
                }
                m_chips.noteOn( channel, tmp & 0x7f, vol );
                break;
            }

            case 2: { // pitch wheel
                m_chips.pitchBend(channel, 0, m_trackPtr[0].nextByte() ); // TODO
                break;
            }

            case 3: // TODO sysevent
                break;

            case 4: { // control change
                auto c1 = m_trackPtr[0].nextByte();
                auto c2 = m_trackPtr[0].nextByte();
                switch(c1 & 0x7f) {
                case 0: // patch change:
                    m_chips.programChange( channel, c2 & 0x7f );
                    break;
                case 3: // volume
                    interpretControllerInfo( m_trackPtr, true, channel, 7, c2 & 0x7f );
                    break;
                }

                break;
            }
            }

            if( isLast ) {
                // read timing information...
                m_trackPtr[0].delay = m_trackPtr[0].readDelta();
            }
        }

        m_trackPtr[0].delay--;

        if ( m_activeTracks == 0 ) {
            resetTracks();
            if ( m_loop ) {
                continue;
            }
            else {
                return false;
            }
        }

        break; // if not looped, only once for our only track
    }

    advanceTick();
    return true;
}

void EMidi::allNotesOff()
{
    for( size_t channel = 0; channel < NUM_MIDI_CHANNELS; channel++ )
        m_chips.allNotesOff(channel);
}

void EMidi::setChannelVolume( int channel, int volume )
{
    m_channelVolume[ channel ] = volume;

    volume *= m_totalVolume;
    volume /= 255;

    m_chips.setChannelVolume( channel, volume );
}

void EMidi::sendChannelVolumes()
{
    for( size_t channel = 0; channel < NUM_MIDI_CHANNELS; channel++ )
        m_chips.setChannelVolume( channel, m_channelVolume[ channel ] );
}

void EMidi::reset()
{
    allNotesOff();

    for( size_t channel = 0; channel < NUM_MIDI_CHANNELS; channel++ ) {
        m_chips.controlChange( channel, DualChips::ControlData::ResetAllControllers );
        m_chips.controlChange( channel, DualChips::ControlData::RpnMsb, 0 );
        m_chips.controlChange( channel, DualChips::ControlData::RpnLsb, 0 );
        m_chips.controlChange( channel, DualChips::ControlData::DataentryMsb, 2 );
        m_chips.controlChange( channel, DualChips::ControlData::DataentryLsb, 0 );
        static constexpr auto GenMidiDefaultVolume = 90;
        m_channelVolume[ channel ] = GenMidiDefaultVolume;
    }

    sendChannelVolumes();
}

void EMidi::setVolume ( int volume )
{
    volume = std::min( 255, volume );
    volume = std::max( 0, volume );

    m_totalVolume = volume;

    sendChannelVolumes();
}

EMidi::EMidi(Stream &stream)
{
    if(!tryLoadMidi(stream) && !tryLoadMus(stream))
        throw std::runtime_error("xxx"); // TODO
}

EMidi::~EMidi() {
    delete[] m_trackPtr;
}


bool EMidi::tryLoadMidi(Stream &stream)
{
    stream.seek(0);

    m_loop = false;

    char signature[4];
    stream.read(signature, 4);
    if ( std::strncmp(signature, "MThd", 4) != 0 ) {
        return false;
    }

    uint32_t headersize;
    stream.read(&headersize);
    ppp::swapEndian(&headersize);

    auto headerPos = stream.pos();

    uint16_t format;
    stream.read(&format);
    ppp::swapEndian(&format);

    stream.read(&m_numTracks);
    ppp::swapEndian(&m_numTracks);

    stream.read(&m_division);
    ppp::swapEndian(&m_division);

    if ( m_division < 0 ) {
        // If a SMPTE time division is given, just set to 96 so no errors occur
        m_division = 96;
    }

    if ( format > 1 ) {
        return false;
    }

    stream.seek( headerPos + headersize );

    if ( m_numTracks == 0 ) {
        return false;
    }

    m_trackPtr = new Track[m_numTracks];
    auto CurrentTrack = m_trackPtr;
    auto numtracks    = m_numTracks;
    while( numtracks-- ) {
        stream.read(signature, 4);
        if( std::strncmp(signature, "MTrk", 4) != 0 ) {
            delete[] m_trackPtr;
            m_trackPtr = nullptr;

            return false;
        }

        uint32_t tracklength;
        stream.read(&tracklength);
        ppp::swapEndian(&tracklength);

        CurrentTrack->data.resize(tracklength);
        stream.read(CurrentTrack->data.data(), CurrentTrack->data.size());
        CurrentTrack++;
    }

    initEmidi();

    resetTracks();

    reset();

    setTempo( 120 );

    m_format = Format::PlainMidi;
    return true;
}

#pragma pack(push,1)
struct MusHeader {
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

bool EMidi::tryLoadMus(Stream &stream)
{
    stream.seek(0);

    m_loop = false;

    MusHeader header;
    stream.read(&header);

    if ( std::strncmp(header.id, "MUS\x1a", 4) != 0 ) {
        return false;
    }

    m_division = 60; // TODO check
    m_numTracks = 1;
    m_trackPtr = new Track[m_numTracks];

    stream.seek(header.scoreStart);
    m_trackPtr[0].data.resize(header.scoreLen);
    stream.read(m_trackPtr[0].data.data(), m_trackPtr[0].data.size());

    //initEmidi();

    m_trackPtr[0].EMIDI_IncludeTrack = true;
    resetTracks();

    reset();

    setTempo( 140 );

    m_format = Format::IdMus;
    return true;
}

void EMidi::setTempo ( int tempo )
{
    m_tempo = tempo;
    m_ticksPerSecond = ( tempo * m_division ) / 60;
}


void EMidi::initEmidi()
{
    resetTracks();

    Track* Track = m_trackPtr;
    int tracknum = 0;
    while( tracknum < m_numTracks && Track != nullptr ) {
        m_tick = 0;
        m_beat = 1;
        m_measure = 1;
        m_beatsPerMeasure = 4;
        m_ticksPerBeat = m_division;
        m_timeBase = 4;

        m_positionInTicks = 0;
        m_activeTracks    = 0;
        m_context         = -1;

        Track->RunningStatus = -1;
        Track->active        = true;

        Track->EMIDI_ProgramChange = false;
        Track->EMIDI_VolumeChange  = false;
        Track->EMIDI_IncludeTrack  = true;

        // std::memset( Track->context, 0, sizeof( Track->context ) );

        while( Track->delay > 0 ) {
            advanceTick();
            Track->delay--;
        }

        bool IncludeFound = false;
        while ( Track->active ) {
            if( Track->dataPos >= Track->data.size() ) {
                Track->active = false;
                break;
            }
            auto event = Track->nextByte();

            if ( ( (event>>4)&0x0f ) == MIDI_SPECIAL ) {
                switch( event )
                {
                case MIDI_SYSEX :
                case MIDI_SYSEX_CONTINUE :
                    Track->sysEx();
                    break;

                case MIDI_META_EVENT :
                    metaEvent( Track );
                    break;
                }

                if ( Track->active ) {
                    Track->delay = Track->readDelta();
                    while( Track->delay > 0 ) {
                        advanceTick();
                        Track->delay--;
                    }
                }

                continue;
            }


            if ( event & MIDI_RUNNING_STATUS ) {
                Track->RunningStatus = event;
            }
            else {
                BOOST_ASSERT(Track->RunningStatus != -1);
                event = Track->RunningStatus;
                Track->dataPos--;
            }

            auto command = (event>>4)&0x0f;
            BOOST_ASSERT(command>=8);
            int length = commandLengths[ command ];

            if ( command == MIDI_CONTROL_CHANGE ) {
                if ( Track->currentByte() == MIDI_MONO_MODE_ON )
                    length++;
                int c1=0, c2=0;
                if(length>0) {
                    c1 = Track->nextByte();
                    --length;
                }
                if(length>0) {
                    c2 = Track->nextByte();
                    --length;
                }

                switch( c1 ) {
                case EMIDI_LOOP_START :
                case EMIDI_SONG_LOOP_START :
                    if ( c2 == 0 ) {
                        Track->context[ 0 ].loopcount = EMIDI_INFINITE;
                    }
                    else {
                        Track->context[ 0 ].loopcount = c2;
                    }

                    Track->context[ 0 ].pos              = Track->dataPos;
                    Track->context[ 0 ].loopstart        = Track->dataPos;
                    Track->context[ 0 ].RunningStatus    = Track->RunningStatus;
                    Track->context[ 0 ].tick             = m_tick;
                    Track->context[ 0 ].beat             = m_beat;
                    Track->context[ 0 ].measure          = m_measure;
                    Track->context[ 0 ].BeatsPerMeasure  = m_beatsPerMeasure;
                    Track->context[ 0 ].TicksPerBeat     = m_ticksPerBeat;
                    Track->context[ 0 ].TimeBase         = m_timeBase;
                    break;

                case EMIDI_LOOP_END :
                case EMIDI_SONG_LOOP_END :
                    if ( c2 == EMIDI_END_LOOP_VALUE ) {
                        Track->context[ 0 ].loopstart = -1;
                        Track->context[ 0 ].loopcount = 0;
                    }
                    break;

                case EMIDI_INCLUDE_TRACK :
                    if ( EMIDI_AffectsCurrentCard( c2, EMIDI_Adlib ) ) {
                        //printf( "Include track %d on card %d\n", tracknum, c2 );
                        IncludeFound = true;
                        Track->EMIDI_IncludeTrack = true;
                    }
                    else if ( !IncludeFound ) {
                        //printf( "Track excluded %d on card %d\n", tracknum, c2 );
                        IncludeFound = true;
                        Track->EMIDI_IncludeTrack = false;
                    }
                    break;

                case EMIDI_EXCLUDE_TRACK :
                    if ( EMIDI_AffectsCurrentCard( c2, EMIDI_Adlib ) ) {
                        //printf( "Exclude track %d on card %d\n", tracknum, c2 );
                        Track->EMIDI_IncludeTrack = false;
                    }
                    break;

                case EMIDI_PROGRAM_CHANGE :
                    if ( !Track->EMIDI_ProgramChange ) {
                        //printf( "Program change on track %d\n", tracknum );
                        Track->EMIDI_ProgramChange = true;
                    }
                    break;

                case EMIDI_VOLUME_CHANGE :
                    if ( !Track->EMIDI_VolumeChange ) {
                        //printf( "Volume change on track %d\n", tracknum );
                        Track->EMIDI_VolumeChange = true;
                    }
                    break;

                case EMIDI_CONTEXT_START :
                    if ( ( c2 > 0 ) && ( c2 < EMIDI_NUM_CONTEXTS ) ) {
                        Track->context[ c2 ].pos              = Track->dataPos;
                        Track->context[ c2 ].loopstart        = Track->context[ 0 ].loopstart;
                        Track->context[ c2 ].loopcount        = Track->context[ 0 ].loopcount;
                        Track->context[ c2 ].RunningStatus    = Track->RunningStatus;
                        Track->context[ c2 ].tick             = m_tick;
                        Track->context[ c2 ].beat             = m_beat;
                        Track->context[ c2 ].measure          = m_measure;
                        Track->context[ c2 ].BeatsPerMeasure  = m_beatsPerMeasure;
                        Track->context[ c2 ].TicksPerBeat     = m_ticksPerBeat;
                        Track->context[ c2 ].TimeBase         = m_timeBase;
                    }
                    break;

                case EMIDI_CONTEXT_END :
                    break;
                }
            }

            BOOST_ASSERT(length>=0);
            Track->dataPos += length;
            Track->delay = Track->readDelta();

            while( Track->delay > 0 ) {
                advanceTick();
                Track->delay--;
            }
        }

        Track++;
        tracknum++;
    }

    resetTracks();
}

}
