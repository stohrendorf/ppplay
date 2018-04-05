#pragma once

#include "genmod/channelstate.h"

#include <cstdint>
#include <boost/assert.hpp>

namespace ppp
{
namespace it
{
constexpr uint8_t HCFLG_MSK_NOTE_1 = 0x01;
constexpr uint8_t HCFLG_MSK_NOTE_2 = 0x10;
constexpr uint8_t HCFLG_MSK_NOTE = HCFLG_MSK_NOTE_1 | HCFLG_MSK_NOTE_2;
constexpr uint8_t HCFLG_MSK_INS_1 = 0x02;
constexpr uint8_t HCFLG_MSK_INS_2 = 0x20;
constexpr uint8_t HCFLG_MSK_INS = HCFLG_MSK_INS_1 | HCFLG_MSK_INS_2;
constexpr uint8_t HCFLG_MSK_VOL_1 = 0x04;
constexpr uint8_t HCFLG_MSK_VOL_2 = 0x40;
constexpr uint8_t HCFLG_MSK_VOL = HCFLG_MSK_VOL_1 | HCFLG_MSK_VOL_2;
constexpr uint8_t HCFLG_MSK_CMD_1 = 0x08;
constexpr uint8_t HCFLG_MSK_CMD_2 = 0x80;
constexpr uint8_t HCFLG_MSK_CMD = HCFLG_MSK_CMD_1 | HCFLG_MSK_CMD_2;

constexpr uint16_t HCFLG_UPD_MODE = 0x0003;
constexpr uint16_t HCFLG_UPD_NEVER = 0x0000;
constexpr uint16_t HCFLG_UPD_IF_ON = 0x0001;
constexpr uint16_t HCFLG_UPD_ALWAYS = 0x0002;
constexpr uint16_t HCFLG_ON = 0x0004;
constexpr uint16_t HCFLG_CUT = 0x0008;
constexpr uint16_t HCFLG_SLIDE = 0x0010;
constexpr uint16_t HCFLG_ROW_UPDATED = 0x0040;
constexpr uint16_t HCFLG_RANDOM = 0x0080;
constexpr uint16_t HCFLG_UPD_VOL_IF_ON = 0x0100;
constexpr uint16_t HCFLG_UPD_VOL_ALWAYS = 0x0200;
constexpr uint16_t HCFLG_NO_IRQ = 0x8000;

struct SlaveChannel;

struct HostChannel final
    : public ISerializable
{
    uint16_t flags = 0;
    uint8_t cellMask = 0; //!< msk
    uint8_t patternNote = 0; //!< nte
    uint8_t patternInstrument = 0xff; //!< ins
    uint8_t patternVolume = 0; //!< vol
    uint8_t patternFx = 0; //!< cmdVal1
    uint8_t patternFxParam = 0; //!< cmdVal2
    uint8_t lastPatternFx = 0; //!< ocmVal1
    uint8_t lastPatternFxParam = 0; //!< ocmVal2
    uint8_t volumeFx = 0; //!< vcmVal1
    uint8_t volumeFxParam = 0; //!< vcmVal2
    uint8_t mch = 0;
    uint8_t midiProgram = 0; //!< mpr
    uint8_t effectiveNote = 0; //!< nt2, effective sample note after translation
    uint8_t sampleIndex = 0; //!< smp, sample index (1-based)
    uint8_t dkl = 0;
    uint8_t efg = 0;
    uint8_t o00 = 0;
    uint8_t i00 = 0;
    uint8_t j00 = 0;
    uint8_t m00 = 0;
    uint8_t n00 = 0;
    uint8_t p00 = 0;
    uint8_t q00 = 0;
    uint8_t t00 = 0;
    uint8_t s00 = 0;
    uint8_t oxh = 0;
    uint8_t w00 = 0;
    uint8_t vce = 0;
    uint8_t goe = 0;
    uint8_t sfx = 0;

    //ofs:0x20
    uint8_t cuc = 0;
    uint8_t vse = 0; //!< 0..64
    int8_t ltr = 0; //!< Last tremolo

private:
    SlaveChannel* m_scOffst = nullptr;

public:
    void setSlave(SlaveChannel* slave)
    {
        BOOST_ASSERT(slave != nullptr);
        m_scOffst = slave;
    }

    SlaveChannel* getSlave()
    {
        return m_scOffst;
    }

    const SlaveChannel* getSlave() const
    {
        return m_scOffst;
    }

    uint8_t plr = 0; //!< pattern loop row (1-based)
    uint8_t plc = 0; //!< pattern loop count
    uint8_t panbrelloWaveform = 0;
    uint8_t panbrelloOffset = 0;
    uint8_t panbrelloDepth = 0;
    uint8_t panbrelloSpeed = 0;
    uint8_t lastRandomPanbrelloValue = 0;
    int8_t lvi = 0;

    //ofs:0x3e
    uint8_t cp = 32; // init from header, 0..64
    uint8_t channelVolume = 64; //!< 0..64
    int8_t vch = 0; //!< volume change?
    int8_t tremorCountdown = 0;
    bool tremorOn = false;
    int8_t retriggerCountdown = 0; //!< rtc
    uint32_t portaTargetFrequency = 0;
    uint8_t vwf = 0;
    uint8_t vibratoPosition = 0;
    uint8_t vdp = 0;
    uint8_t vsp = 0;
    uint8_t tremoloWaveForm = 0;
    uint8_t tremoloPosition = 0;
    uint8_t tremoloDepth = 0;
    uint8_t tremoloSpeed = 0;

    bool portaSlideUp = false;
    uint16_t slideSpeed = 0;

    uint8_t tremorOnTime = 0;
    uint8_t tremorOffTime = 0;
    uint8_t arpeggioStage = 0;
    float arpeggioStage1 = 0;
    float arpeggioStage2 = 0;
    int8_t channelVolumeChange = 0;
    int8_t panbrelloChange = 0;
    int8_t globalVolumeChange = 0;

    int8_t sfxData = 0;
    uint8_t sfxType = 0;

    void enable()
    {
        flags |= HCFLG_ON;
    }

    void disable()
    {
        flags &= ~HCFLG_ON;
    }

    bool isEnabled() const noexcept
    {
        return (flags & HCFLG_ON) != 0;
    }

    ChannelState channelState;

    AbstractArchive& serialize(AbstractArchive* archive) override
    {
        return *archive
               % flags
               % cellMask
               % patternNote
               % patternInstrument
               % patternVolume
               % patternFx
               % patternFxParam
               % lastPatternFx
               % lastPatternFxParam
               % volumeFx
               % volumeFxParam
               % mch
               % midiProgram
               % effectiveNote
               % sampleIndex
               % dkl
               % efg
               % o00
               % i00
               % j00
               % m00
               % n00
               % p00
               % q00
               % t00
               % s00
               % oxh
               % w00
               % vce
               % goe
               % sfx
               % cuc
               % vse
               % ltr
               % *reinterpret_cast<uintptr_t*>(&m_scOffst)
               % plr
               % plc
               % panbrelloWaveform
               % panbrelloOffset
               % panbrelloDepth
               % panbrelloSpeed
               % lastRandomPanbrelloValue
               % lvi
               % cp
               % channelVolume
               % vch
               % tremorCountdown
               % tremorOn
               % retriggerCountdown
               % portaTargetFrequency
               % vwf
               % vibratoPosition
               % vdp
               % vsp
               % tremoloWaveForm
               % tremoloPosition
               % tremoloDepth
               % tremoloSpeed
               % portaSlideUp
               % slideSpeed
               % tremorOnTime
               % tremorOffTime
               % arpeggioStage
               % arpeggioStage1
               % arpeggioStage2
               % channelVolumeChange
               % panbrelloChange
               % globalVolumeChange
               % sfxData
               % sfxType
               % channelState;
    }
};
}
}