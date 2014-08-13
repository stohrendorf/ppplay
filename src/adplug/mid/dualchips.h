#ifndef PPPLAY_DUALCHIPS_H
#define PPPLAY_DUALCHIPS_H

#include "ymf262/opl3.h"

#include <set>

namespace ppp
{
class DualChips
{
public:
    enum Array {
        First,
        Second,
        NumArrays
    };

    struct Timbre
    {
        uint8_t SAVEK[ 2 ];
        uint8_t Level[ 2 ];
        uint8_t Env1[ 2 ];
        uint8_t Env2[ 2 ];
        uint8_t Wave[ 2 ];
        uint8_t Feedback;
        int8_t Transpose;
        //int8_t Velocity;
    };

    struct Voice
    {
        uint num = 0;
        uint key = 0;
        uint velocity = 0;
        uint channel = -1;
        uint pitch = 0;
        int timbre = -1;
        Array arraySelector = Array::First;
        bool isNoteOn = false;
    };

    typedef std::vector<Voice*> VoiceList;

    struct OplChannel
    {
        static constexpr auto DefaultChannelVolume = 90;
        static constexpr auto DefaultPitchBendRange = 200;

        std::set<Voice*> Voices{};
        int Timbre = 0;
        int Pitchbend = 0;
        int KeyOffset = 0;
        uint KeyDetune = 0;
        uint Volume = DefaultChannelVolume;
        uint EffectiveVolume = 0;
        int Detune = 0;
        uint RPN;
        short PitchBendRange = DefaultPitchBendRange;
        short PitchBendSemiTones = DefaultPitchBendRange/100;
        short PitchBendHundreds = DefaultPitchBendRange%100;

        Voice* getVoice(uint8_t key) const
        {
            for(auto voice : Voices)
                if(voice->key == key)
                    return voice;

            return nullptr;
        }
    };

    enum class ControlData {
        ResetAllControllers, RpnMsb, RpnLsb, DataentryMsb, DataentryLsb
    };

    DualChips() : m_voices(), m_channels(), m_timbreBank(), m_voiceLevel(), m_voiceKsl()
    {
        extern std::array<DualChips::Timbre,256> ADLIB_TimbreBank;
        m_timbreBank = ADLIB_TimbreBank;

        for(auto& x : m_voiceLevel)
            x.fill(0);
        for(auto& x : m_voiceKsl)
            x.fill(0);
        reset();
        resetVoices();
    }

    void programChange(uint8_t channel, uint8_t patch);
    void noteOff(uint8_t channel, uint8_t key);
    void noteOn(uint8_t channel, uint8_t key, int velocity);
    void setChannelVolume(uint8_t channel, uint8_t volume);
    void allNotesOff(uint8_t channel);
    void controlChange(uint8_t channel, ControlData type, uint8_t data = 0);
    void setChannelDetune(uint8_t channel, int detune);
    void pitchBend(uint8_t channel, uint8_t lsb, uint8_t msb);

    void read(std::array<int16_t,4>* data) {
        m_chip.read(data);
    }


private:
    opl::Opl3 m_chip{};
    std::array<Voice, 18> m_voices;
    std::array<OplChannel, 16> m_channels;
    std::array<Timbre,256> m_timbreBank;
    std::array<std::array<int,Array::NumArrays>,18> m_voiceLevel;
    std::array<std::array<int,Array::NumArrays>,18> m_voiceKsl;
    std::set<Voice*> m_voicePool{};

    void send(Array which, uint16_t reg, uint8_t val) {
        switch (which) {
        case First:
            m_chip.writeReg(reg, val);
            break;
        case Second:
            m_chip.writeReg(reg + 0x100, val);
            break;
        default:
            break;
        }
    }

    void applyTimbre(Voice *voice);
    void applyVolume(Voice *voice);
    void applyPitch(Voice *voice);
    Voice* allocVoice();
    Voice *getVoice(OplChannel *channel, uint8_t key) const;
    void resetVoices();
    void flushCard(Array port);
    void reset();
};
}

#endif // DUALCHIPS_H
