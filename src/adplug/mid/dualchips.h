#ifndef PPPLAY_DUALCHIPS_H
#define PPPLAY_DUALCHIPS_H

#include "ymf262/opl3.h"

#include <set>

namespace ppp
{
class DualChips
{
public:
    struct Timbre
    {
        struct OperatorData {
            uint8_t modulator;
            uint8_t carrier;
        };
        using SlotData = uint8_t;

        OperatorData amVibEgtKsrMult;
        OperatorData kslLevel;
        OperatorData attackDecay;
        OperatorData sustainRelease;
        OperatorData wave;
        SlotData feedback;
        int8_t Transpose;
    };

    struct Voice
    {
        //! @brief This voice's index; used to calculate the slot.
        uint num = 0;
        //! @brief The active MIDI key.
        uint key = 0;
        //! @brief Key velocity (0..127).
        uint8_t velocity = 0;
        //! @brief The channel this voice belongs to.
        int channel = -1;
        //! @brief The current frequency.
        uint32_t pitch = 0;
        //! @brief The active MIDI patch/instrument.
        int timbre = -1;
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
        int Detune = 0;
        uint RPN = 0;
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

    DualChips() : m_voices(), m_channels(), m_timbreBank()
    {
        extern std::array<DualChips::Timbre,256> ADLIB_TimbreBank;
        m_timbreBank = ADLIB_TimbreBank;

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

    void useAdlibVolumes(bool value) noexcept {
        m_adlibVolumes = value;
    }

private:
    opl::Opl3 m_chip{};
    std::array<Voice, 18> m_voices;
    std::array<OplChannel, 16> m_channels;
    std::array<Timbre,256> m_timbreBank;
    std::set<Voice*> m_voicePool{};
    bool m_adlibVolumes = true;

    void applyTimbre(Voice *voice);
    void applyVolume(Voice *voice);
    void applyPitch(Voice *voice);
    Voice* allocVoice();
    void resetVoices();
    void flushCard();
    void reset();
};
}

#endif // DUALCHIPS_H
