#ifndef PPPLAY_DUALCHIPS_H
#define PPPLAY_DUALCHIPS_H

#include "ymf262/opl3.h"

#include "stuff/utils.h"

#ifdef USE_BANKDB
#include "../bankgen/bankdatabase.h"
#endif

#include "stuff/system.h"

#include <set>

namespace ppp
{
class MultiChips
{
    DISABLE_COPY(MultiChips)
public:
#ifndef USE_BANKDB
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
        int8_t transpose;
    };
#endif

    struct Voice
    {
        static constexpr auto StereoOffset = 9;
        //! @brief This voice's chip index.
        size_t chip = 0;
        //! @brief This voice's index; used to calculate the slot.
        uint slotId = 0;
        //! @brief The active MIDI key.
        uint key = 0;
        //! @brief Key velocity (0..127).
        uint8_t velocity = 0;
        //! @brief The channel this voice belongs to.
        int channel = -1;
        //! @brief The active MIDI patch/instrument.
        int timbre = -1;
        bool isNoteOn = false;
        uint8_t pan = 64;
#ifdef USE_BANKDB
        Voice* secondary = nullptr;
#endif
    };

    using VoiceList = std::vector<Voice*>;

    struct Channel
    {
        static constexpr auto DefaultChannelVolume = 90;
        static constexpr auto DefaultPitchBendRange = 200;

        std::set<Voice*> voices{};
        int timbre = -1;
        int pitchbend = 0;
        int keyOffset = 0;
        uint keyDetune = 0;
        uint volume = DefaultChannelVolume;
        int detune = 0;
        uint rpn = 0;
        uint8_t pan = 64;
        short pitchBendRange = DefaultPitchBendRange;
        short pitchBendSemiTones = DefaultPitchBendRange/100;
        short pitchBendHundreds = DefaultPitchBendRange%100;

        Voice* getVoice(uint8_t key) const
        {
            for(auto voice : voices) {
                if(voice->key == key)
                    return voice;
            }

            return nullptr;
        }
    };

    enum class ControlData {
        ResetAllControllers, RpnMsb, RpnLsb, DataentryMsb, DataentryLsb, Pan
    };

    MultiChips(size_t chipCount, bool stereo)
        : m_chips(chipCount)
        , m_voices(chipCount * (stereo ? 9 : 18))
        , m_channels()
#ifndef USE_BANKDB
        , m_timbreBank()
#endif
        , m_stereo(stereo)
    {
        if(chipCount == 0)
            throw std::runtime_error("Must at least use one chip");

#ifndef USE_BANKDB
        extern std::array<MultiChips::Timbre,256> ADLIB_TimbreBank;
        m_timbreBank = ADLIB_TimbreBank;
#else
        m_bankDb.load(ppp::whereAmI() + "/../share/ppplay/bankdb.xml");
        m_melodicBank = m_bankDb.bank("dM");
        if(!m_melodicBank)
            throw std::runtime_error("Bank not found");
        if(m_melodicBank->uses4op)
            throw std::runtime_error("Bank uses pure 4-op slots");
        if(m_melodicBank->onlyPercussion)
            throw std::runtime_error("Bank only provides rhythm instruments");
        m_percussionBank = m_bankDb.bank("HMIGP");
        if(!m_percussionBank)
            m_percussionBank = m_melodicBank;
#endif

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
        if(data) {
            data->fill(0);
            for(opl::Opl3& chip : m_chips) {
                std::array<int16_t,4> tmp;
                chip.read(&tmp);
                for(int i=0; i<4; ++i)
                    (*data)[i] += tmp[i];
            }
        }
        else {
            for(opl::Opl3& chip : m_chips)
                chip.read(nullptr);
        }
    }

    void useAdlibVolumes(bool value) noexcept {
        m_adlibVolumes = value;
    }

private:
    std::vector<opl::Opl3> m_chips;
    std::vector<Voice> m_voices;
    std::array<Channel, 16> m_channels;
#ifndef USE_BANKDB
    std::array<MultiChips::Timbre,256> m_timbreBank;
#else
    bankdb::BankDatabase m_bankDb{};
    const bankdb::Bank* m_melodicBank = nullptr;
    const bankdb::Bank* m_percussionBank = nullptr;
    static const bankdb::Instrument* instrument(size_t idx, const bankdb::Bank* bank)
    {
        if(bank == nullptr)
            return nullptr;

        const auto& it = bank->instruments.find(idx);
        if(it == bank->instruments.end())
            return nullptr;
        else
            return &it->second;
    }

    const bankdb::Instrument* instrument(size_t idx) const
    {
        if(idx < 128)
            return instrument(idx, m_melodicBank);
        else
            return instrument(idx, m_percussionBank);
    }
#endif
    std::set<Voice*> m_voicePool{};
    bool m_adlibVolumes = true;
    bool m_stereo;

    void applyTimbre(Voice *voice, bool rightChan);
    void applyVolume(Voice *voice, bool rightChan);
    void applyPitch(Voice *voice, bool rightChan);
    Voice* allocVoice();
    void freeVoice(Channel& channel, Voice* voice);
    void resetVoices();
    void flushCard();
    void reset();
};
}

#endif // DUALCHIPS_H
