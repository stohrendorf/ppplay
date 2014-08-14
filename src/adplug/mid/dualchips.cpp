#include "dualchips.h"

#include <iostream>

namespace ppp
{

namespace
{
// Slot numbers as a function of the voice and the operator.
// ( melodic only)

constexpr std::array<std::array<uint8_t,2>,9> voiceSlotOffsets
{{
        { 0, 3 },    // voice 0
        { 1, 4 },    // 1
        { 2, 5 },    // 2
        { 6, 9 },    // 3
        { 7, 10 },   // 4
        { 8, 11 },   // 5
        { 12, 15 },  // 6
        { 13, 16 },  // 7
        { 14, 17 },  // 8
    }};

// This table gives the offset of each slot within the chip.
// offset = fn( slot)
constexpr std::array<uint8_t,18> slotRegisterOffsets
{{
        0,  1,  2,  3,  4,  5,
        8,  9, 10, 11, 12, 13,
        16, 17, 18, 19, 20, 21
    }};

constexpr std::array<std::array<uint16_t,12>, 32> NotePitch
   {{
      { 0x157, 0x16b, 0x181, 0x198, 0x1b0, 0x1ca, 0x1e5, 0x202, 0x220, 0x241, 0x263, 0x287 },
      { 0x157, 0x16b, 0x181, 0x198, 0x1b0, 0x1ca, 0x1e5, 0x202, 0x220, 0x242, 0x264, 0x288 },
      { 0x158, 0x16c, 0x182, 0x199, 0x1b1, 0x1cb, 0x1e6, 0x203, 0x221, 0x243, 0x265, 0x289 },
      { 0x158, 0x16c, 0x183, 0x19a, 0x1b2, 0x1cc, 0x1e7, 0x204, 0x222, 0x244, 0x266, 0x28a },
      { 0x159, 0x16d, 0x183, 0x19a, 0x1b3, 0x1cd, 0x1e8, 0x205, 0x223, 0x245, 0x267, 0x28b },
      { 0x15a, 0x16e, 0x184, 0x19b, 0x1b3, 0x1ce, 0x1e9, 0x206, 0x224, 0x246, 0x268, 0x28c },
      { 0x15a, 0x16e, 0x185, 0x19c, 0x1b4, 0x1ce, 0x1ea, 0x207, 0x225, 0x247, 0x269, 0x28e },
      { 0x15b, 0x16f, 0x185, 0x19d, 0x1b5, 0x1cf, 0x1eb, 0x208, 0x226, 0x248, 0x26a, 0x28f },
      { 0x15b, 0x170, 0x186, 0x19d, 0x1b6, 0x1d0, 0x1ec, 0x209, 0x227, 0x249, 0x26b, 0x290 },
      { 0x15c, 0x170, 0x187, 0x19e, 0x1b7, 0x1d1, 0x1ec, 0x20a, 0x228, 0x24a, 0x26d, 0x291 },
      { 0x15d, 0x171, 0x188, 0x19f, 0x1b7, 0x1d2, 0x1ed, 0x20b, 0x229, 0x24b, 0x26e, 0x292 },
      { 0x15d, 0x172, 0x188, 0x1a0, 0x1b8, 0x1d3, 0x1ee, 0x20c, 0x22a, 0x24c, 0x26f, 0x293 },
      { 0x15e, 0x172, 0x189, 0x1a0, 0x1b9, 0x1d4, 0x1ef, 0x20d, 0x22b, 0x24d, 0x270, 0x295 },
      { 0x15f, 0x173, 0x18a, 0x1a1, 0x1ba, 0x1d4, 0x1f0, 0x20e, 0x22c, 0x24e, 0x271, 0x296 },
      { 0x15f, 0x174, 0x18a, 0x1a2, 0x1bb, 0x1d5, 0x1f1, 0x20f, 0x22d, 0x24f, 0x272, 0x297 },
      { 0x160, 0x174, 0x18b, 0x1a3, 0x1bb, 0x1d6, 0x1f2, 0x210, 0x22e, 0x250, 0x273, 0x298 },
      { 0x161, 0x175, 0x18c, 0x1a3, 0x1bc, 0x1d7, 0x1f3, 0x211, 0x22f, 0x251, 0x274, 0x299 },
      { 0x161, 0x176, 0x18c, 0x1a4, 0x1bd, 0x1d8, 0x1f4, 0x212, 0x230, 0x252, 0x276, 0x29b },
      { 0x162, 0x176, 0x18d, 0x1a5, 0x1be, 0x1d9, 0x1f5, 0x212, 0x231, 0x254, 0x277, 0x29c },
      { 0x162, 0x177, 0x18e, 0x1a6, 0x1bf, 0x1d9, 0x1f5, 0x213, 0x232, 0x255, 0x278, 0x29d },
      { 0x163, 0x178, 0x18f, 0x1a6, 0x1bf, 0x1da, 0x1f6, 0x214, 0x233, 0x256, 0x279, 0x29e },
      { 0x164, 0x179, 0x18f, 0x1a7, 0x1c0, 0x1db, 0x1f7, 0x215, 0x235, 0x257, 0x27a, 0x29f },
      { 0x164, 0x179, 0x190, 0x1a8, 0x1c1, 0x1dc, 0x1f8, 0x216, 0x236, 0x258, 0x27b, 0x2a1 },
      { 0x165, 0x17a, 0x191, 0x1a9, 0x1c2, 0x1dd, 0x1f9, 0x217, 0x237, 0x259, 0x27c, 0x2a2 },
      { 0x166, 0x17b, 0x192, 0x1aa, 0x1c3, 0x1de, 0x1fa, 0x218, 0x238, 0x25a, 0x27e, 0x2a3 },
      { 0x166, 0x17b, 0x192, 0x1aa, 0x1c3, 0x1df, 0x1fb, 0x219, 0x239, 0x25b, 0x27f, 0x2a4 },
      { 0x167, 0x17c, 0x193, 0x1ab, 0x1c4, 0x1e0, 0x1fc, 0x21a, 0x23a, 0x25c, 0x280, 0x2a6 },
      { 0x168, 0x17d, 0x194, 0x1ac, 0x1c5, 0x1e0, 0x1fd, 0x21b, 0x23b, 0x25d, 0x281, 0x2a7 },
      { 0x168, 0x17d, 0x194, 0x1ad, 0x1c6, 0x1e1, 0x1fe, 0x21c, 0x23c, 0x25e, 0x282, 0x2a8 },
      { 0x169, 0x17e, 0x195, 0x1ad, 0x1c7, 0x1e2, 0x1ff, 0x21d, 0x23d, 0x260, 0x283, 0x2a9 },
      { 0x16a, 0x17f, 0x196, 0x1ae, 0x1c8, 0x1e3, 0x1ff, 0x21e, 0x23e, 0x261, 0x284, 0x2ab },
      { 0x16a, 0x17f, 0x197, 0x1af, 0x1c8, 0x1e4, 0x200, 0x21f, 0x23f, 0x262, 0x286, 0x2ac }
   }};

} // anonymous namespace



namespace slotbase
{
static constexpr auto FNumL = 0xa0;
static constexpr auto KonBlockFnumH = 0xb0;
static constexpr auto ChAbcdFbCnt = 0xc0;
}
namespace operatorbase
{
static constexpr auto AmVibEgtKsrMult = 0x20;
static constexpr auto KslTl = 0x40;
static constexpr auto AttackDecay = 0x60;
static constexpr auto SustainRelease = 0x80;
static constexpr auto WaveSelect = 0xe0;
}



void DualChips::applyTimbre(Voice* voice)
{
    auto channel = voice->channel;

    BOOST_ASSERT( channel < m_channels.size() );

    int patch = (channel==9)
              ? voice->key + 128
              : m_channels[ channel ].Timbre;

    if ( voice->timbre == patch  ||  patch<0 || patch>=m_timbreBank.size() )
    {
        return;
    }

    voice->timbre = patch;
    BOOST_ASSERT( patch>=0 && patch<m_timbreBank.size() );
    auto timbre = &m_timbreBank[ patch ];

    const auto arraySelector = voice->arraySelector;
    const auto realSlot = ( voice->num >= voiceSlotOffsets.size() ) ? voice->num - voiceSlotOffsets.size() : voice->num;

    BOOST_ASSERT( realSlot>=0 && realSlot<voiceSlotOffsets.size() );


    // FIRST OPERATOR

    auto slotOffset = voiceSlotOffsets[ realSlot ][ 0 ];
    BOOST_ASSERT( slotOffset>=0 && slotOffset<slotRegisterOffsets.size() );
    auto registerOffset = slotRegisterOffsets[ slotOffset ];
    BOOST_ASSERT( registerOffset>=0 && registerOffset<=21 );

    m_voiceLevel[ slotOffset ][ arraySelector ] = 63 - ( timbre->Level[ 0 ] & 0x3f );
    m_voiceKsl[ slotOffset ][ arraySelector ]   = timbre->Level[ 0 ] & 0xc0;


    send( arraySelector, slotbase::FNumL + realSlot, 0 );
    send( arraySelector, slotbase::KonBlockFnumH + realSlot, 0 );

    // Let voice clear the release
    send( arraySelector, operatorbase::SustainRelease + registerOffset, 0xff );

    send( arraySelector, operatorbase::AttackDecay + registerOffset, timbre->Env1[ 0 ] );
    send( arraySelector, operatorbase::SustainRelease + registerOffset, timbre->Env2[ 0 ] );
    send( arraySelector, operatorbase::AmVibEgtKsrMult + registerOffset, timbre->SAVEK[ 0 ] );
    send( arraySelector, operatorbase::WaveSelect + registerOffset, timbre->Wave[ 0 ] );
    send( arraySelector, operatorbase::KslTl + registerOffset, timbre->Level[ 0 ] );


    // SECOND OPERATOR

    slotOffset = voiceSlotOffsets[ realSlot ][ 1 ];
    BOOST_ASSERT( slotOffset>=0 && slotOffset<slotRegisterOffsets.size() );

    send( arraySelector, slotbase::ChAbcdFbCnt + realSlot, ( timbre->Feedback & 0x0f ) | 0xf0 );
#if 0
    send( Array::First, slotbase::ChAbcdFbCnt + realSlot, ( timbre->Feedback & 0x0f ) | 0x20 );
    send( Array::Second, slotbase::ChAbcdFbCnt + realSlot, ( timbre->Feedback & 0x0f ) | 0x10 );
#endif

    registerOffset = slotRegisterOffsets[ slotOffset ];
    BOOST_ASSERT( registerOffset>=0 && registerOffset<=21 );

    m_voiceLevel[ slotOffset ][ arraySelector ] = 63 - ( timbre->Level[ 1 ] & 0x3f );
    m_voiceKsl[ slotOffset ][ arraySelector ]   = timbre->Level[ 1 ] & 0xc0;
    send( arraySelector, operatorbase::KslTl + registerOffset, 63 );

    // Let voice clear the release
    send( arraySelector, operatorbase::SustainRelease + registerOffset, 0xff );

    send( arraySelector, operatorbase::AttackDecay + registerOffset, timbre->Env1[ 1 ] );
    send( arraySelector, operatorbase::SustainRelease + registerOffset, timbre->Env2[ 1 ] );
    send( arraySelector, operatorbase::AmVibEgtKsrMult + registerOffset, timbre->SAVEK[ 1 ] );
    send( arraySelector, operatorbase::WaveSelect + registerOffset, timbre->Wave[ 1 ] );
    send( arraySelector, operatorbase::KslTl + registerOffset, timbre->Level[ 1 ] );
}

void DualChips::applyVolume(Voice* voice)
{
    auto channel = voice->channel;

    BOOST_ASSERT(voice->timbre != -1);
    const auto timbre = &m_timbreBank[ voice->timbre ];

    const uint velocity = std::min( 127u, voice->velocity /*+ timbre->Velocity*/ );

    const auto realVoice = ( voice->num >= voiceSlotOffsets.size() ) ? voice->num - voiceSlotOffsets.size() : voice->num;
    auto slotOffset = voiceSlotOffsets[ realVoice ][ 1 ];
    const auto arraySelector = voice->arraySelector;


    static constexpr uint8_t volTable[128] = {
       0,  11, 16, 19, 22, 25, 27, 29, 32, 33, 35, 37, 39, 40, 42, 43,
       45, 46, 48, 49, 50, 51, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
       64, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 75, 76, 77,
       78, 79, 80, 80, 81, 82, 83, 83, 84, 85, 86, 86, 87, 88, 89, 89,
       90, 91, 91, 92, 93, 93, 94, 95, 96, 96, 97, 97, 98, 99, 99, 100,
       101, 101, 102, 103, 103, 104, 104, 105, 106, 106, 107, 107, 108,
       109, 109, 110, 110, 111, 112, 112, 113, 113, 114, 114, 115, 115,
       116, 117, 117, 118, 118, 119, 119, 120, 120, 121, 121, 122, 122,
       123, 123, 124, 124, 125, 125, 126, 126, 127
    };


    // amplitude
    uint volume  = m_voiceLevel[ slotOffset ][ arraySelector ];
    volume *= ( velocity + 0x80 );
    volume  = ( m_channels[ channel ].Volume * volume ) >> 14;

    BOOST_ASSERT( volume < 128 );

    //volume ^= 63;
    volume = (volTable[volume]>>1)^0x3f;
    volume |= m_voiceKsl[ slotOffset ][ arraySelector ];

    send( arraySelector, operatorbase::KslTl + slotRegisterOffsets[ slotOffset ], volume );
}

DualChips::Voice *DualChips::allocVoice()
{
    if ( !m_voicePool.empty() )
    {
        auto voice = m_voicePool.begin();
        m_voicePool.erase( voice );
        return *voice;
    }

    return nullptr;
}

DualChips::Voice* DualChips::getVoice(OplChannel* channel, uint8_t key) const
{
    return channel->getVoice(key);
}

void DualChips::applyPitch(Voice* voice)
{
    auto channel = voice->channel;

    int note;
    uint32_t patch;
    if ( channel == 9 ) {
        patch = voice->key + 128;
        note  = m_timbreBank[ patch ].Transpose;
    }
    else {
        patch = m_channels[ channel ].Timbre;
        note  = voice->key + m_timbreBank[ patch ].Transpose;
    }

    note += m_channels[ channel ].KeyOffset - 12;
    if ( note >= 8*12 )
        note = 8*12 - 1;
    else if ( note < 0 )
        note = 0;

    // FIRST ARRAY

    auto detune = m_channels[ channel ].KeyDetune;
    BOOST_ASSERT(detune>=0 && detune<NotePitch.size());

    uint32_t pitch = ((note/12)<<10) | NotePitch[ detune ][ note%12 ];

    voice->pitch = pitch;

    if( voice->isNoteOn )
        pitch |= 0x2000;

    const auto realSlot = ( voice->num >= voiceSlotOffsets.size() ) ? voice->num - voiceSlotOffsets.size() : voice->num;

    send( voice->arraySelector, slotbase::FNumL + realSlot, pitch );
    send( voice->arraySelector, slotbase::KonBlockFnumH + realSlot, pitch >> 8 );
}

void DualChips::setChannelVolume(uint8_t channel, uint8_t volume)
{
    if ( channel >= m_channels.size() )
        return;

    volume = std::max<uint8_t>( 0, volume );
    volume = std::min<uint8_t>( volume, 127 );
    m_channels[ channel ].Volume = volume;

    for(auto voice : m_channels[ channel ].Voices)
        applyVolume( voice );
}

void DualChips::noteOff(uint8_t channel, uint8_t key)
{
    if ( channel >= m_channels.size() )
        return;

    auto voice = m_channels[channel].getVoice(key);

    if ( !voice )
        return;

    voice->isNoteOn = false;

    const auto realSlot = ( voice->num >= voiceSlotOffsets.size() ) ? voice->num - voiceSlotOffsets.size() : voice->num;

    send( voice->arraySelector, slotbase::KonBlockFnumH + realSlot, (voice->pitch>>8)&0xff );

    m_channels[ channel ].Voices.erase( m_channels[channel].Voices.find(voice) );
    m_voicePool.insert( voice );
}

void DualChips::noteOn(uint8_t channel, uint8_t key, int velocity)
{
    if ( channel >= m_channels.size() )
        return;

    if ( velocity == 0 ) {
        noteOff( channel, key );
        return;
    }

    auto voice = allocVoice();

    if ( !voice ) {
        if ( !m_channels[ 9 ].Voices.empty() ) {
            noteOff( 9, (*m_channels[ 9 ].Voices.begin())->key );
            voice = allocVoice();
        }
        if ( !voice ) {
            return;
        }
    }

    voice->key      = key;
    voice->channel  = channel;
    voice->velocity = velocity;
    voice->isNoteOn = true;

    m_channels[ channel ].Voices.insert( voice );

    applyTimbre( voice );
    applyVolume( voice );
    applyPitch( voice );
}

void DualChips::allNotesOff(uint8_t channel)
{
    if ( channel >= m_channels.size() )
        return;

    while( !m_channels[ channel ].Voices.empty() )
        noteOff( channel, (*m_channels[ channel ].Voices.begin())->key );
}

void DualChips::controlChange(uint8_t channel, ControlData type, uint8_t data)
{
    if ( channel >= m_channels.size() )
        return;

    switch( type )
    {
    case ControlData::ResetAllControllers :
        resetVoices();
        setChannelVolume( channel, OplChannel::DefaultChannelVolume );
        setChannelDetune( channel, 0 );
        break;

    case ControlData::RpnMsb :
       m_channels[ channel ].RPN &= 0x00FF;
       m_channels[ channel ].RPN |= uint16_t( data ) << 8;
       break;

    case ControlData::RpnLsb :
       m_channels[ channel ].RPN &= 0xFF00;
       m_channels[ channel ].RPN |= data;
       break;

    case ControlData::DataentryMsb :
       if ( m_channels[ channel ].RPN == 0 ) {
          m_channels[ channel ].PitchBendSemiTones = data;
          m_channels[ channel ].PitchBendRange     =
             m_channels[ channel ].PitchBendSemiTones * 100 +
             m_channels[ channel ].PitchBendHundreds;
          }
       break;

    case ControlData::DataentryLsb :
       if ( m_channels[ channel ].RPN == 0 ) {
          m_channels[ channel ].PitchBendHundreds = data;
          m_channels[ channel ].PitchBendRange    =
             m_channels[ channel ].PitchBendSemiTones * 100 +
             m_channels[ channel ].PitchBendHundreds;
          }
       break;
    }
}

void DualChips::programChange(uint8_t channel, uint8_t patch)
{
    if ( channel >= m_channels.size() )
        return;

    m_channels[ channel ].Timbre  = patch;
}

void DualChips::setChannelDetune(uint8_t channel, int detune)
{
    if ( channel >= m_channels.size() )
        return;

    m_channels[ channel ].Detune = detune;
}

void DualChips::resetVoices()
{
    m_voicePool.clear();

    for( size_t index = 0; index < m_voices.size(); index++ ) {
        m_voices[ index ].num = index;
        m_voices[ index ].arraySelector = ( index < m_voices.size()/2 ) ? Array::First : Array::Second;
        m_voicePool.insert(&m_voices[index]);
    }
}

void DualChips::flushCard(Array port)
{
    for( size_t i = 0 ; i < m_voices.size()/2; i++ ) {
        auto slot1 = slotRegisterOffsets[ voiceSlotOffsets[ i ][ 0 ] ];
        auto slot2 = slotRegisterOffsets[ voiceSlotOffsets[ i ][ 1 ] ];

        send( port, slotbase::FNumL + i, 0 );
        send( port, slotbase::KonBlockFnumH + i, 0 );

        send( port, operatorbase::WaveSelect + slot1, 0 );
        send( port, operatorbase::WaveSelect + slot2, 0 );

        // Set the envelope to be fast and quiet
        send( port, operatorbase::AttackDecay + slot1, 0xff );
        send( port, operatorbase::AttackDecay + slot2, 0xff );
        send( port, operatorbase::SustainRelease + slot1, 0xff );
        send( port, operatorbase::SustainRelease + slot2, 0xff );

        // Maximum attenuation
        send( port, operatorbase::KslTl + slot1, 0xff );
        send( port, operatorbase::KslTl + slot2, 0xff );
    }
}

void DualChips::reset()
{
    m_chip.writeReg(1, 0x20); // LSI test
    m_chip.writeReg(8, 0); // NTS

    // Set the values: AM Depth, VIB depth & Rhythm
    m_chip.writeReg(0xbd, 0); // DAM, DVB, RYT, Percussion
    m_chip.writeReg(0x105, 1); // NEW=1

    flushCard( Array::First );
    flushCard( Array::Second );
}

void DualChips::pitchBend(uint8_t channel, uint8_t lsb, uint8_t msb)
{
    if(channel >= m_channels.size())
        return;

    auto pitchbend = lsb | ( uint16_t(msb) << 8 );
    m_channels[ channel ].Pitchbend = pitchbend;

#define PITCHBEND_CENTER 1638400
#define FINETUNE_RANGE 32

    auto TotalBend  = pitchbend * m_channels[ channel ].PitchBendRange;
    TotalBend /= ( PITCHBEND_CENTER / FINETUNE_RANGE );

    m_channels[ channel ].KeyOffset  = ( int )( TotalBend / FINETUNE_RANGE );
    m_channels[ channel ].KeyOffset -= m_channels[ channel ].PitchBendSemiTones;

    m_channels[ channel ].KeyDetune = ( unsigned )( TotalBend % FINETUNE_RANGE );

    for( auto voice : m_channels[ channel ].Voices ) {
        applyPitch( voice );
    }
}
}
