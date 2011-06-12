/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Syron <mr.syron@googlemail.com>

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

#include "genmod/breseninter.h"
#include "xmchannel.h"
#include "xmmodule.h"

#include <cmath>

namespace ppp { namespace xm {

static const uint8_t KeyOff = 97;

enum {
	EfxFinePortaUp = 1,
	EfxFinePortaDown = 2,
	EfxSetGlissCtrl = 3,
	EfxSetVibratoCtrl = 4,
	EfxSetFinetune = 5,
	EfxPatLoop = 6, //!< @todo Implement!
	EfxSetTremoloCtrl = 7,
	EfxRetrigger = 9,
	EfxFineVolSlideUp = 0x0a,
	EfxFineVolSlideDown = 0x0b,
	EfxNoteCut = 0x0c,
	EfxNoteDelay = 0x0d,
	EfxPatDelay = 0x0e, //!< @todo Implement!
	VfxNone = 0,
	VfxVolSlideDown = 6,
	VfxVolSlideUp = 7,
	VfxFineVolSlideDown = 8,
	VfxFineVolSlideUp = 9,
	VfxSetVibSpeed = 0xa,
	VfxVibrato = 0xb,
	VfxSetPanning = 0xc,
	VfxPanSlideLeft = 0xd,
	VfxPanSlideRight = 0xe,
	VfxPorta = 0xf
};

XmChannel::XmChannel(XmModule *module, int frq) : GenChannel(frq), m_module(module), m_finetune(0), m_bres(1,1), m_glissandoCtrl(false), m_vibratoCtrl(0), m_vibratoPhase(0)
{
}

XmSample::Ptr XmChannel::currentSample()
{
    XmInstrument::Ptr instr = currentInstrument();
	if(instr && m_baseNote>0) {
		XmSample::Ptr smp = instr->mapNoteSample(m_baseNote-1);
		if(smp) {
			m_currentSample = smp;
		}
	}
    return m_currentSample;
}

XmInstrument::Ptr XmChannel::currentInstrument()
{
	XmInstrument::Ptr instr = m_module->getInstrument(m_instrumentIndex);
	if(instr!=m_currentInstrument) {
		m_currentSample.reset();
		m_currentInstrument = instr;
	}
	return m_currentInstrument;
}


void XmChannel::fxPortaUp(uint8_t fxByte)
{
    reuseIfZero(m_lastPortaUpFx, fxByte);
    int tmp = m_basePeriod - (fxByte << 2);
    if(tmp < 1)
        tmp = 1;
    m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::fxPortaDown(uint8_t fxByte)
{
    reuseIfZero(m_lastPortaDownFx, fxByte);
    int tmp = m_basePeriod + (fxByte << 2);
    if(tmp > 0x7cff)
        tmp = 0x7cff;
    m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::fxVolSlide(uint8_t fxByte)
{
    reuseIfZero(m_lastVolSlideFx, fxByte);
    int tmp = m_baseVolume;
    if(highNibble(fxByte) == 0)
        tmp -= lowNibble(fxByte);
    else
        tmp += highNibble(fxByte);
    m_baseVolume = m_currentVolume = clip<int>(tmp, 0, 0x40);
}

void XmChannel::fxPanSlide(uint8_t fxByte)
{
    reuseIfZero(m_lastPanSlideFx, fxByte);
    int tmp = m_panning;
    if(highNibble(fxByte) == 0)
        tmp -= fxByte;
    else
        tmp += highNibble(fxByte);
    m_panning = clip(tmp, 0, 0xff);
}

void XmChannel::triggerNote()
{
	uint8_t note = m_currentCell.getNote();
    if(note == KeyOff) {
        doKeyOff();
		return;
	}
	else if(note == 0) {
		note = m_baseNote;
		if(note == 0) {
			return;
		}
	}
	m_baseNote = note;
	
	if(!currentInstrument()) {
// 		m_instrumentIndex = 0;
		setActive(false);
		return;
	}

	if(!currentSample()) {
		m_realNote = m_baseNote;
// 		m_finetune = 0;
		m_basePeriod = m_currentPeriod = 0;
		setActive(false);
	}
	else {
		int tmp = static_cast<int>(currentSample()->relativeNote()) + m_baseNote;
		if(!inRange(tmp, 1, 119)) {
			LOG_WARNING("OUT OF RANGE NOTE: rel=%d base=%d r=%d", currentSample()->relativeNote(), m_baseNote, tmp);
			setActive(false);
			return;
		}
		m_realNote = tmp-1;
		if(m_currentCell.getEffect()==Effect::Extended && highNibble(m_currentCell.getEffectValue())==EfxSetFinetune) {
			m_finetune = (lowNibble(m_currentCell.getEffectValue())<<4)-0x80;
		}
		else {
			m_finetune = currentSample()->finetune();
		}
		uint16_t newPer = m_module->noteToPeriod( m_realNote, m_finetune );
		if(newPer != 0) {
			m_basePeriod = newPer;
			m_currentPeriod = m_basePeriod;
		}
		if(m_currentCell.getEffect() == Effect::Offset) {
			fxOffset( m_currentCell.getEffectValue() );
		}
		else if(!(m_currentCell.getEffect()==Effect::Porta || m_currentCell.getEffect()==Effect::PortaVolSlide || highNibble(m_currentCell.getVolume())==VfxPorta)) {
			setPosition(0);
		}
		setActive(true);
	}
}

void XmChannel::doKeyOff()
{
	m_keyOn = false;
	XmInstrument::Ptr instr = currentInstrument();
	if(!instr)
		return;
	
	if(!m_volumeEnvelope.enabled()) {
		m_baseVolume = m_currentVolume = 0;
	}
}

void XmChannel::doKeyOn()
{
	if((m_vibratoCtrl&4)==0) {
		m_vibratoPhase = 0;
	}
	m_tremoloPhase = m_retriggerCounter = m_tremorCountdown = 0;
	m_keyOn = true;
	m_volumeEnvelope = currentInstrument()->volumeProcessor();
	m_volScale = 0x8000;
	m_volScaleRate = currentInstrument()->fadeout();
	setActive(true);
}


void XmChannel::update(const XmCell::Ptr cell)
{
    if(m_module->getPlaybackInfo().tick == 0) {
		m_currentCell = *cell;
		
		if((m_vibratoCtrl&4) == 0) {
			// check for vib reset
			if(m_currentCell.getEffect()!=Effect::Vibrato && m_currentCell.getEffect()!=Effect::VibratoVolSlide && highNibble(m_currentCell.getVolume())!=VfxVibrato) {
				m_vibratoPhase = 0;
			}
		}
		
		if(m_currentCell.getInstr() != 0 && m_currentCell.getInstr()<0x80) {
			m_instrumentIndex = m_currentCell.getInstr();
		}
		if(m_currentCell.getEffect()==Effect::Extended) {
			if(highNibble(m_currentCell.getEffectValue())==EfxNoteDelay) {
				if(lowNibble(m_currentCell.getEffectValue())!=0) {
					return;
				}
			}
			else if(highNibble(m_currentCell.getEffectValue()) == EfxRetrigger && lowNibble(m_currentCell.getEffectValue())==0) {
				if(m_currentCell.getNote() != KeyOff) {
					triggerNote();
					applySampleDefaults();
					doKeyOn();
				}
				else {
					doKeyOff();
					applySampleDefaults();
				}
			}
		}
		if(highNibble(m_currentCell.getVolume()) == VfxPorta) {
			if(lowNibble(m_currentCell.getVolume()) != 0) {
				m_portaSpeed = lowNibble(m_currentCell.getVolume())<<4;
			}
			calculatePortaTarget( m_currentCell.getNote() );
		}
		else if(m_currentCell.getEffect()==Effect::Porta) {
			if(m_currentCell.getEffectValue()!=0) {
				m_portaSpeed = m_currentCell.getEffectValue()<<2;
			}
			calculatePortaTarget( m_currentCell.getNote() );
		}
		else if(m_currentCell.getEffect()!=Effect::PortaVolSlide && m_currentCell.getNote() != 0) {
			if(m_currentCell.getNote() == KeyOff) {
				doKeyOff();
			}
			else {
				triggerNote();
				applySampleDefaults();
				doKeyOn();
			}
		}
		switch(highNibble(m_currentCell.getVolume())) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
				m_baseVolume = m_currentCell.getVolume()-0x10;
				m_currentVolume = m_baseVolume;
				break;
			case VfxFineVolSlideDown:
				vfxFineVolSlideDown(m_currentCell.getVolume());
				break;
			case VfxFineVolSlideUp:
				vfxFineVolSlideUp(m_currentCell.getVolume());
				break;
			case VfxSetVibSpeed:
				vfxSetVibratoSpeed(m_currentCell.getVolume());
				break;
			case VfxSetPanning:
				vfxSetPan(m_currentCell.getVolume());
		}
		switch(m_currentCell.getEffect()) {
			case Effect::None:
				break;
			// To get rid of the "enumeration value XY not handled in switch" warnings...
			case Effect::Arpeggio:
			case Effect::PortaUp:
			case Effect::PortaDown:
			case Effect::Porta:
			case Effect::Vibrato:
			case Effect::PortaVolSlide:
			case Effect::VibratoVolSlide:
			case Effect::Tremolo:
			case Effect::VolSlide:
			case Effect::GlobalVolSlide:
			case Effect::KeyOff:
			case Effect::PanSlide:
			case Effect::Tremor:
				// All the above effects are handled in tick 1+
				break;
			case Effect::Offset:
				// this effect is handled in triggerNote()
				break;
			case Effect::SetPanning:
				fxSetPan(m_currentCell.getEffectValue());
				break;
			case Effect::SetVolume:
				fxSetVolume(m_currentCell.getEffectValue());
				break;
			case Effect::Extended:
				fxExtended(m_currentCell.getEffectValue());
				break;
			case Effect::SetTempoBpm:
				fxSetTempoBpm( m_currentCell.getEffectValue() );
				break;
			case Effect::SetGlobalVol:
				fxSetGlobalVolume( m_currentCell.getEffectValue() );
				break;
			case Effect::ExtraFinePorta:
				fxExtraFinePorta(m_currentCell.getEffectValue());
				break;
			case Effect::Retrigger:
				fxRetrigger(m_currentCell.getEffectValue());
				break;
			case Effect::SetEnvPos:
				m_volumeEnvelope.setPosition(m_currentCell.getEffectValue());
				break;
		}
	}
	else { // tick 1+
		if(m_currentCell.getEffect()==Effect::Extended) {
			if(highNibble(m_currentCell.getEffectValue())==EfxNoteDelay) {
				if(lowNibble(m_currentCell.getEffectValue())==m_module->getPlaybackInfo().tick) {
					triggerNote();
					applySampleDefaults();
					doKeyOn();
				}
			}
		}
		switch(highNibble(m_currentCell.getVolume())) {
			case VfxVolSlideDown:
				vfxSlideDown(m_currentCell.getVolume());
				break;
			case VfxVolSlideUp:
				vfxSlideUp(m_currentCell.getVolume());
				break;
			case VfxPanSlideLeft:
				vfxPanSlideLeft(m_currentCell.getVolume());
				break;
			case VfxPanSlideRight:
				vfxPanSlideRight(m_currentCell.getVolume());
				break;
			case VfxPorta:
				fxPorta();
				break;
			case VfxVibrato:
				vfxVibrato(m_currentCell.getVolume());
				break;
		}
		switch(m_currentCell.getEffect()) {
			case Effect::SetPanning:
			case Effect::Offset:
			case Effect::SetVolume:
			case Effect::SetTempoBpm:
			case Effect::SetGlobalVol:
			case Effect::PanSlide:
			case Effect::ExtraFinePorta:
			case Effect::None:
				// already handled
				break;
			case Effect::Arpeggio:
				fxArpeggio(m_currentCell.getEffectValue());
				break;
			case Effect::PortaUp:
				fxPortaUp(m_currentCell.getEffectValue());
				break;
			case Effect::PortaDown:
				fxPortaDown(m_currentCell.getEffectValue());
				break;
			case Effect::VolSlide:
				fxVolSlide(m_currentCell.getEffectValue());
				break;
			case Effect::Extended:
				fxExtended(m_currentCell.getEffectValue());
				break;
			case Effect::Porta:
				fxPorta();
				break;
			case Effect::PortaVolSlide:
				fxPorta();
				fxVolSlide(m_currentCell.getEffectValue());
				break;
			case Effect::Vibrato:
				fxVibrato(m_currentCell.getEffectValue());
				break;
			case Effect::VibratoVolSlide:
				fxVibrato(m_currentCell.getEffectValue());
				fxVolSlide(m_currentCell.getEffectValue());
				break;
			case Effect::GlobalVolSlide:
				fxGlobalVolSlide(m_currentCell.getEffectValue());
				break;
			case Effect::PatBreak:
				m_module->doPatternBreak( m_currentCell.getEffectValue() );
				break;
			case Effect::Tremolo:
				fxTremolo(m_currentCell.getEffectValue());
				break;
			case Effect::KeyOff:
				if(m_module->getPlaybackInfo().tick == m_currentCell.getEffectValue()) {
					doKeyOff();
				}
				break;
			case Effect::Tremor:
				fxTremor(m_currentCell.getEffectValue());
				break;
			case Effect::Retrigger:
				retriggerNote();
				break;
		}
	}
	if(!m_keyOn) {
		if(m_volScale >= m_volScaleRate) {
			m_volScale -= m_volScaleRate;
		}
		else {
			m_volScale = m_volScaleRate = 0;
		}
	}
	m_volumeEnvelope.increasePosition( m_keyOn );
	m_realVolume = m_volumeEnvelope.realVolume(m_currentVolume, m_module->getPlaybackInfo().globalVolume, m_volScale);
 /*   if(!isActive())
        return;
 */   // m_basePeriod = (g_AmigaPeriodTab[(m_relativeNote % 12) * 8] << 4) >> (m_relativeNote / 12);
/*	m_basePeriod = m_module->noteToPeriod( (m_realNote<<4) + ((m_finetune>>3) + 16) );
    m_basePeriod = clip<int>(m_basePeriod, 1, 0x7cff);
	m_currentPeriod = m_basePeriod;*/
	updateStatus();
}

std::string XmChannel::getCellString()
{
    return std::string();
}

std::string XmChannel::getFxDesc() const throw(PppException)
{
    return std::string();
}

std::string XmChannel::getFxName() const throw(PppException)
{
    return std::string();
}

std::string XmChannel::getNoteName() throw(PppException)
{
    return std::string();
}

void XmChannel::mixTick(MixerFrameBuffer &mixBuffer) throw(PppException)
{
    if(!isActive())
        return;
    m_bres.reset(getPlaybackFrq(), m_module->periodToFrequency(m_currentPeriod));
    //BresenInterpolation bres(getPlaybackFrq(), 8363 * 1712 *8 / m_currentPeriod);
    MixerSample *mixBufferPtr = &mixBuffer->front().left;
    XmSample::Ptr currSmp = currentSample();
    int32_t pos = getPosition();
    for(std::size_t i = 0; i < mixBuffer->size(); i++) {
        int16_t sampleVal = currSmp->getLeftSampleAt(pos);
        *(mixBufferPtr++) += (sampleVal * m_realVolume) >> 6;
        sampleVal = currSmp->getRightSampleAt(pos);
        *(mixBufferPtr++) += (sampleVal * m_realVolume) >> 6;
        if(pos == GenSample::EndOfSample)
            break;
        m_bres.next(pos);
    }
    if(pos != GenSample::EndOfSample)
        currentSample()->adjustPos(pos);
    setPosition(pos);
    if(pos == GenSample::EndOfSample)
        setActive(false);
}

void XmChannel::simTick(std::size_t bufSize)
{

}

void XmChannel::updateStatus() throw(PppException)
{
	if(!isActive()) {
		setStatusString("");
		return;
	}
	setStatusString( stringf("Note=%u(%u) vol=%.2u ins=%.2u vfx=%.2x fx=%.2x/%.2x [%s]",
							 m_realNote, m_baseNote, m_realVolume, m_instrumentIndex, m_currentCell.getVolume(), m_currentCell.getEffect(), m_currentCell.getEffectValue(), m_volumeEnvelope.toString().c_str() ));
}

void XmChannel::fxSetVolume(uint8_t fxByte)
{
	if(fxByte > 0x40)
		return;
	m_currentVolume = m_baseVolume = fxByte;
}

void XmChannel::fxSetPan(uint8_t fxByte)
{
	m_panning = fxByte;
}

void XmChannel::fxSetTempoBpm(uint8_t fxByte)
{
	if(fxByte>=1 && fxByte<=0x1f) {
		m_module->setSpeed(fxByte);
	}
	else if(fxByte>=0x20) {
		m_module->setTempo(fxByte);
	}
}

void XmChannel::vfxFineVolSlideDown(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	m_currentVolume = m_baseVolume = std::max<int>(0, m_baseVolume-fxByte);
}

void XmChannel::vfxFineVolSlideUp(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	m_currentVolume = m_baseVolume = std::min<int>(0x40, m_baseVolume+fxByte);
}

void XmChannel::vfxSetPan(uint8_t fxByte)
{
	m_panning = lowNibble(fxByte)<<4;
}

void XmChannel::fxOffset(uint8_t fxByte)
{
	reuseIfZero( m_lastOffsetFx, fxByte );
	setPosition( m_lastOffsetFx << 8 );
}

void XmChannel::vfxSlideDown(uint8_t fxByte)
{
	m_currentVolume = m_baseVolume = std::max<int>(m_baseVolume-lowNibble(fxByte), 0);
}

void XmChannel::vfxSlideUp(uint8_t fxByte)
{
	m_currentVolume = m_baseVolume = std::min<int>(m_baseVolume+lowNibble(fxByte), 0x40);
}

void XmChannel::fxSetGlobalVolume(uint8_t fxByte)
{
	m_module->setGlobalVolume( std::min<uint8_t>(fxByte, 0x40) );
}

void XmChannel::efxFinePortaUp(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	reuseIfZero( m_lastFinePortaUpFx, fxByte );
	int tmp = m_basePeriod - (fxByte<<2);
	if(tmp < 1)
		tmp = 1;
	m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::efxFinePortaDown(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	reuseIfZero( m_lastFinePortaDownFx, fxByte );
	int tmp = m_basePeriod + (fxByte<<2);
	if(tmp > 0x7cff )
		tmp = 0x7cff;
	m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::efxFineVolDown(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	reuseIfZero( m_lastFineVolDownFx, fxByte );
	m_currentVolume = m_baseVolume = std::max(m_baseVolume-fxByte, 0);
}

void XmChannel::efxFineVolUp(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	reuseIfZero( m_lastFineVolUpFx, fxByte );
	m_currentVolume = m_baseVolume = std::min(m_baseVolume+fxByte, 0x40);
}

void XmChannel::fxExtraFinePorta(uint8_t fxByte)
{
	if(highNibble(fxByte)==1) {
		fxByte = lowNibble(fxByte);
		reuseIfZero(m_lastFinePortaUpFx, fxByte);
		m_basePeriod = m_currentPeriod = std::max(1, m_basePeriod - fxByte);
	}
	else if(highNibble(fxByte)==2) {
		fxByte = lowNibble(fxByte);
		reuseIfZero(m_lastFinePortaDownFx, fxByte);
		m_basePeriod = m_currentPeriod = std::min(0x7cff, m_basePeriod + fxByte);
	}
}

void XmChannel::fxExtended(uint8_t fxByte)
{
	switch(highNibble(fxByte)) {
		case EfxFinePortaUp:
			efxFinePortaUp(fxByte);
			break;
		case EfxFinePortaDown:
			efxFinePortaDown(fxByte);
			break;
		case EfxFineVolSlideUp:
			efxFineVolUp(fxByte);
			break;
		case EfxFineVolSlideDown:
			efxFineVolDown(fxByte);
			break;
		case EfxSetGlissCtrl:
			m_glissandoCtrl = lowNibble(fxByte)!=0;
			break;
		case EfxSetVibratoCtrl:
			m_vibratoCtrl = lowNibble(fxByte);
			break;
		case EfxSetTremoloCtrl:
			m_tremoloCtrl = lowNibble(fxByte);
			break;
		case EfxNoteCut:
			if(m_module->getPlaybackInfo().tick == lowNibble(fxByte)) {
				m_baseVolume = m_currentVolume = 0;
			}
			break;
		case EfxNoteDelay:
			if(m_module->getPlaybackInfo().tick == lowNibble(fxByte)) {
				triggerNote();
				applySampleDefaults();
				doKeyOn();
				if(m_currentCell.getVolume()>=0x10 && m_currentCell.getVolume()<=0x5f) {
					m_currentVolume = m_baseVolume = m_currentCell.getVolume()-0x10;
				}
				else if(highNibble(fxByte) == VfxSetPanning) {
					vfxSetPan(fxByte);
				}
			}
			break;
		case EfxRetrigger:
			if(lowNibble(fxByte) != 0) {
				if(m_module->getPlaybackInfo().tick%lowNibble(fxByte) == 0) {
					triggerNote();
					doKeyOn();
				}
			}
			break;
	}
}

void XmChannel::applySampleDefaults()
{
	XmSample::Ptr smp = currentSample();
	if(m_currentCell.getInstr()!=0 && smp) {
		m_baseVolume = m_currentVolume = smp->getVolume();
		m_panning = smp->panning();
	}
}

void XmChannel::vfxSetVibratoSpeed(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte)<<2;
	if(fxByte)
		m_vibratoSpeed = fxByte;
}

void XmChannel::vfxPanSlideLeft(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	m_panning = std::max<int>(0, m_panning-fxByte);
}

void XmChannel::vfxPanSlideRight(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	m_panning = std::min<int>(0xff, m_panning+fxByte);
}

void XmChannel::fxPorta()
{
	int tmp = m_basePeriod;
	if(m_portaTargetPeriod<m_basePeriod) {
		tmp = m_basePeriod-m_portaSpeed;
		if(tmp<m_portaTargetPeriod) {
			tmp = m_portaTargetPeriod;
		}
	}
	else if(m_portaTargetPeriod>m_basePeriod) {
		tmp = m_basePeriod+m_portaSpeed;
		if(tmp>m_portaTargetPeriod) {
			tmp = m_portaTargetPeriod;
		}
	}
	else {
		return;
	}
	m_basePeriod = tmp;
	if(!m_glissandoCtrl) {
		m_currentPeriod = m_basePeriod;
	}
	else {
		m_currentPeriod = m_module->glissando(m_basePeriod, m_finetune);
	}
}

void XmChannel::calculatePortaTarget(uint8_t targetNote)
{
	if(!currentSample())
		return;
	if(targetNote!=0 && targetNote!=KeyOff) {
		uint16_t newPer = m_module->noteToPeriod(targetNote + currentSample()->relativeNote() - 1, currentSample()->finetune());
		if(newPer != 0) {
			m_portaTargetPeriod = newPer;
		}
	}
}

void XmChannel::fxArpeggio(uint8_t fxByte)
{
	if(fxByte == 0) {
		return;
	}
	switch( m_module->getPlaybackInfo().tick % 3) {
		case 0:
			m_currentPeriod = m_basePeriod;
			break;
		case 1:
			m_currentPeriod = m_module->glissando( m_basePeriod, m_finetune, highNibble(fxByte) );
			break;
		case 2:
			m_currentPeriod = m_module->glissando( m_basePeriod, m_finetune, lowNibble(fxByte) );
			break;
	}
}

void XmChannel::fxVibrato(uint8_t fxByte)
{
	if(fxByte != 0) {
		if(lowNibble(fxByte) != 0) {
			m_vibratoDepth = lowNibble(fxByte);
		}
		if(highNibble(fxByte)>>2 != 0) {
			m_vibratoSpeed = highNibble(fxByte)>>2;
		}
	}
	doVibrato();
}

static const std::array<uint8_t, 32> g_VibTable = {{
	0, 24, 49, 74, 97,120,141,161,180,197,212,224,235,
	244,250,253,255,253,250,244,235,224,212,197,180,161,
	141,120, 97, 74, 49, 24
}};

void XmChannel::doVibrato()
{
	uint8_t value = (m_vibratoPhase>>2) & 0x1f;
	switch(m_vibratoCtrl&3) {
		case 0:
			value = g_VibTable[value];
			break;
		case 1:
			value <<= 3;
			if((m_vibratoPhase&0x80) == 0) {
				value = ~value;
			}
			break;
		case 2:
			value = 0xff;
	}
	uint16_t delta = (value*m_vibratoDepth)>>5;
	if(m_vibratoPhase&0x80) {
		m_currentPeriod = m_basePeriod - delta;
	}
	else {
		m_currentPeriod = m_basePeriod + delta;
	}
	m_vibratoPhase += m_vibratoSpeed;
}

void XmChannel::vfxVibrato(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	if(fxByte != 0) {
		m_vibratoDepth = fxByte;
	}
	doVibrato();
}

void XmChannel::fxGlobalVolSlide(uint8_t fxByte)
{
	reuseIfZero( m_lastGlobVolSlideFx, fxByte );
	if(highNibble(fxByte)==0) {
		fxByte = lowNibble(fxByte);
		m_module->setGlobalVolume( std::max(0, m_module->getPlaybackInfo().globalVolume-fxByte ) );
	}
	else {
		fxByte = highNibble(fxByte);
		m_module->setGlobalVolume( std::min(0x40, m_module->getPlaybackInfo().globalVolume+fxByte ) );
	}
}

void XmChannel::fxTremolo(uint8_t fxByte)
{
	if(highNibble(fxByte)!=0) {
		m_tremoloSpeed = highNibble(fxByte)<<2;
	}
	if(lowNibble(fxByte)!=0) {
		m_tremoloDepth = lowNibble(fxByte);
	}
	uint8_t value = (m_tremoloPhase>>2) & 0x1f;
	switch(m_tremoloCtrl&3) {
		case 0:
			value = g_VibTable[value];
			break;
		case 1:
			value <<= 3;
			// This is _NOT_ a typo or c&p error!
			if((m_vibratoPhase&0x80) == 0) {
				value = ~value;
			}
			break;
		case 2:
			value = 0xff;
	}
	uint16_t delta = (value*m_tremoloDepth)>>6;
	if(m_tremoloPhase&0x80) {
		m_currentVolume = m_baseVolume - delta;
	}
	else {
		m_currentVolume = m_baseVolume + delta;
	}
	m_tremoloPhase += m_tremoloSpeed;
}

void XmChannel::fxTremor(uint8_t fxByte)
{
	reuseIfZero(m_lastTremorFx, fxByte);
	if((m_tremorCountdown&0x7f) == 0) {
		if((m_tremorCountdown&0x80) != 0) {
			m_tremorCountdown = lowNibble(fxByte);
		}
		else {
			m_tremorCountdown = 0x80 | highNibble(fxByte);
		}
	}
	else {
		m_tremorCountdown = (m_tremorCountdown&0x80) | ((m_tremorCountdown&0x7f)-1);
	}
	if(m_tremorCountdown&0x80) {
		m_currentVolume = m_baseVolume;
	}
	else {
		m_currentVolume = 0;
	}
}

void XmChannel::retriggerNote()
{
	m_retriggerCounter++;
	if(m_retriggerCounter < m_retriggerLength) {
		return;
	}
	m_retriggerCounter = 0;
	int newVol = m_baseVolume;
	switch(m_retriggerVolumeType) {
		case 0:
		case 8:
			break;
		case 1:
			newVol = std::max(0, newVol-1);
			break;
		case 2:
			newVol = std::max(0, newVol-2);
			break;
		case 3:
			newVol = std::max(0, newVol-4);
			break;
		case 4:
			newVol = std::max(0, newVol-8);
			break;
		case 5:
			newVol = std::max(0, newVol-16);
			break;
		case 6:
			newVol >>= 1;
			newVol += (newVol>>2) + (newVol>>3);
			break;
		case 7:
			newVol >>= 1;
			break;
		case 9:
			newVol = std::min(0x40, newVol+1);
			break;
		case 10:
			newVol = std::min(0x40, newVol+2);
			break;
		case 11:
			newVol = std::min(0x40, newVol+4);
			break;
		case 12:
			newVol = std::min(0x40, newVol+8);
			break;
		case 13:
			newVol = std::min(0x40, newVol+16);
			break;
		case 14:
			newVol += newVol>>1;
			if(newVol>0x40) {
				newVol = 0x40;
			}
			break;
		case 15:
			newVol <<= 1;
			if(newVol>0x40) {
				newVol = 0x40;
			}
			break;
	}
	m_currentVolume = m_baseVolume = newVol;
	if(m_currentCell.getVolume()>=0x10 && m_currentCell.getVolume()<=0x50) {
		m_currentVolume = m_baseVolume = m_currentCell.getVolume()-0x10;
	}
	else if(highNibble(m_currentCell.getVolume()) == VfxSetPanning) {
		vfxSetPan(m_currentCell.getVolume());
	}
	triggerNote();
	doKeyOn();
}

void XmChannel::fxRetrigger(uint8_t fxByte)
{
	if(lowNibble(fxByte)!=0) {
		m_retriggerLength = lowNibble(fxByte);
	}
	if(highNibble(fxByte)!=0) {
		m_retriggerVolumeType = highNibble(fxByte);
	}
	if(m_currentCell.getNote() != 0) {
		retriggerNote();
	}
}

}}
