#include "slavechannel.h"

#include "sample.h"
#include "genmod/sample.h"
#include "instrument.h"
#include "sample.h"
#include "itmodule.h"
#include "hostchannel.h"
#include "itdata.h"

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
  this->fadeOut = 1024;
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

  if( newPan != SurroundPan )
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

  BOOST_ASSERT( newPan <= 64 || newPan == SurroundPan );
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

void SlaveChannel::doFadeOut()
{
  flags |= SCFLG_FADEOUT;
  fadeOut -= insOffs->fadeout;
  if( fadeOut <= 0 )
  {
    fadeOut = 0;

    // Turn off channel
    if( !disowned )
    {
      disowned = true;
      getHost()->disable();
    }

    flags |= SCFLG_NOTE_CUT;
  }

  flags |= SCFLG_RECALC_VOL;
}

void SlaveChannel::updateVibrato()
{
  if( smpOffs->header.vid == 0 || smpOffs->header.vis == 0 )
  {
    return;
  }

  vip += smpOffs->header.vis;

  auto depth = static_cast<uint16_t>((viDepth + smpOffs->header.vir) / 256u);
  if( depth > smpOffs->header.vid )
  {
    depth = smpOffs->header.vid;
  }

  viDepth = depth;

  int value;
  switch( smpOffs->header.vit )
  {
  case 0:
    value = fineSineData[vip];
    break;
  case 1:
    value = fineRampDownData[vip];
    break;
  case 2:
    value = fineSquareWave[vip];
    break;
  case 3:
  default:
    value = (std::rand() & 0x7f) - 64;
  }

  value *= depth * 4;
  value /= 256;
  if( value < 0 )
  {
    pitchSlideDownLinear( -value );
  }
  else if( value > 0 )
  {
    pitchSlideUpLinear( value );
  }
}

void SlaveChannel::recalculate(int16_t globalVolume)
{
  if( (flags & SCFLG_RECALC_VOL) != 0 )
  {
    flags &= ~SCFLG_RECALC_VOL;
    flags |= SCFLG_RECALC_FINAL_VOL;
    int volume = effectiveVolume * channelVolume; // AX = (0->4096)
    BOOST_ASSERT( volume >= 0 && volume <= 64 * 64 );

    volume *= fadeOut; // Fadeout Vol (0->1024), DX:AX = 0->4194304
    BOOST_ASSERT( volume >= 0 && volume <= 64 * 64 * 1024 );

    volume /= 128; // AX = (0->32768)
    BOOST_ASSERT( volume >= 0 && volume <= 64 * 64 * 8 );

    volume *= sampleVolume; // DX:AX = 0->4194304
    volume /= 128; // AX = 0->32768
    BOOST_ASSERT( volume >= 0 && volume <= 64 * 64 * 8 );

    BOOST_ASSERT( vEnvelope.value.trunc() >= 0 && vEnvelope.value.trunc() <= 64 );
    volume *= vEnvelope.value.trunc();
    volume /= 64;
    BOOST_ASSERT( volume >= 0 && volume <= 64 * 64 * 8 );

    volume *= globalVolume;
    volume /= 128;
    BOOST_ASSERT( volume >= 0 && volume <= 32768 );

    _16bVol = volume;
  }

  if( (flags & SCFLG_RECALC_PAN) != 0 )
  {
    flags &= ~SCFLG_RECALC_PAN;
    flags |= SCFLG_PAN_CHANGE;

    if( pan == SurroundPan )
    {
      midiFinalPan = SurroundPan;
      finalPan = SurroundPan;
    }
    else
    {
      auto value = 32 - std::abs( 32 - pan );
      BOOST_ASSERT( value >= -32 && value <= 32 );
      value = value * pEnvelope.value.trunc() / 32;
      BOOST_ASSERT( value >= -32 && value <= 32 );
      value += pan;
      BOOST_ASSERT( value >= 0 && value <= 64 );

      midiFinalPan = value;
      value -= 32;
      BOOST_ASSERT( value >= -32 && value <= 32 );
      value *= insOffs->pitchPanSeparation / 2; // AX = -2048->+2048
      BOOST_ASSERT( value >= -2048 && value <= 2048 );
      value /= 64; // AL = -32->+32
      BOOST_ASSERT( value >= -32 && value <= 32 );
      value += 32;
      finalPan = value;
    }
  }

  updateVibrato();
}

void SlaveChannel::applyRandom()
{
  if( insOffs->rv != 0 )
  {
    auto vol = sampleVolume + (insOffs->rv * ((std::rand() & 0xff) - 128) / 64 + 1) * sampleVolume / 199;
    if( vol < 0 )
    {
      sampleVolume = 0;
    }
    else if( vol > 128 )
    {
      sampleVolume = 128;
    }
    else
    {
      sampleVolume = static_cast<uint8_t>(vol);
    }
  }

  if( insOffs->rp != 0 && pan != SurroundPan )
  {
    const auto newPan = pan + insOffs->rp * ((std::rand() & 0xff) - 128) / 128;
    if( newPan < 0 )
    {
      pan = 0;
      ps = 0;
    }
    else if( pan > 64 )
    {
      pan = 64;
      ps = 64;
    }
    else
    {
      pan = static_cast<uint8_t>(newPan);
      ps = static_cast<uint8_t>(newPan);
      BOOST_ASSERT( pan <= 64 || pan == SurroundPan );
    }
  }
}
}
}
