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
	EfxPatLoop = 6,
	EfxSetTremoloCtrl = 7,
	EfxRetrigger = 9,
	EfxFineVolSlideUp = 0x0a,
	EfxFineVolSlideDown = 0x0b,
	EfxNoteCut = 0x0c,
	EfxNoteDelay = 0x0d,
	EfxPatDelay = 0x0e,
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

XmChannel::XmChannel(XmModule *module, int frq) : GenChannel(frq), m_module(module), m_finetune(0), m_panEnvIdx(0), m_volEnvIdx(0), m_bres(1,1)
{
}

XmSample::Ptr XmChannel::currentSample()
{
    XmInstrument::Ptr instr = currentInstrument();
	if(instr) {
		XmSample::Ptr smp = instr->mapNoteSample(m_baseNote);
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
		m_finetune = 0;
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
		if(m_currentCell.getEffect()==Effect::FxExtended && highNibble(m_currentCell.getEffectValue())==EfxSetFinetune) {
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
		if(m_currentCell.getEffect() == Effect::FxOffset) {
			fxOffset( m_currentCell.getEffectValue() );
		}
		else if(!(m_currentCell.getEffect()==Effect::FxPorta || m_currentCell.getEffect()==Effect::FxPortaVolSlide || highNibble(m_currentCell.getVolume())==VfxPorta)) {
			setPosition(0);
		}
		setActive(true);
	}
}

void XmChannel::doKeyOff()
{
	m_volFadeoutVal = 0;
	XmInstrument::Ptr instr = currentInstrument();
	if(!instr)
		return;
	
	if(!(instr->panEnvFlags() & XmInstrument::EnvelopeFlags::Enabled)) {
		uint16_t pos = instr->panPoint( m_panEnvIdx ).position;
		if(m_panEnvPos >= pos) {
			m_panEnvPos = pos-1;
		}
	}
	if(instr->panEnvFlags() & XmInstrument::EnvelopeFlags::Enabled) {
		uint16_t pos = instr->volPoint( m_volEnvIdx ).position;
		if(m_volEnvPos >= pos) {
			m_volEnvPos = pos-1;
		}
	}
	else {
		m_baseVolume = m_currentVolume = 0;
	}
	// FIXME
    setActive(false);
}

void XmChannel::update(const XmCell::Ptr cell)
{
// 	m_currentCell = *cell;
    if(m_module->getPlaybackInfo().tick == 0) {
		m_currentCell = *cell;
		if(m_currentCell.getInstr() != 0 && m_currentCell.getInstr()<0x80) {
			m_instrumentIndex = m_currentCell.getInstr();
		}
		if(m_currentCell.getEffect()==Effect::FxExtended) {
			if(highNibble(m_currentCell.getEffectValue())==EfxNoteDelay) {
				if(lowNibble(m_currentCell.getEffectValue())!=0) {
					return;
				}
			}
			else if(highNibble(m_currentCell.getEffectValue()) == EfxRetrigger && lowNibble(m_currentCell.getEffectValue())==0) {
				if(m_currentCell.getNote() != KeyOff) {
					triggerNote();
					applySampleDefaults();
					// TODO resetVibPanEnv
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
		else if(m_currentCell.getEffect()==Effect::FxPorta) {
			if(m_currentCell.getEffectValue()!=0) {
				m_portaSpeed = m_currentCell.getEffectValue()<<2;
			}
			calculatePortaTarget( m_currentCell.getNote() );
		}
		else if(m_currentCell.getEffect()!=Effect::FxPortaVolSlide && m_currentCell.getNote() != 0) {
			if(m_currentCell.getNote() == KeyOff) {
				doKeyOff();
			}
			else {
				triggerNote();
				applySampleDefaults();
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
			case Effect::FxSetPanning:
				fxSetPan(m_currentCell.getEffectValue());
				break;
			case Effect::FxSetVolume:
				fxSetVolume(m_currentCell.getEffectValue());
				break;
			case Effect::FxExtended:
				fxExtended(m_currentCell.getEffectValue());
				break;
			case Effect::FxSetTempoBpm:
				fxSetTempoBpm( m_currentCell.getEffectValue() );
				break;
			case Effect::FxSetGlobalVol:
				fxSetGlobalVolume( m_currentCell.getEffectValue() );
				break;
			case Effect::FxExtraFinePorta:
				fxExtraFinePorta(m_currentCell.getEffectValue());
				break;
		}
	}
	else { // tick 1+
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
		}
		switch(m_currentCell.getEffect()) {
			case Effect::FxPortaUp:
				fxPortaUp(m_currentCell.getEffectValue());
				break;
			case Effect::FxPortaDown:
				fxPortaDown(m_currentCell.getEffectValue());
				break;
			case Effect::FxVolSlide:
				fxVolSlide(m_currentCell.getEffectValue());
				break;
			case Effect::FxExtended:
				fxExtended(m_currentCell.getEffectValue());
				break;
			case Effect::FxPorta:
				fxPorta();
				break;
			case Effect::FxPortaVolSlide:
				fxPorta();
				fxVolSlide(m_currentCell.getEffectValue());
				break;
		}
	}
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
        *(mixBufferPtr++) += (sampleVal * m_currentVolume) >> 6;
        sampleVal = currSmp->getRightSampleAt(pos);
        *(mixBufferPtr++) += (sampleVal * m_currentVolume) >> 6;
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
	setStatusString( stringf("Note=%u(%u) vol=%.2u ins=%.2u vfx=%.2x fx=%.2x/%.2x frq=%u pos=%d",
							 m_realNote, m_baseNote, m_currentVolume, m_instrumentIndex, m_currentCell.getVolume(), m_currentCell.getEffect(), m_currentCell.getEffectValue(), m_module->periodToFrequency(m_currentPeriod), getPosition() ));
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
	m_currentPeriod = m_basePeriod = tmp;
	// TODO glissando
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


}}
