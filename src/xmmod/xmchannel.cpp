/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @ingroup XmModule
 * @{
 */

#include "xmchannel.h"
#include "genmod/genbase.h"
#include <genmod/standardfxdesc.h>
#include "xmmodule.h"
#include "xmcell.h"
#include "xmsample.h"
#include "xminstrument.h"
#include "xmbase.h"

#include <cmath>

namespace ppp
{
namespace xm
{
XmChannel::XmChannel(ppp::xm::XmModule* module)
  : m_currentCell( new XmCell() )
  , m_module( module )
{
  m_state.instrument = 0;
  m_reverse = false;
}

XmChannel::~XmChannel()
{
  delete m_currentCell;
}

ChannelState XmChannel::status() const
{
  return m_state;
}

const std::unique_ptr<XmSample>& XmChannel::currentSample() const
{
  const auto& instr = currentInstrument();
  if( instr && m_lastNote > 0 )
  {
    return instr->mapNoteSample( m_lastNote - 1 );
  }
  static const std::unique_ptr<XmSample> none;
  return none;
}

const std::unique_ptr<XmInstrument>& XmChannel::currentInstrument() const
{
  return m_module->getInstrument( m_state.instrument );
}

void XmChannel::fxPortaUp(uint8_t fxByte)
{
  m_lastPortaUpFx = fxByte;
  int tmp = m_basePeriod - (m_lastPortaUpFx << 2);
  if( tmp < 1 )
  {
    tmp = 1;
  }
  m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::fxPortaDown(uint8_t fxByte)
{
  m_lastPortaDownFx = fxByte;
  int tmp = m_basePeriod + (m_lastPortaDownFx << 2);
  if( tmp > 0x7cff )
  {
    tmp = 0x7cff;
  }
  m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::fxVolSlide(uint8_t fxByte)
{
  m_lastVolSlideFx = fxByte;
  int tmp = m_baseVolume;
  if( m_lastVolSlideFx.hi() == 0 )
  {
    tmp -= m_lastVolSlideFx.lo();
  }
  else
  {
    tmp += m_lastVolSlideFx.hi();
  }
  m_baseVolume = m_currentVolume = clip<int>( tmp, 0, 0x40 );
}

void XmChannel::fxPanSlide(uint8_t fxByte)
{
  m_lastPanSlideFx = fxByte;
  int tmp = m_panning;
  if( m_lastPanSlideFx.hi() == 0 )
  {
    tmp -= m_lastPanSlideFx.lo();
  }
  else
  {
    tmp += m_lastPanSlideFx.hi();
  }
  m_panning = clip( tmp, 0, 0xff );
}

void XmChannel::triggerNote(uint8_t note)
{
  m_state.active = true;
  m_state.noteTriggered = true;
  if( note == KeyOffNote )
  {
    doKeyOff();
    return;
  }
  m_lastNote = note;
  m_reverse = false;

  if( m_lastNote == 0 )
  {
    return;
  }

  const auto& instr = currentInstrument();
  if( instr )
  {
    m_panningEnvelope = instr->panningProcessor();
    m_volumeEnvelope = instr->volumeProcessor();
  }

  if( !currentSample() )
  {
    // 		setActive(false);
    return;
  }

  note = m_lastNote + currentSample()->relativeNote();
  if( !between<uint8_t>( note, 0, 119 ) )
  {
    return;
  }

  if( m_currentCell->effect() == Effect::Extended && (m_currentCell->effectValue() >> 4) == EfxSetFinetune )
  {
    m_finetune = ((m_currentCell->effectValue() & 0x0f) << 4) - 0x80;
  }
  else
  {
    m_finetune = currentSample()->finetune();
  }

  if( note != 0 )
  {
    if( uint16_t newPer = m_module->noteToPeriod( note - 1, m_finetune ) )
    {
      m_currentPeriod = m_basePeriod = newPer;
    }
  }

  if( m_currentCell->effect() == Effect::Offset )
  {
    fxOffset( m_currentCell->effectValue() );
  }
  else
  {
    m_stepper = 0;
  }
}

void XmChannel::doKeyOff()
{
  m_keyOn = false;
  if( !m_panningEnvelope.enabled() )
  {
    m_panningEnvelope.doKeyOff();
  }
  if( !m_volumeEnvelope.enabled() )
  {
    m_baseVolume = m_currentVolume = 0;
  }
  else
  {
    m_volumeEnvelope.doKeyOff();
  }
  m_state.note = ChannelState::KeyOff;
}

void XmChannel::doKeyOn()
{
  if( !currentInstrument() )
  {
    return;
  }

  m_keyOn = true;
  m_state.noteTriggered = true;
  m_tremoloPhase = 0;
  m_retriggerCounter = 0;
  m_tremorCountdown = 0;
  m_state.active = true;

  if( (m_vibratoCtrl & 4) == 0 )
  {
    m_vibratoPhase = 0;
  }
  m_tremoloPhase = m_retriggerCounter = m_tremorCountdown = 0;

  if( m_panningEnvelope.enabled() )
  {
    m_panningEnvelope.retrigger();
  }
  if( m_volumeEnvelope.enabled() )
  {
    m_volumeEnvelope.retrigger();
  }

  m_volScale = 0x8000;
  m_volScaleRate = currentInstrument()->fadeout();

  if( currentInstrument()->vibDepth() == 0 )
  {
    return;
  }

  m_autoVibPhase = 0;
  if( currentInstrument()->vibSweep() == 0 )
  {
    m_autoVibDepth = currentInstrument()->vibDepth() << 8;
    m_autoVibSweepRate = 0;
  }
  else
  {
    m_autoVibDepth = 0;
    m_autoVibSweepRate = m_autoVibDepth / currentInstrument()->vibSweep();
  }
}

constexpr std::array<const int8_t, 256> g_AutoVibTable = { {
                                                             0, -2, -3, -5, -6, -8, -9, -11, -12, -14, -16,
                                                             -17, -19, -20, -22, -23, -24, -26, -27, -29, -30, -32,
                                                             -33, -34, -36, -37, -38, -39, -41, -42, -43, -44, -45,
                                                             -46, -47, -48, -49, -50, -51, -52, -53, -54, -55, -56,
                                                             -56, -57, -58, -59, -59, -60, -60, -61, -61, -62, -62,
                                                             -62, -63, -63, -63, -64, -64, -64, -64, -64, -64, -64,
                                                             -64, -64, -64, -64, -63, -63, -63, -62, -62, -62, -61,
                                                             -61, -60, -60, -59, -59, -58, -57, -56, -56, -55, -54,
                                                             -53, -52, -51, -50, -49, -48, -47, -46, -45, -44, -43,
                                                             -42, -41, -39, -38, -37, -36, -34, -33, -32, -30, -29,
                                                             -27, -26, -24, -23, -22, -20, -19, -17, -16, -14, -12,
                                                             -11, -9, -8, -6, -5, -3, -2, 0, 2, 3, 5,
                                                             6, 8, 9, 11, 12, 14, 16, 17, 19, 20, 22,
                                                             23, 24, 26, 27, 29, 30, 32, 33, 34, 36, 37,
                                                             38, 39, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                                                             50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
                                                             59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63,
                                                             64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                                                             63, 63, 63, 62, 62, 62, 61, 61, 60, 60, 59,
                                                             59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50,
                                                             49, 48, 47, 46, 45, 44, 43, 42, 41, 39, 38,
                                                             37, 36, 34, 33, 32, 30, 29, 27, 26, 24, 23,
                                                             22, 20, 19, 17, 16, 14, 12, 11, 9, 8, 6,
                                                             5, 3, 2
                                                           }
};

void XmChannel::updateTick0(const XmCell& cell, bool estimateOnly)
{
  if( estimateOnly )
  {
    switch( cell.effect() )
    {
    case Effect::Extended:
      fxExtended( cell.effectValue(), true );
      break;
    case Effect::SetTempoBpm:
      fxSetTempoBpm( cell.effectValue() );
      break;
    default:
      // silence "not handled" warnings
      break;
    }
    return;
  }

  m_state.noteTriggered = false;

  {
    bool vibratoContinued = m_currentCell->effectValue() != 0;
    vibratoContinued &=
      m_currentCell->effect() == Effect::Vibrato || m_currentCell->effect() == Effect::VibratoVolSlide;
    vibratoContinued &= (cell.effect() == Effect::Vibrato || cell.effect() == Effect::VibratoVolSlide);
    if( !vibratoContinued )
    {
      m_currentPeriod = m_basePeriod;
    }
  }

  *m_currentCell = cell;

  if( between<uint8_t>( m_currentCell->instrument(), 1, 0x80 ) )
  {
    m_state.instrument = m_currentCell->instrument();
    m_reverse = false;
  }

  if( m_currentCell->effect() == Effect::Extended && (m_currentCell->effectValue() >> 4) == EfxNoteDelay
    && (m_currentCell->effectValue() & 0x0f) != 0 )
  {
    // note delay, but not on tick 0
    return;
  }

  enum
  {
    Nothing,
    KeyOffCheck,
    KeyOnCheck,
    TriggerCheck
  } nextCheck = Nothing;

  if( m_currentCell->effect() == Effect::Extended && (m_currentCell->effectValue() >> 4) == EfxRetrigger )
  {
    if( (m_currentCell->effectValue() & 0x0f) == 0 )
    {
      // retrigger every frame
      nextCheck = TriggerCheck;
    }
    if( m_currentCell->note() == KeyOffNote )
    {
      nextCheck = KeyOffCheck;
    }
    else if( m_currentCell->note() != 0 )
    {
      calculatePortaTarget( m_currentCell->note() );
      nextCheck = KeyOnCheck;
    }
  }
  else if( (m_currentCell->volume() >> 4u) == VfxPorta )
  {
    if( (m_currentCell->volume() & 0x0fu) != 0 )
    {
      m_portaSpeed = (m_currentCell->volume() & 0x0fu) * 64u;
    }
    if( m_currentCell->note() == KeyOffNote )
    {
      nextCheck = KeyOffCheck;
    }
    else
    {
      if( m_currentCell->note() != 0 )
      {
        calculatePortaTarget( m_currentCell->note() );
      }
      nextCheck = KeyOnCheck;
    }
  }
  else if( m_currentCell->effect() == Effect::Porta || m_currentCell->effect() == Effect::PortaVolSlide )
  {
    if( m_currentCell->effect() != Effect::PortaVolSlide && m_currentCell->effectValue() != 0 )
    {
      m_portaSpeed = m_currentCell->effectValue() * 4u;
    }
    if( m_currentCell->note() == KeyOffNote )
    {
      nextCheck = KeyOffCheck;
    }
    else
    {
      if( m_currentCell->note() != 0 )
      {
        calculatePortaTarget( m_currentCell->note() );
      }
      nextCheck = KeyOnCheck;
    }
  }
  else if( m_currentCell->effect() == Effect::KeyOff && m_currentCell->effectValue() == 0 )
  {
    nextCheck = KeyOffCheck;
  }
  else if( m_currentCell->note() == 0 )
  {
    nextCheck = KeyOnCheck;
  }
  else
  {
    nextCheck = TriggerCheck;
  }

  if( nextCheck == TriggerCheck )
  {
    if( m_lastNote == KeyOffNote )
    {
      nextCheck = KeyOffCheck;
    }
    else
    {
      triggerNote( m_currentCell->note() );
      nextCheck = KeyOnCheck;
    }
  }
  if( nextCheck == KeyOffCheck )
  {
    doKeyOff();
    if( m_currentCell->instrument() != 0 )
    {
      applySampleDefaults();
    }
    nextCheck = Nothing;
  }
  else if( nextCheck == KeyOnCheck )
  {
    if( m_currentCell->instrument() != 0 )
    {
      applySampleDefaults();
      doKeyOn();
    }
    nextCheck = Nothing;
  }
  BOOST_ASSERT( nextCheck == Nothing );

  m_state.fxDesc = fxdesc::NullFx;
  switch( m_currentCell->volume() >> 4u )
  {
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x04:
  case 0x05:
    m_currentVolume = m_baseVolume = clip( m_currentCell->volume() - 0x10, 0, 0x40 );
    break;
  case VfxFineVolSlideDown:
    vfxFineVolSlideDown( m_currentCell->volume() );
    m_state.fxDesc = fxdesc::SlowVolSlideDown;
    break;
  case VfxFineVolSlideUp:
    vfxFineVolSlideUp( m_currentCell->volume() );
    m_state.fxDesc = fxdesc::SlowVolSlideUp;
    break;
  case VfxSetVibSpeed:
    vfxSetVibratoSpeed( m_currentCell->volume() );
    m_state.fxDesc = fxdesc::Vibrato;
    break;
  case VfxSetPanning:
    vfxSetPan( m_currentCell->volume() );
    m_state.fxDesc = fxdesc::SetPanPos;
    break;
  case VfxVolSlideDown:
    m_state.fxDesc = fxdesc::VolSlideDown;
    break;
  case VfxVolSlideUp:
    m_state.fxDesc = fxdesc::VolSlideUp;
    break;
  case VfxPanSlideLeft:
    m_state.fxDesc = fxdesc::PanSlideLeft;
    break;
  case VfxPanSlideRight:
    m_state.fxDesc = fxdesc::PanSlideRight;
    break;
  case VfxPorta:
    m_state.fxDesc = fxdesc::Porta;
    break;
  case VfxVibrato:
    m_state.fxDesc = fxdesc::Vibrato;
    break;
  }
  switch( m_currentCell->effect() )
  {
  case Effect::None:
    break;
  case Effect::Arpeggio:
    m_state.fxDesc = fxdesc::Arpeggio;
    break;
  case Effect::PortaUp:
    m_state.fxDesc = fxdesc::FastPitchSlideUp;
    break;
  case Effect::PortaDown:
    m_state.fxDesc = fxdesc::FastPitchSlideDown;
    break;
  case Effect::Porta:
    m_state.fxDesc = fxdesc::Porta;
    break;
  case Effect::Vibrato:
    m_state.fxDesc = fxdesc::Vibrato;
    break;
  case Effect::PortaVolSlide:
    m_state.fxDesc = fxdesc::PortaVolSlide;
    break;
  case Effect::VibratoVolSlide:
    m_state.fxDesc = fxdesc::VibVolSlide;
    break;
  case Effect::Tremolo:
    m_state.fxDesc = fxdesc::Tremolo;
    break;
  case Effect::VolSlide:
    if( m_lastVolSlideFx.hi() == 0 )
    {
      m_state.fxDesc = fxdesc::VolSlideDown;
    }
    else
    {
      m_state.fxDesc = fxdesc::VolSlideUp;
    }
    break;
  case Effect::GlobalVolSlide:
    if( m_lastGlobVolSlideFx.hi() == 0 )
    {
      m_state.fxDesc = fxdesc::GlobalVolSlideDown;
    }
    else
    {
      m_state.fxDesc = fxdesc::GlobalVolSlideUp;
    }
    break;
  case Effect::KeyOff:
    m_state.fxDesc = fxdesc::KeyOff;
    break;
  case Effect::PanSlide:
    if( m_lastPanSlideFx.hi() == 0 )
    {
      m_state.fxDesc = fxdesc::PanSlideLeft;
    }
    else
    {
      m_state.fxDesc = fxdesc::PanSlideRight;
    }
    break;
  case Effect::Tremor:
    m_state.fxDesc = fxdesc::Tremor;
    break;
  case Effect::PosJump:
    m_state.fxDesc = fxdesc::JumpOrder;
    break;
  case Effect::PatBreak:
    // All the above effects are handled in tick 1+
    m_state.fxDesc = fxdesc::PatternBreak;
    break;
  case Effect::Offset:
    // this effect is handled in triggerNote()
    m_state.fxDesc = fxdesc::Offset;
    break;
  case Effect::SetPanning:
    fxSetPan( m_currentCell->effectValue() );
    m_state.fxDesc = fxdesc::SetPanPos;
    break;
  case Effect::SetVolume:
    fxSetVolume( m_currentCell->effectValue() );
    m_state.fxDesc = fxdesc::SetVolume;
    break;
  case Effect::Extended:
    fxExtended( m_currentCell->effectValue(), false );
    break;
  case Effect::SetTempoBpm:
    fxSetTempoBpm( m_currentCell->effectValue() );
    m_state.fxDesc = fxdesc::SetTempo;
    break;
  case Effect::SetGlobalVol:
    fxSetGlobalVolume( m_currentCell->effectValue() );
    m_state.fxDesc = fxdesc::GlobalVolume;
    break;
  case Effect::ExtraFinePorta:
    fxExtraFinePorta( m_currentCell->effectValue() );
    if( (m_currentCell->effectValue() >> 4) == 1 )
    {
      m_state.fxDesc = fxdesc::SlowPitchSlideUp;
    }
    else if( (m_currentCell->effectValue() >> 4) == 2 )
    {
      m_state.fxDesc = fxdesc::SlowPitchSlideDown;
    }
    else
    {
      m_state.fxDesc = fxdesc::NullFx;
    }
    break;
  case Effect::Retrigger:
    fxRetrigger( m_currentCell->effectValue() );
    m_state.fxDesc = fxdesc::Retrigger;
    break;
  case Effect::SetEnvPos:
    m_volumeEnvelope.setPosition( m_currentCell->effectValue() );
    m_panningEnvelope.setPosition( m_currentCell->effectValue() );
    m_state.fxDesc = fxdesc::SetEnvelopePos;
    break;
  }
}

void XmChannel::updateTick1(const XmCell& cell, bool estimateOnly)
{
  if( estimateOnly )
  {
    switch( cell.effect() )
    {
    case Effect::Extended:
      fxExtended( cell.effectValue(), true );
      break;
    case Effect::PatBreak:
      m_module->doPatternBreak( (cell.effectValue() >> 4) * 10 + (cell.effectValue() & 0x0f) );
      break;
    case Effect::PosJump:
      m_module->doJumpPos( cell.effectValue() );
      break;
    default:
      // silence "not handled" warnings
      break;
    }
    return;
  }
  if( m_currentCell->effect() == Effect::Extended )
  {
    if( (m_currentCell->effectValue() >> 4) == EfxNoteDelay )
    {
      if( (m_currentCell->effectValue() & 0x0f) == m_module->state().tick )
      {
        triggerNote( m_currentCell->note() );
        applySampleDefaults();
        doKeyOn();
      }
    }
  }
  switch( m_currentCell->volume() >> 4 )
  {
  case VfxVolSlideDown:
    vfxSlideDown( m_currentCell->volume() );
    break;
  case VfxVolSlideUp:
    vfxSlideUp( m_currentCell->volume() );
    break;
  case VfxPanSlideLeft:
    vfxPanSlideLeft( m_currentCell->volume() );
    break;
  case VfxPanSlideRight:
    vfxPanSlideRight( m_currentCell->volume() );
    break;
  case VfxPorta:
    fxPorta();
    break;
  case VfxVibrato:
    vfxVibrato( m_currentCell->volume() );
    break;
  }
  switch( m_currentCell->effect() )
  {
  case Effect::SetPanning:
  case Effect::Offset:
  case Effect::SetVolume:
  case Effect::SetTempoBpm:
  case Effect::SetGlobalVol:
  case Effect::PanSlide:
  case Effect::ExtraFinePorta:
  case Effect::None:
  case Effect::SetEnvPos:
    // already handled
    break;
  case Effect::Arpeggio:
    fxArpeggio( m_currentCell->effectValue() );
    break;
  case Effect::PortaUp:
    fxPortaUp( m_currentCell->effectValue() );
    break;
  case Effect::PortaDown:
    fxPortaDown( m_currentCell->effectValue() );
    break;
  case Effect::VolSlide:
    fxVolSlide( m_currentCell->effectValue() );
    break;
  case Effect::Extended:
    fxExtended( m_currentCell->effectValue(), false );
    break;
  case Effect::Porta:
    fxPorta();
    break;
  case Effect::PortaVolSlide:
    fxPorta();
    fxVolSlide( m_currentCell->effectValue() );
    break;
  case Effect::Vibrato:
    fxVibrato( m_currentCell->effectValue() );
    break;
  case Effect::VibratoVolSlide:
    fxVibrato( m_currentCell->effectValue() );
    fxVolSlide( m_currentCell->effectValue() );
    break;
  case Effect::GlobalVolSlide:
    fxGlobalVolSlide( m_currentCell->effectValue() );
    break;
  case Effect::PatBreak:
    m_module->doPatternBreak( (m_currentCell->effectValue() >> 4) * 10 + (m_currentCell->effectValue() & 0x0f) );
    break;
  case Effect::PosJump:
    m_module->doJumpPos( m_currentCell->effectValue() );
    break;
  case Effect::Tremolo:
    fxTremolo( m_currentCell->effectValue() );
    break;
  case Effect::KeyOff:
    if( m_module->state().tick == m_currentCell->effectValue() )
    {
      doKeyOff();
    }
    break;
  case Effect::Tremor:
    fxTremor( m_currentCell->effectValue() );
    break;
  case Effect::Retrigger:
    retriggerNote();
    break;
  }
}

void XmChannel::update(const XmCell& cell, bool estimateOnly)
{
  m_state.cell = cell.trackerString();
  if( cell.effect() == Effect::None )
  {
    m_state.fx = 0;
  }
  else
  {
    if( uint8_t( cell.effect() ) <= 9 )
    {
      m_state.fx = '0' + uint8_t( cell.effect() );
    }
    else
    {
      m_state.fx = 'A' + uint8_t( cell.effect() ) - 0x0a;
    }
  }
  if( m_module->state().tick == 0 && !m_module->isRunningPatDelay() )
  {
    updateTick0( cell, estimateOnly );
  }
  else
  { // tick 1+
    updateTick1( cell, estimateOnly );
  }

  if( !m_keyOn )
  {
    if( m_volScale >= m_volScaleRate )
    {
      m_volScale -= m_volScaleRate;
    }
    else
    {
      m_volScale = m_volScaleRate = 0;
    }
  }
  m_volumeEnvelope.increasePosition( m_keyOn );
  m_realVolume = m_volumeEnvelope.realVolume( m_currentVolume, m_module->state().globalVolume, m_volScale );
  m_panningEnvelope.increasePosition( m_keyOn );
  m_realPanning = m_panningEnvelope.realPanning( m_panning );

  if( currentInstrument() )
  {
    if( m_autoVibSweepRate != 0 && m_autoVibDepth != 0 && m_keyOn )
    {
      uint16_t tmp = m_autoVibSweepRate + m_autoVibDepth;
      if( tmp > currentInstrument()->vibDepth() )
      {
        m_autoVibSweepRate = 0;
        m_autoVibDeltaPeriod = 0;
        tmp = currentInstrument()->vibDepth();
      }
      m_autoVibDepth = tmp;
    }
    m_autoVibPhase += currentInstrument()->vibRate();
    int8_t value = 0;
    switch( m_vibratoCtrl & 3 )
    {
    case 0:
      value = g_AutoVibTable[m_autoVibPhase];
      break;
    case 1:
      if( (m_autoVibPhase & 0x80) == 0 )
      {
        value = -0x40;
      }
      else
      {
        value = 0x40;
      }
      break;
    case 2:
      value = (m_autoVibPhase >> 1) + 0x40;
      value &= 0x7f;
      value -= 0x40;
      break;
    case 3:
      value = -(m_autoVibPhase >> 1) + 0x40;
      value &= 0x7f;
      value -= 0x40;
      break;
    }
    uint16_t newPeriod = (value * m_autoVibDepth >> 14) + m_currentPeriod;
    if( newPeriod > 0x7cff )
    {
      m_autoVibDeltaPeriod = 0;
    }
    else
    {
      m_autoVibDeltaPeriod = value * m_autoVibDepth >> 14;
    }
  }
  else
  {
    m_autoVibDeltaPeriod = 0;
  }

  updateStatus();

  if( m_currentVolume == 0 && m_realVolume == 0 && m_volScale == 0 )
  {
    m_state.active = false;
  }
}

void XmChannel::mixTick(const MixerFrameBufferPtr& mixBuffer)
{
  if( !m_state.active || !mixBuffer )
  {
    return;
  }
  m_stepper.setStepSize( m_module->periodToFrequency( m_currentPeriod + m_autoVibDeltaPeriod ), m_module->frequency() );
  const auto& currSmp = currentSample();
  if( !currSmp )
  {
    m_state.active = false;
    return;
  }
  int volLeft = std::round( std::sqrt( m_realPanning / 256.0f ) * 65536 ) * m_realVolume / 256;
  int volRight = std::round( std::sqrt( (256 - m_realPanning) / 256.0f ) * 65536 ) * m_realVolume / 256;
  m_state.active =
    mix( *currSmp,
         currSmp->loopType(),
         m_module->interpolation(),
         m_stepper,
         *mixBuffer,
         m_reverse,
         currSmp->loopStart(),
         currSmp->loopEnd(),
         volLeft,
         volRight,
         13,
         false ) != 0;
}

void XmChannel::updateStatus()
{
  if( m_realPanning == 0xff )
  {
    m_state.panning = 100;
  }
  else
  {
    m_state.panning = (m_realPanning - 0x80) * 100 / 0x80;
  }
  m_state.volume = clip<int>( m_realVolume, 0, 0x40 ) * 100 / 0x40;

  if( m_currentCell->note() == KeyOffNote )
  {
    m_state.note = ChannelState::KeyOff;
  }
  else if( !currentSample() )
  {
    m_state.note = ChannelState::NoNote;
  }
  else
  {
    float fofs = m_module->periodToFineNoteIndex( m_currentPeriod + m_autoVibDeltaPeriod, m_finetune );
    fofs -= m_finetune / 8.0 + 16;
    fofs /= 16;
    fofs -= currentSample()->relativeNote();
    if( fofs < 0 )
    {
      m_state.note = ChannelState::TooLow;
    }
    else if( fofs > ChannelState::MaxNote )
    {
      m_state.note = ChannelState::TooHigh;
    }
    else
    {
      m_state.note = std::lround( fofs );
    }
  }

  if( const auto& ins = currentInstrument() )
  {
    m_state.instrumentName = ins->title();
  }
  else
  {
    m_state.instrumentName.clear();
  }
}

void XmChannel::fxSetVolume(uint8_t fxByte)
{
  m_currentVolume = m_baseVolume = std::min<int>( 0x40, fxByte );
}

void XmChannel::fxSetPan(uint8_t fxByte)
{
  m_panning = fxByte;
}

void XmChannel::fxSetTempoBpm(uint8_t fxByte)
{
  if( fxByte >= 1 && fxByte <= 0x1f )
  {
    m_module->setSpeed( fxByte );
  }
  else if( fxByte >= 0x20 )
  {
    m_module->setTempo( fxByte );
  }
}

void XmChannel::vfxFineVolSlideDown(uint8_t fxByte)
{
  fxByte &= 0x0f;
  m_currentVolume = m_baseVolume = std::max<int>( 0, m_baseVolume - fxByte );
}

void XmChannel::vfxFineVolSlideUp(uint8_t fxByte)
{
  fxByte &= 0x0f;
  m_currentVolume = m_baseVolume = std::min<int>( 0x40, m_baseVolume + fxByte );
}

void XmChannel::vfxSetPan(uint8_t fxByte)
{
  m_panning = fxByte << 4;
}

void XmChannel::fxOffset(uint8_t fxByte)
{
  m_lastOffsetFx = fxByte;
  m_stepper = m_lastOffsetFx << 8;
  if( !currentSample() || (currentSample() && m_stepper.trunc() >= currentSample()->length()) )
  {
    m_state.active = false;
  }
}

void XmChannel::vfxSlideDown(uint8_t fxByte)
{
  m_currentVolume = m_baseVolume = std::max<int>( m_baseVolume - (fxByte & 0x0f), 0 );
}

void XmChannel::vfxSlideUp(uint8_t fxByte)
{
  m_currentVolume = m_baseVolume = std::min<int>( m_baseVolume + (fxByte & 0x0f), 0x40 );
}

void XmChannel::fxSetGlobalVolume(uint8_t fxByte)
{
  m_module->state().globalVolume = std::min<uint8_t>( fxByte, 0x40 );
}

void XmChannel::efxFinePortaUp(uint8_t fxByte)
{
  m_lastFinePortaUpFx = fxByte & 0x0f;
  uint16_t tmp = m_basePeriod - (m_lastFinePortaUpFx << 2);
  if( tmp < 1 )
  {
    tmp = 1;
  }
  m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::efxFinePortaDown(uint8_t fxByte)
{
  m_lastFinePortaDownFx = fxByte & 0x0f;
  uint16_t tmp = m_basePeriod + (m_lastFinePortaDownFx << 2);
  if( tmp > 0x7cff )
  {
    tmp = 0x7cff;
  }
  m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::efxFineVolDown(uint8_t fxByte)
{
  m_lastFineVolDownFx = fxByte & 0x0f;
  m_currentVolume = m_baseVolume = std::max<int>( m_baseVolume - m_lastFineVolDownFx, 0 );
}

void XmChannel::efxFineVolUp(uint8_t fxByte)
{
  m_lastFineVolUpFx = fxByte & 0x0f;
  m_currentVolume = m_baseVolume = std::min<int>( m_baseVolume + m_lastFineVolUpFx, 0x40 );
}

void XmChannel::fxExtraFinePorta(uint8_t fxByte)
{
  if( (fxByte >> 4) == 1 )
  {
    m_lastXFinePortaUp = fxByte & 0x0f;
    m_basePeriod = m_currentPeriod = std::max( 1, m_basePeriod - m_lastXFinePortaUp );
  }
  else if( (fxByte >> 4) == 2 )
  {
    m_lastXFinePortaDown = fxByte & 0x0f;
    m_basePeriod = m_currentPeriod = std::min( 0x7cff, m_basePeriod + m_lastXFinePortaDown );
  }
}

void XmChannel::fxExtended(uint8_t fxByte, bool estimateOnly)
{
  if( estimateOnly )
  {
    switch( fxByte >> 4 )
    {
    case EfxPatLoop:
      if( m_module->state().tick == 0 )
      {
        efxPatLoop( fxByte );
      }
      break;
    case EfxPatDelay:
      if( m_module->state().tick == 0 )
      {
        m_module->doPatDelay( fxByte & 0x0f );
      }
      break;
    }
    return;
  }
  switch( fxByte >> 4 )
  {
  case EfxFinePortaUp:
    efxFinePortaUp( fxByte );
    m_state.fxDesc = fxdesc::SlowPitchSlideUp;
    break;
  case EfxFinePortaDown:
    efxFinePortaDown( fxByte );
    m_state.fxDesc = fxdesc::SlowPitchSlideDown;
    break;
  case EfxFineVolSlideUp:
    efxFineVolUp( fxByte );
    m_state.fxDesc = fxdesc::SlowVolSlideUp;
    break;
  case EfxFineVolSlideDown:
    efxFineVolDown( fxByte );
    m_state.fxDesc = fxdesc::SlowVolSlideDown;
    break;
  case EfxSetGlissCtrl:
    m_glissandoCtrl = (fxByte & 0x0f) != 0;
    m_state.fxDesc = fxdesc::Glissando;
    break;
  case EfxSetVibratoCtrl:
    m_vibratoCtrl = fxByte & 0x0f;
    m_state.fxDesc = fxdesc::SetVibWaveform;
    break;
  case EfxSetTremoloCtrl:
    m_tremoloCtrl = fxByte & 0x0f;
    m_state.fxDesc = fxdesc::SetTremWaveform;
    break;
  case EfxNoteCut:
    if( m_module->state().tick == (fxByte & 0x0f) )
    {
      m_baseVolume = m_currentVolume = 0;
    }
    m_state.fxDesc = fxdesc::NoteCut;
    break;
  case EfxNoteDelay:
    m_state.fxDesc = fxdesc::NoteDelay;
    if( m_module->state().tick == (fxByte & 0x0f) )
    {
      triggerNote( m_currentCell->note() );
      if( m_currentCell->instrument() != 0 )
      {
        applySampleDefaults();
      }
      doKeyOn();
      if( m_currentCell->volume() >= 0x10 && m_currentCell->volume() <= 0x50 )
      {
        m_currentVolume = m_baseVolume = m_currentCell->volume() - 0x10;
      }
      else if( (m_currentCell->volume() >> 4) == VfxSetPanning )
      {
        m_panning = m_currentCell->volume() << 4;
      }
    }
    break;
  case EfxRetrigger:
    if( (fxByte & 0x0f) != 0 )
    {
      if( m_module->state().tick % (fxByte & 0x0f) == 0 )
      {
        triggerNote( 0 );
        doKeyOn();
      }
    }
    m_state.fxDesc = fxdesc::Retrigger;
    break;
  case EfxPatLoop:
    if( m_module->state().tick == 0 )
    {
      efxPatLoop( fxByte );
    }
    m_state.fxDesc = fxdesc::PatternLoop;
    break;
  case EfxPatDelay:
    if( m_module->state().tick == 0 )
    {
      m_module->doPatDelay( fxByte & 0x0f );
    }
    m_state.fxDesc = fxdesc::PatternDelay;
    break;
  case EfxSetPan:
    if( m_module->state().tick == 0 )
    {
      m_panning = 0xff * (fxByte & 0x0f) / 0x0f;
    }
    m_state.fxDesc = fxdesc::SetPanPos;
    break;
  }
}

void XmChannel::applySampleDefaults()
{
  const auto& smp = currentSample();
  if( m_currentCell->instrument() != 0 && smp )
  {
    m_baseVolume = m_currentVolume = smp->volume();
    m_panning = smp->panning();
  }
}

void XmChannel::vfxSetVibratoSpeed(uint8_t fxByte)
{
  m_vibratoSpeed = (fxByte & 0x0f) << 2;
}

void XmChannel::vfxPanSlideLeft(uint8_t fxByte)
{
  fxByte &= 0x0f;
  m_panning = std::max<int>( 0, m_panning - fxByte );
}

void XmChannel::vfxPanSlideRight(uint8_t fxByte)
{
  fxByte &= 0x0f;
  m_panning = std::min<int>( 0xff, m_panning + fxByte );
}

void XmChannel::fxPorta()
{
  int tmp = m_basePeriod;
  if( m_portaTargetPeriod < m_basePeriod )
  {
    tmp -= m_portaSpeed;
    if( tmp < m_portaTargetPeriod )
    {
      tmp = m_portaTargetPeriod;
    }
  }
  else if( m_portaTargetPeriod > m_basePeriod )
  {
    tmp += m_portaSpeed;
    if( tmp > m_portaTargetPeriod )
    {
      tmp = m_portaTargetPeriod;
    }
  }
  else
  {
    return;
  }
  m_basePeriod = tmp;
  if( !m_glissandoCtrl )
  {
    m_currentPeriod = m_basePeriod;
  }
  else
  {
    m_currentPeriod = m_module->glissando( m_basePeriod, m_finetune );
  }
}

void XmChannel::calculatePortaTarget(uint8_t tarnote)
{
  if( !currentSample() )
  {
    return;
  }
  if( tarnote != 0 && tarnote != KeyOffNote )
  {
    uint16_t newPer = m_module->noteToPeriod( tarnote + currentSample()->relativeNote() - 1, m_finetune );
    if( newPer != 0 )
    {
      m_portaTargetPeriod = newPer;
    }
  }
}

void XmChannel::fxArpeggio(uint8_t fxByte)
{
  if( fxByte == 0 )
  {
    return;
  }
  switch( m_module->state().tick % 3 )
  {
  case 0:
    m_currentPeriod = m_basePeriod;
    break;
  case 1:
    m_currentPeriod = m_module->glissando( m_basePeriod, m_finetune, fxByte >> 4 );
    break;
  case 2:
    m_currentPeriod = m_module->glissando( m_basePeriod, m_finetune, fxByte & 0x0f );
    break;
  }
}

void XmChannel::fxVibrato(uint8_t fxByte)
{
  if( (fxByte & 0x0f) != 0 )
  {
    m_vibratoDepth = fxByte & 0x0f;
  }
  if( (fxByte >> 4) != 0 )
  {
    m_vibratoSpeed = (fxByte >> 4) << 2;
  }
  doVibrato();
}

constexpr std::array<const uint8_t, 32> g_VibTable = { {
                                                         0, 24, 49, 74, 97, 120, 141, 161, 180, 197, 212, 224, 235,
                                                         244, 250, 253, 255, 253, 250, 244, 235, 224, 212, 197, 180,
                                                         161,
                                                         141, 120, 97, 74, 49, 24
                                                       }
};

void XmChannel::doVibrato()
{
  uint8_t value = (m_vibratoPhase >> 2) & 0x1f;
  switch( m_vibratoCtrl & 3 )
  {
  case 0:
    value = g_VibTable[value];
    break;
  case 1:
    value <<= 3;
    if( (m_vibratoPhase & 0x80) == 0 )
    {
      value = ~value;
    }
    break;
  default:
    value = 0xff;
  }
  uint16_t delta = (value * m_vibratoDepth) >> 5;
  if( m_vibratoPhase & 0x80 )
  {
    m_currentPeriod = m_basePeriod - delta;
  }
  else
  {
    m_currentPeriod = m_basePeriod + delta;
  }
  m_vibratoPhase += m_vibratoSpeed;
}

void XmChannel::vfxVibrato(uint8_t fxByte)
{
  fxByte &= 0x0f;
  if( fxByte != 0 )
  {
    m_vibratoDepth = fxByte;
  }
  doVibrato();
}

void XmChannel::fxGlobalVolSlide(uint8_t fxByte)
{
  // 	reuseIfZero( m_lastGlobVolSlideFx, fxByte );
  m_lastGlobVolSlideFx = fxByte;
  if( m_lastGlobVolSlideFx.hi() == 0 )
  {
    m_module->state().globalVolume = std::max( 0, m_module->state().globalVolume - m_lastGlobVolSlideFx.lo() );
  }
  else
  {
    m_module->state().globalVolume = std::min( 0x40, m_module->state().globalVolume + m_lastGlobVolSlideFx.hi() );
  }
}

void XmChannel::fxTremolo(uint8_t fxByte)
{
  if( (fxByte >> 4) != 0 )
  {
    m_tremoloSpeed = (fxByte >> 4) << 2;
  }
  if( (fxByte & 0x0f) != 0 )
  {
    m_tremoloDepth = (fxByte & 0x0f);
  }
  uint8_t value = (m_tremoloPhase >> 2) & 0x1f;
  switch( m_tremoloCtrl & 3 )
  {
  case 0:
    value = g_VibTable[value];
    break;
  case 1:
    value <<= 3;
    // This is _NOT_ a typo or c&p error!
    if( (m_vibratoPhase & 0x80) == 0 )
    {
      value = ~value;
    }
    break;
  default:
    value = 0xff;
  }
  uint16_t delta = (value * m_tremoloDepth) >> 6;
  if( m_tremoloPhase & 0x80 )
  {
    m_currentVolume = clip( m_baseVolume - delta, 0, 64 );
  }
  else
  {
    m_currentVolume = clip( m_baseVolume + delta, 0, 64 );
  }
  m_tremoloPhase += m_tremoloSpeed;
}

void XmChannel::fxTremor(uint8_t fxByte)
{
  // 	reuseIfZero( m_lastTremorFx, fxByte );
  m_lastTremorFx = fxByte;
  if( (m_tremorCountdown & 0x7f) == 0 )
  {
    if( (m_tremorCountdown & 0x80) != 0 )
    {
      m_tremorCountdown = m_lastTremorFx.lo();
    }
    else
    {
      m_tremorCountdown = 0x80 | m_lastTremorFx.hi();
    }
  }
  else
  {
    m_tremorCountdown = (m_tremorCountdown & 0x80) | ((m_tremorCountdown & 0x7f) - 1);
  }
  if( m_tremorCountdown & 0x80 )
  {
    m_currentVolume = m_baseVolume;
  }
  else
  {
    m_currentVolume = 0;
  }
}

void XmChannel::retriggerNote()
{
  m_retriggerCounter++;
  if( m_retriggerCounter < m_retriggerLength )
  {
    return;
  }
  m_retriggerCounter = 0;
  int newVol = m_baseVolume;
  switch( m_retriggerVolumeType )
  {
  case 0:
  case 8:
    break;
  case 1:
    newVol -= 1;
    break;
  case 2:
    newVol -= 2;
    break;
  case 3:
    newVol -= 4;
    break;
  case 4:
    newVol -= 8;
    break;
  case 5:
    newVol -= 16;
    break;
  case 6:
  {
    newVol >>= 1;
    int tmp = newVol >> 2;
    newVol += tmp;
    tmp >>= 1;
    newVol += tmp;
  }
    break;
  case 7:
    newVol >>= 1;
    break;
  case 9:
    newVol += 1;
    break;
  case 10:
    newVol += 2;
    break;
  case 11:
    newVol += 4;
    break;
  case 12:
    newVol += 8;
    break;
  case 13:
    newVol += 16;
    break;
  case 14:
    newVol += newVol >> 1;
    break;
  case 15:
    newVol <<= 1;
    break;
  }
  m_currentVolume = m_baseVolume = clip( newVol, 0, 0x40 );
  const uint8_t cellVolFx = m_currentCell->volume();
  if( cellVolFx >= 0x10 && cellVolFx <= 0x50 )
  {
    m_currentVolume = m_baseVolume = cellVolFx - 0x10;
  }
  else if( (cellVolFx >> 4) == VfxSetPanning )
  {
    m_panning = (cellVolFx & 0x0f) << 4;
  }
  triggerNote( 0 );
}

void XmChannel::fxRetrigger(uint8_t fxByte)
{
  if( m_module->state().tick != 0 )
  {
    retriggerNote();
    return;
  }
  m_retriggerLength = fxByte & 0x0f;
  m_retriggerVolumeType = fxByte >> 4;
  if( m_currentCell->note() != 0 )
  {
    retriggerNote();
  }
}

void XmChannel::efxPatLoop(uint8_t fxByte)
{
  fxByte &= 0x0f;
  if( fxByte == 0 )
  {
    m_patLoopRow = m_module->state().row;
    return;
  }
  if( m_patLoopCounter == 0 )
  {
    // when the loop counter is zero, this is the first loop running,
    // so set the counter and wait for the next loop
    m_patLoopCounter = fxByte;
    m_module->doPatLoop( m_patLoopRow );
    return;
  }
  // loop counter is not zero, meaning we are in a loop. in this case
  // decrease it. if it is not zero then, we must jump back.
  m_patLoopCounter--;
  if( m_patLoopCounter != 0 )
  {
    m_module->doPatLoop( m_patLoopRow );
  }
}

AbstractArchive& XmChannel::serialize(AbstractArchive* data)
{
  *data
    % m_baseVolume
    % m_currentVolume
    % m_realVolume
    % m_panning
    % m_realPanning
    % m_basePeriod
    % m_currentPeriod
    % m_autoVibDeltaPeriod
    % m_finetune
    % m_realNote
    % m_currentCell
    % m_volumeEnvelope
    % m_panningEnvelope
    % m_volScale
    % m_volScaleRate
    % m_keyOn
    % m_lastVolSlideFx
    % m_lastPortaUpFx
    % m_lastPortaDownFx
    % m_lastPanSlideFx
    % m_lastOffsetFx
    % m_lastFinePortaUpFx
    % m_lastFinePortaDownFx
    % m_lastFineVolUpFx
    % m_lastFineVolDownFx
    % m_lastXFinePortaUp
    % m_lastXFinePortaDown
    % m_lastGlobVolSlideFx
    % m_lastTremorFx
    % m_portaSpeed
    % m_portaTargetPeriod
    % m_vibratoSpeed
    % m_vibratoDepth
    % m_vibratoPhase
    % m_vibratoCtrl
    % m_glissandoCtrl
    % m_tremoloDepth
    % m_tremoloSpeed
    % m_tremoloPhase
    % m_tremoloCtrl
    % m_tremorCountdown
    % m_retriggerCounter
    % m_retriggerLength
    % m_retriggerVolumeType
    % m_patLoopCounter
    % m_patLoopRow
    % m_autoVibType
    % m_autoVibSweep
    % m_autoVibSweepRate
    % m_autoVibDepth
    % m_autoVibPhase
    % m_lastNote
    % m_stepper
    % m_state
    % m_reverse;
  return *data;
}
}
}

/**
 * @}
 */