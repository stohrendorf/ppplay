#include "multichips.h"

#include "stuff/stringutils.h"

namespace ppp
{
namespace
{
constexpr std::array<std::array<uint16_t, 12>, 32> NotePitch
    {{
         {{0x157, 0x16b, 0x181, 0x198, 0x1b0, 0x1ca, 0x1e5, 0x202, 0x220, 0x241, 0x263, 0x287}},
         {{0x157, 0x16b, 0x181, 0x198, 0x1b0, 0x1ca, 0x1e5, 0x202, 0x220, 0x242, 0x264, 0x288}},
         {{0x158, 0x16c, 0x182, 0x199, 0x1b1, 0x1cb, 0x1e6, 0x203, 0x221, 0x243, 0x265, 0x289}},
         {{0x158, 0x16c, 0x183, 0x19a, 0x1b2, 0x1cc, 0x1e7, 0x204, 0x222, 0x244, 0x266, 0x28a}},
         {{0x159, 0x16d, 0x183, 0x19a, 0x1b3, 0x1cd, 0x1e8, 0x205, 0x223, 0x245, 0x267, 0x28b}},
         {{0x15a, 0x16e, 0x184, 0x19b, 0x1b3, 0x1ce, 0x1e9, 0x206, 0x224, 0x246, 0x268, 0x28c}},
         {{0x15a, 0x16e, 0x185, 0x19c, 0x1b4, 0x1ce, 0x1ea, 0x207, 0x225, 0x247, 0x269, 0x28e}},
         {{0x15b, 0x16f, 0x185, 0x19d, 0x1b5, 0x1cf, 0x1eb, 0x208, 0x226, 0x248, 0x26a, 0x28f}},
         {{0x15b, 0x170, 0x186, 0x19d, 0x1b6, 0x1d0, 0x1ec, 0x209, 0x227, 0x249, 0x26b, 0x290}},
         {{0x15c, 0x170, 0x187, 0x19e, 0x1b7, 0x1d1, 0x1ec, 0x20a, 0x228, 0x24a, 0x26d, 0x291}},
         {{0x15d, 0x171, 0x188, 0x19f, 0x1b7, 0x1d2, 0x1ed, 0x20b, 0x229, 0x24b, 0x26e, 0x292}},
         {{0x15d, 0x172, 0x188, 0x1a0, 0x1b8, 0x1d3, 0x1ee, 0x20c, 0x22a, 0x24c, 0x26f, 0x293}},
         {{0x15e, 0x172, 0x189, 0x1a0, 0x1b9, 0x1d4, 0x1ef, 0x20d, 0x22b, 0x24d, 0x270, 0x295}},
         {{0x15f, 0x173, 0x18a, 0x1a1, 0x1ba, 0x1d4, 0x1f0, 0x20e, 0x22c, 0x24e, 0x271, 0x296}},
         {{0x15f, 0x174, 0x18a, 0x1a2, 0x1bb, 0x1d5, 0x1f1, 0x20f, 0x22d, 0x24f, 0x272, 0x297}},
         {{0x160, 0x174, 0x18b, 0x1a3, 0x1bb, 0x1d6, 0x1f2, 0x210, 0x22e, 0x250, 0x273, 0x298}},
         {{0x161, 0x175, 0x18c, 0x1a3, 0x1bc, 0x1d7, 0x1f3, 0x211, 0x22f, 0x251, 0x274, 0x299}},
         {{0x161, 0x176, 0x18c, 0x1a4, 0x1bd, 0x1d8, 0x1f4, 0x212, 0x230, 0x252, 0x276, 0x29b}},
         {{0x162, 0x176, 0x18d, 0x1a5, 0x1be, 0x1d9, 0x1f5, 0x212, 0x231, 0x254, 0x277, 0x29c}},
         {{0x162, 0x177, 0x18e, 0x1a6, 0x1bf, 0x1d9, 0x1f5, 0x213, 0x232, 0x255, 0x278, 0x29d}},
         {{0x163, 0x178, 0x18f, 0x1a6, 0x1bf, 0x1da, 0x1f6, 0x214, 0x233, 0x256, 0x279, 0x29e}},
         {{0x164, 0x179, 0x18f, 0x1a7, 0x1c0, 0x1db, 0x1f7, 0x215, 0x235, 0x257, 0x27a, 0x29f}},
         {{0x164, 0x179, 0x190, 0x1a8, 0x1c1, 0x1dc, 0x1f8, 0x216, 0x236, 0x258, 0x27b, 0x2a1}},
         {{0x165, 0x17a, 0x191, 0x1a9, 0x1c2, 0x1dd, 0x1f9, 0x217, 0x237, 0x259, 0x27c, 0x2a2}},
         {{0x166, 0x17b, 0x192, 0x1aa, 0x1c3, 0x1de, 0x1fa, 0x218, 0x238, 0x25a, 0x27e, 0x2a3}},
         {{0x166, 0x17b, 0x192, 0x1aa, 0x1c3, 0x1df, 0x1fb, 0x219, 0x239, 0x25b, 0x27f, 0x2a4}},
         {{0x167, 0x17c, 0x193, 0x1ab, 0x1c4, 0x1e0, 0x1fc, 0x21a, 0x23a, 0x25c, 0x280, 0x2a6}},
         {{0x168, 0x17d, 0x194, 0x1ac, 0x1c5, 0x1e0, 0x1fd, 0x21b, 0x23b, 0x25d, 0x281, 0x2a7}},
         {{0x168, 0x17d, 0x194, 0x1ad, 0x1c6, 0x1e1, 0x1fe, 0x21c, 0x23c, 0x25e, 0x282, 0x2a8}},
         {{0x169, 0x17e, 0x195, 0x1ad, 0x1c7, 0x1e2, 0x1ff, 0x21d, 0x23d, 0x260, 0x283, 0x2a9}},
         {{0x16a, 0x17f, 0x196, 0x1ae, 0x1c8, 0x1e3, 0x1ff, 0x21e, 0x23e, 0x261, 0x284, 0x2ab}},
         {{0x16a, 0x17f, 0x197, 0x1af, 0x1c8, 0x1e4, 0x200, 0x21f, 0x23f, 0x262, 0x286, 0x2ac}}
     }};
} // anonymous namespace

std::string MultiChips::s_defaultMelodicBank = "dM";
std::string MultiChips::s_defaultPercussionBank = "HMIGP";

void MultiChips::applyTimbre(Voice* voice, bool rightChan)
{
    int patch = (voice->channel == 9)
                ? voice->key + 128
                : m_channels.at(voice->channel).timbre;

    if( m_stereo && !rightChan )
    {
        if( voice->timbre == patch )
        {
            return;
        }
    }

    const bankdb::Instrument* timbre = instrument(patch);
    if( !timbre )
    {
        throw std::runtime_error(stringFmt("Patch #%d not found", patch));
    }
    if( timbre->second )
    {
        if( !timbre->pseudo4op )
        {
            throw std::runtime_error("4-op");
        }
        if( !voice->secondary )
        {
            voice->secondary = allocVoice();
        }
    }
    voice->timbre = patch;

    opl::SlotView slot = m_chips[voice->chip].getSlotView(voice->slotId + (m_stereo && rightChan ? Voice::StereoOffset : 0));

    slot.setKeyOn(false);
    slot.setFnum(0);
    slot.setBlock(0);
    timbre->first->apply(slot);
    if( timbre->first->data[10] & 1 )
    {
        slot.modulator().setTotalLevel(63);
    }
    slot.carrier().setTotalLevel(63);

    if( m_stereo )
    {
        slot.setOutput(!rightChan, !rightChan, rightChan, rightChan);
    }
    else
    {
        slot.setOutput(true, true, true, true);
    }

    if( voice->secondary )
    {
        BOOST_ASSERT(timbre->second != nullptr);

        slot = m_chips[voice->secondary->chip].getSlotView(voice->secondary->slotId + (m_stereo && rightChan ? Voice::StereoOffset : 0));

        slot.setKeyOn(false);
        slot.setFnum(0);
        slot.setBlock(0);
        timbre->second->apply(slot);
        if( timbre->second->data[10] & 1 )
        {
            slot.modulator().setTotalLevel(63);
        }

        slot.carrier().setTotalLevel(63);

        if( m_stereo )
        {
            slot.setOutput(!rightChan, !rightChan, rightChan, rightChan);
        }
        else
        {
            slot.setOutput(true, true, true, true);
        }
    }
}

#define USE_VOLUME_MAPPING

namespace
{
inline uint8_t calculateTotalLevel(uint8_t insLevel, uint8_t velocity, uint8_t channelVolume, bool adlibMode, uint8_t pan)
{
    BOOST_ASSERT(insLevel < 64);
    unsigned int volume = (insLevel ^ 0x3f) * 2u;
    volume *= std::min<uint16_t>(127u, velocity) + 0x80;
    volume *= channelVolume;
    volume /= 127 * 255;

    if( adlibMode )
    {
        volume /= 2;
        BOOST_ASSERT(volume < 64);
    }
    else
    {
        static constexpr uint8_t volTable[128] = {
            0, 1, 3, 5, 6, 8, 10, 11,
            13, 14, 16, 17, 19, 20, 22, 23,
            25, 26, 27, 29, 30, 32, 33, 34,
            36, 37, 39, 41, 43, 45, 47, 49,
            50, 52, 54, 55, 57, 59, 60, 61,
            63, 64, 66, 67, 68, 69, 71, 72,
            73, 74, 75, 76, 77, 79, 80, 81,
            82, 83, 84, 84, 85, 86, 87, 88,
            89, 90, 91, 92, 92, 93, 94, 95,
            96, 96, 97, 98, 99, 99, 100, 101,
            101, 102, 103, 103, 104, 105, 105, 106,
            107, 107, 108, 109, 109, 110, 110, 111,
            112, 112, 113, 113, 114, 114, 115, 115,
            116, 117, 117, 118, 118, 119, 119, 120,
            120, 121, 121, 122, 122, 123, 123, 123,
            124, 124, 125, 125, 126, 126, 127, 127
        };

        BOOST_ASSERT(volume < 128);
        volume = volTable[volume] / 2u;
    }

    if( pan < 64 )
    {
        volume = (volume * pan) / 64;
    }

    return volume ^ 0x3f;
}
}

void MultiChips::applyVolume(Voice* voice, bool rightChan)
{
    opl::SlotView slot = m_chips[voice->chip].getSlotView(voice->slotId + (m_stereo && rightChan ? Voice::StereoOffset : 0));

    const bankdb::Instrument* timbre = instrument(voice->timbre);
    if( !timbre )
    {
        throw std::runtime_error(stringFmt("Patch #%d not found", voice->timbre));
    }
    if( timbre->second )
    {
        if( !timbre->pseudo4op )
        {
            throw std::runtime_error("4-op");
        }
        if( !voice->secondary )
        {
            voice->secondary = allocVoice();
        }
    }

    const Channel& chan = m_channels.at(voice->channel);
    uint8_t pan = 64;
    if( m_stereo )
    {
        pan = rightChan ? chan.pan : 127 - chan.pan;
    }

    unsigned int level = calculateTotalLevel(timbre->first->data[8] & 0x3f, voice->velocity, chan.volume, m_adlibVolumes, pan);
    slot.carrier().setTotalLevel(level);

    if( timbre->first->data[10] & 0x01 )
    {
        level = calculateTotalLevel(timbre->first->data[9] & 0x3f, voice->velocity, chan.volume, m_adlibVolumes, pan);
        slot.modulator().setTotalLevel(level);
    }

    if( voice->secondary )
    {
        BOOST_ASSERT(timbre->second != nullptr);

        slot = m_chips[voice->secondary->chip].getSlotView(voice->secondary->slotId + (m_stereo && rightChan ? Voice::StereoOffset : 0));

        level = calculateTotalLevel(timbre->second->data[8] & 0x3f, voice->velocity, chan.volume, m_adlibVolumes, pan);
        slot.carrier().setTotalLevel(level);

        if( timbre->second->data[10] & 0x01 )
        {
            level = calculateTotalLevel(timbre->second->data[9] & 0x3f, voice->velocity, chan.volume, m_adlibVolumes, pan);
            slot.modulator().setTotalLevel(level);
        }
    }
}

MultiChips::Voice* MultiChips::allocVoice()
{
    if( !m_voicePool.empty() )
    {
        auto result = *m_voicePool.begin();
        if( m_voicePool.erase(result) == 0 )
        {
            throw std::runtime_error("Oooops");
        }
        BOOST_ASSERT(result->secondary == nullptr);
        return result;
    }

    return nullptr;
}

void MultiChips::freeVoice(MultiChips::Channel& channel, MultiChips::Voice* voice)
{
    if( voice == nullptr )
    {
        return;
    }

    auto it = channel.voices.find(voice);
    if( it != channel.voices.end() )
    {
        channel.voices.erase(it);
    }
    if( !m_voicePool.insert(voice).second )
    {
        throw std::runtime_error("Could not free voice");
    }
}

void MultiChips::applyPitch(Voice* voice, bool rightChan)
{
    int note;
    uint32_t patch;
    int detune1 = 0, detune2 = 0;
    if( voice->channel == 9 )
    {
        patch = voice->key + 128;
        const bankdb::Instrument* timbre = instrument(patch);
        if( !timbre )
        {
            throw std::runtime_error(stringFmt("Patch #%d not found", patch));
        }
        if( timbre->second )
        {
            if( !timbre->pseudo4op )
            {
                throw std::runtime_error("4-op");
            }
            if( !voice->secondary )
            {
                voice->secondary = allocVoice();
            }
        }
        if( !timbre->noteOverride )
        {
            throw std::runtime_error("No note override");
        }
        note = *timbre->noteOverride;
    }
    else
    {
        patch = m_channels.at(voice->channel).timbre;
        const bankdb::Instrument* timbre = instrument(patch);
        if( !timbre )
        {
            throw std::runtime_error(stringFmt("Patch #%d not found", patch));
        }
        if( timbre->second )
        {
            if( !timbre->pseudo4op )
            {
                throw std::runtime_error("4-op");
            }
            if( !voice->secondary )
            {
                voice->secondary = allocVoice();
            }
        }
        note = voice->key;
        detune1 = timbre->first->finetune;
        if( timbre->second )
        {
            detune2 = timbre->second->finetune;
        }
    }

    const auto detune = m_channels.at(voice->channel).keyDetune;

    const auto origNote = note;
    note += m_channels.at(voice->channel).keyOffset - 12 + detune1;
    if( note >= 8 * 12 )
    {
        note = 8 * 12 - 1;
    }
    else if( note < 0 )
    {
        note = 0;
    }

    opl::SlotView slot = m_chips[voice->chip].getSlotView(voice->slotId + (m_stereo && rightChan ? Voice::StereoOffset : 0));
    slot.setFnum(NotePitch[detune][note % 12]);
    slot.setBlock(note / 12u);
    slot.setKeyOn(voice->isNoteOn);

    if( voice->secondary )
    {
        slot = m_chips[voice->secondary->chip].getSlotView(voice->secondary->slotId + (m_stereo && rightChan ? Voice::StereoOffset : 0));

        note = origNote;
        note += m_channels.at(voice->channel).keyOffset - 12 + detune2;

        if( note >= 8 * 12 )
        {
            note = 8 * 12 - 1;
        }
        else if( note < 0 )
        {
            note = 0;
        }

        slot.setFnum(NotePitch[detune][note % 12]);
        slot.setBlock(note / 12u);
        slot.setKeyOn(voice->isNoteOn);
    }
}

void MultiChips::setChannelVolume(uint8_t channel, uint8_t volume)
{
    volume = std::max<uint8_t>(0, volume);
    volume = std::min<uint8_t>(volume, 127);
    m_channels.at(channel).volume = volume;

    for( auto voice : m_channels.at(channel).voices )
    {
        applyVolume(voice, false);
        if( m_stereo )
        {
            applyVolume(voice, true);
        }
    }
}

void MultiChips::noteOff(uint8_t channel, uint8_t key)
{
    Voice* voice = m_channels.at(channel).getVoice(key);

    if( !voice )
    {
        return;
    }

    voice->isNoteOn = false;

    opl::SlotView slot = m_chips[voice->chip].getSlotView(voice->slotId);
    slot.setKeyOn(false);

    if( m_stereo )
    {
        slot = m_chips[voice->chip].getSlotView(voice->slotId + Voice::StereoOffset);
        slot.setKeyOn(false);
    }

    if( voice->secondary )
    {
        BOOST_ASSERT(voice->secondary->secondary == nullptr);

        slot = m_chips[voice->secondary->chip].getSlotView(voice->secondary->slotId);
        slot.setKeyOn(false);

        if( m_stereo )
        {
            slot = m_chips[voice->secondary->chip].getSlotView(voice->secondary->slotId + Voice::StereoOffset);
            slot.setKeyOn(false);
        }

        freeVoice(m_channels.at(channel), voice->secondary);
    }
    voice->secondary = nullptr;

    freeVoice(m_channels.at(channel), voice);
}

void MultiChips::noteOn(uint8_t channel, uint8_t key, int velocity)
{
    if( velocity == 0 )
    {
        noteOff(channel, key);
        return;
    }

    if( m_channels.at(channel).timbre < 0 )
    {
        return;
    }

    Voice* voice = allocVoice();

    if( !voice )
    {
        if( !m_channels.at(9).voices.empty() )
        {
            noteOff(9, (*m_channels.at(9).voices.begin())->key);
            voice = allocVoice();
        }
        if( !voice )
        {
            return;
        }
    }

    voice->key = key;
    voice->channel = channel;
    voice->velocity = velocity;
    voice->isNoteOn = true;

    m_channels.at(channel).voices.insert(voice);

    applyTimbre(voice, false);
    applyVolume(voice, false);
    applyPitch(voice, false);
    if( m_stereo )
    {
        applyTimbre(voice, true);
        applyVolume(voice, true);
        applyPitch(voice, true);
    }
}

void MultiChips::allNotesOff(uint8_t channel)
{
    while( !m_channels.at(channel).voices.empty() )
    {
        noteOff(channel, (*m_channels.at(channel).voices.begin())->key);
    }
}

void MultiChips::controlChange(uint8_t channel, ControlData type, uint8_t data)
{
    switch( type )
    {
        case ControlData::ResetAllControllers:
            resetVoices();
            setChannelVolume(channel, Channel::DefaultChannelVolume);
            setChannelDetune(channel, 0);
            m_channels.at(channel).pan = 64;
            break;

        case ControlData::RpnMsb:
            m_channels.at(channel).rpn &= 0x00FF;
            m_channels.at(channel).rpn |= uint16_t(data) << 8;
            break;

        case ControlData::RpnLsb:
            m_channels.at(channel).rpn &= 0xFF00;
            m_channels.at(channel).rpn |= data;
            break;

        case ControlData::DataentryMsb:
            if( m_channels.at(channel).rpn == 0 )
            {
                m_channels.at(channel).pitchBendSemiTones = data;
                m_channels.at(channel).pitchBendRange =
                    m_channels.at(channel).pitchBendSemiTones * 100 +
                    m_channels.at(channel).pitchBendHundreds;
            }
            break;

        case ControlData::DataentryLsb:
            if( m_channels.at(channel).rpn == 0 )
            {
                m_channels.at(channel).pitchBendHundreds = data;
                m_channels.at(channel).pitchBendRange =
                    m_channels.at(channel).pitchBendSemiTones * 100 +
                    m_channels.at(channel).pitchBendHundreds;
            }
            break;

        case ControlData::Pan:
            if( channel != 9 )
            {
                m_channels.at(channel).pan = data & 0x7f;
            }
            break;
    }
}

void MultiChips::programChange(uint8_t channel, uint8_t patch)
{
    m_channels.at(channel).timbre = patch;
}

void MultiChips::setChannelDetune(uint8_t channel, int detune)
{
}

void MultiChips::resetVoices()
{
    m_voicePool.clear();

    const auto voicesPerChip = m_stereo ? 9 : 18;
    for( size_t index = 0; index < m_voices.size(); ++index )
    {
        m_voices[index].chip = index / voicesPerChip;
        m_voices[index].slotId = index % voicesPerChip;
        m_voices[index].secondary = nullptr;
        m_voicePool.insert(&m_voices[index]);
    }

    for( Channel& chan : m_channels )
    {
        chan.voices.clear();
    }
}

void MultiChips::flushCard()
{
    for( opl::Opl3& chip : m_chips )
    {
        for( int i = 0; i < 36; ++i )
        {
            opl::OperatorView op = chip.getOperatorView(i % 18, i >= 18);
            op.setKsl(3);
            op.setTotalLevel(63);
            op.setAttackRate(0);
            op.setDecayRate(15);
            op.setSustainLevel(15);
            op.setReleaseRate(15);
            op.setWave(0);
        }
        for( int i = 0; i < 18; ++i )
        {
            opl::SlotView slot = chip.getSlotView(i);
            slot.setFnum(0);
            slot.setBlock(0);
            slot.setKeyOn(false);
        }
    }
}

void MultiChips::reset()
{
    for( opl::Opl3& chip : m_chips )
    {
        chip.writeReg(1, 0x20); // LSI test
        chip.writeReg(8, 0); // NTS

        // Set the values: AM Depth, VIB depth & Rhythm
        chip.writeReg(0xbd, 0); // DAM, DVB, RYT, Percussion
        chip.writeReg(0x105, 1); // NEW=1
    }

    flushCard();
}

void MultiChips::pitchBend(uint8_t channel, uint8_t lsb, uint8_t msb)
{
    auto pitchbend = lsb | (uint16_t(msb) << 8);

    static constexpr auto PITCHBEND_CENTER = 1638400;
    static constexpr auto FINETUNE_RANGE = 32;

    auto totalBend = pitchbend * m_channels[channel].pitchBendRange;
    totalBend /= (PITCHBEND_CENTER / FINETUNE_RANGE);

    m_channels.at(channel).keyOffset = totalBend / FINETUNE_RANGE;
    m_channels.at(channel).keyOffset -= m_channels[channel].pitchBendSemiTones;

    m_channels.at(channel).keyDetune = totalBend % FINETUNE_RANGE;

    for( auto voice : m_channels.at(channel).voices )
    {
        applyPitch(voice, false);
        if( m_stereo )
        {
            applyPitch(voice, true);
        }
    }
}
}