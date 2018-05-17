#include "slavechannel.h"

#include "sample.h"
#include "genmod/sample.h"
#include "instrument.h"
#include "sample.h"
#include "itmodule.h"
#include "hostchannel.h"

namespace ppp
{
namespace it
{
void SlaveChannel::applySampleLoop()
{
    uint32_t loopBeg = 0;
    uint32_t loopEnd = 0;
    auto lpm = Sample::LoopType::None;
    if( (smpOffs->header.flg & (ItSampleHeader::FlgLooped | ItSampleHeader::FlgSusLooped)) == 0 )
    {
        // not looped at all
        loopBeg = 0;
        loopEnd = smpOffs->header.length;
    }
    else
    {
        loopBeg = smpOffs->header.loopBeg;
        loopEnd = smpOffs->header.loopEnd;
        if( (smpOffs->header.flg & ItSampleHeader::FlgSusLooped) == 0 )
        {
            // no susloop
            if( (smpOffs->header.flg & ItSampleHeader::FlgLoopPingPong) != 0 )
            {
                lpm = Sample::LoopType::Pingpong; // ping pong
            }
            else
            {
                lpm = Sample::LoopType::Forward; // forward
            }
        }
        else
        {
            // susloop
            if( (flags & SCFLG_NOTE_OFF) == 0 )
            {
                loopBeg = smpOffs->header.susLoopBeg;
                loopEnd = smpOffs->header.susLoopEnd;
                if( (smpOffs->header.flg & ItSampleHeader::FlgSusLoopPingPong) != 0 )
                {
                    lpm = Sample::LoopType::Pingpong; // ping pong
                }
                else
                {
                    lpm = Sample::LoopType::Forward; // forward
                }
            }
            else if( (smpOffs->header.flg & ItSampleHeader::FlgLooped) == 0 )
            {
                // not looped
                loopBeg = 0;
                loopEnd = smpOffs->header.length;
            }
        }
    }

    if( this->lpm != lpm || this->loopBeg != loopBeg || this->loopEnd != loopEnd )
    {
        this->lpm = lpm;
        this->loopBeg = loopBeg;
        this->loopEnd = loopEnd;
        this->flags |= SCFLG_LOOP_CHANGE;
    }
}

void SlaveChannel::setInstrument(const ItModule& module, const ItInstrument& ins, SlaveChannel* lastSlaveChannel)
{
    this->insOffs = &ins;
    this->nna = ins.nna;
    this->dca = ins.dca;
    if( this->getHost()->midiChannel != 0 )
    {
        this->mch = this->getHost()->midiChannel;
        this->midiProgram = this->getHost()->midiProgram;
        this->midiBank = ins.midibnk;
    }

    this->channelVolume = this->getHost()->channelVolume;

    uint8_t newPan;
    if( (ins.dfp & 0x80) == 0 )
    {
        newPan = ins.dfp;
    }
    else
    {
        newPan = this->getHost()->cp;
    }

    if( this->getHost()->sampleIndex != 0 )
    {
        if( (module.m_samples[this->getHost()->sampleIndex - 1]->header.dfp & 0x80) != 0 )
        {
            newPan = module.m_samples[this->getHost()->sampleIndex - 1]->header.dfp & 0x7f;
        }
    }

    if( newPan != 100 )
    {
        auto tmp = (this->getHost()->patternNote - ins.pitchPanCenter) * ins.pitchPanSeparation / 8 + newPan;

        if( tmp < 0 )
        {
            newPan = 0;
        }
        else if( tmp > 64 )
        {
            newPan = 64;
        }
        else
        {
            newPan = tmp;
        }
    }

    BOOST_ASSERT(newPan <= 64 || newPan == 100);
    this->pan = newPan;
    this->ps = newPan;
    this->vEnvelope.value = 64;
    this->vEnvelope.tick = 0;
    this->vEnvelope.nextPointIndex = 0;
    this->vEnvelope.nextPointTick = 0;
    this->pEnvelope.value = 0;
    this->pEnvelope.tick = 0;
    this->pEnvelope.nextPointIndex = 0;
    this->pEnvelope.nextPointTick = 0;
    this->ptEnvelope.value = 0;
    this->ptEnvelope.tick = 0;
    this->ptEnvelope.nextPointIndex = 0;
    this->ptEnvelope.nextPointTick = 0;

    this->flags = SCFLG_NEW_NOTE | SCFLG_FREQ_CHANGE | SCFLG_RECALC_VOL | SCFLG_RECALC_PAN | SCFLG_ON;
    if( ins.pitchEnv.isEnabled() )
    {
        this->flags |= SCFLG_PITCH_ENV;
    }
    if( ins.panEnv.isEnabled() )
    {
        this->flags |= SCFLG_PAN_ENV;
    }
    if( ins.volEnv.isEnabled() )
    {
        this->flags |= SCFLG_VOL_ENV;
    }

    if( lastSlaveChannel != nullptr )
    {
        if( (ins.volEnv.flg & (8 | Envelope::FlgOn)) == (8 | Envelope::FlgOn) )
        {
            // Transfer volume data
            this->vEnvelope.value = lastSlaveChannel->vEnvelope.value;
            this->vEnvelope.tick = lastSlaveChannel->vEnvelope.tick;
            this->vEnvelope.nextPointIndex = lastSlaveChannel->vEnvelope.nextPointIndex;
            this->vEnvelope.nextPointTick = lastSlaveChannel->vEnvelope.nextPointTick;
        }

        if( (ins.panEnv.flg & (8 | Envelope::FlgOn)) == (8 | Envelope::FlgOn) )
        {
            // Transfer pan data
            this->pEnvelope.value = lastSlaveChannel->pEnvelope.value;
            this->pEnvelope.tick = lastSlaveChannel->pEnvelope.tick;
            this->pEnvelope.nextPointIndex = lastSlaveChannel->pEnvelope.nextPointIndex;
            this->pEnvelope.nextPointTick = lastSlaveChannel->pEnvelope.nextPointTick;
        }

        if( (ins.pitchEnv.flg & (8 | Envelope::FlgOn)) == (8 | Envelope::FlgOn) )
        {
            // Transfer pitch data
            this->ptEnvelope.value = lastSlaveChannel->ptEnvelope.value;
            this->ptEnvelope.tick = lastSlaveChannel->ptEnvelope.tick;
            this->ptEnvelope.nextPointIndex = lastSlaveChannel->ptEnvelope.nextPointIndex;
            this->ptEnvelope.nextPointTick = lastSlaveChannel->ptEnvelope.nextPointTick;
        }
    }

    this->getHost()->flags |= HCFLG_RANDOM; // Apply random volume/pan
    if( this->getHost()->midiChannel == 0 )
    {
        this->filterCutoff = 0x7f;
        this->filterResonance = 0;

        // If IFC bit 7 == 1, then set filter cutoff
        if( (ins.ifc & 0x80u) != 0 )
        {
            this->filterCutoff = ins.ifc & 0x7fu;
        }

        // If IFR bit 7 == 1, then set filter resonance
        if( (ins.ifr & 0x80u) != 0 )
        {
            this->filterResonance = ins.ifr & 0x7fu;
        }
    }
}
}
}
