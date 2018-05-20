#pragma once

#include "genmod/stepper.h"
#include "genmod/sample.h"
#include "filters.h"

namespace ppp
{
namespace it
{
struct ItSampleHeader;
struct ItInstrument;
struct ItSample;

class ItModule;

constexpr uint16_t SCFLG_ON = 0x0001;
constexpr uint16_t SCFLG_RECALC_PAN = 0x0002;
constexpr uint16_t SCFLG_NOTE_OFF = 0x0004;
constexpr uint16_t SCFLG_FADEOUT = 0x0008;
constexpr uint16_t SCFLG_RECALC_VOL = 0x0010;
constexpr uint16_t SCFLG_FREQ_CHANGE = 0x0020;
constexpr uint16_t SCFLG_RECALC_FINAL_VOL = 0x0040;
constexpr uint16_t SCFLG_CTR_PAN = 0x0080;
constexpr uint16_t SCFLG_NEW_NOTE = 0x0100;
constexpr uint16_t SCFLG_NOTE_CUT = 0x0200;
constexpr uint16_t SCFLG_LOOP_CHANGE = 0x0400;
constexpr uint16_t SCFLG_MUTED = 0x0800;
constexpr uint16_t SCFLG_VOL_ENV = 0x1000;
constexpr uint16_t SCFLG_PAN_ENV = 0x2000;
constexpr uint16_t SCFLG_PITCH_ENV = 0x4000;
constexpr uint16_t SCFLG_PAN_CHANGE = 0x8000;

struct HostChannel;

struct SEnvelope final
{
    Stepper value{1, 1};
    uint16_t tick = 0; //!< pos
    uint16_t nextPointIndex = 0; //!< curEnN, current envelope node
    uint16_t nextPointTick = 0; //!< nextET
    uint16_t filter = 0;
};

constexpr uint8_t SurroundPan = 100;

struct SlaveChannel
{
    uint16_t flags = SCFLG_NOTE_CUT;

    ppp::Sample::LoopType lpm = ppp::Sample::LoopType::None;
    bool loopDirBackward = false; //!< lpd
    uint32_t frequency = 0;
    uint32_t frequencySet = 0;
    // uint8_t bit; //!< 2 = 16, 0 = 8.
    uint8_t vip = 0; //!< Vibrato Speed
    uint16_t viDepth = 0; //!< Vibrato Depth
    uint32_t midiFset = 0;
    // uint8_t fv; -> _16bVol
    uint8_t effectiveVolume = 0; //!< vol, 0..64
    uint8_t effectiveBaseVolume = 0; //!< vs, 0..64
    uint8_t channelVolume = 0; //!< cvl, 0..64
    uint8_t sampleVolume = 0; //!< svl, 0..128
    int8_t midiFinalPan = 0; //!< fp, pan + pan-envelope, 0..64
    int16_t fadeOut = 0; //!< 0..1024
    uint8_t dca = 0;
    uint8_t pan = 32; //!< 0..64, 100 = Surround, >= 128 = muted.
    uint8_t ps = 0; //!< 0..64
    const ItInstrument* insOffs = nullptr;
    uint8_t effectiveNote = 0; //!< nte
    uint8_t ins = 0; //!< Instrument number (1 based, 0 if none)
    const ItSample* smpOffs = nullptr;
    uint8_t smp = 0; //!< Sample number (0 based)
    uint8_t finalPan = 0; //!< fpp, 0..64

    int16_t mixVolumeL = 0;
    int16_t mixVolumeR = 0;

private:
    HostChannel* m_hcOffst = nullptr;

public:
    void setHost(HostChannel* host)
    {
        BOOST_ASSERT(host != nullptr);
        m_hcOffst = host;
    }

    HostChannel* getHost()
    {
        return m_hcOffst;
    }

    const HostChannel* getHost() const
    {
        BOOST_ASSERT(m_hcOffst != nullptr);
        return m_hcOffst;
    }

    bool disowned = false;
    uint8_t nna = 0;
    uint8_t mch = 0;
    int8_t midiProgram = 0; //!< mpr
    uint16_t midiBank = 0; //!< mbank
    uint8_t filterCutoff = 0x7f;
    uint16_t envFilterCutoff = 256; //!< 0..256
    uint8_t filterResonance = 0;
    uint32_t loopBeg = 0;
    uint32_t loopEnd = 0;
    uint16_t _16bVol = 0; //!< fv, 0..32768
    Stepper sampleOffset{1, 1};
    SEnvelope vEnvelope{};
    SEnvelope pEnvelope{};
    SEnvelope ptEnvelope{};

    void applySampleLoop();

    void setInstrument(const ItModule& module, const ItInstrument& ins, SlaveChannel* lastSlaveChannel);

    Filter filterL{};
    Filter filterR{};
};
}
}
