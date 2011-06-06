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

using namespace ppp;
using namespace ppp::xm;

enum Effect {
	FxArpeggio = 0,
	FxPortaUp = 1,
	FxPortaDown = 2,
	FxPorta = 3,
	FxVibrato = 4,
	FxPortaVolSlide = 5,
	FxVibratoVolSlide = 6,
	FxTremolo = 7,
	FxSetPanning = 8,
	FxOffset = 9,
	FxVolSlide = 0x0a,
	FxPosJump = 0x0b,
	FxSetVolume = 0x0c,
	FxPatBreak = 0x0d,
	FxExtended = 0x0e,
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
	FxSetTempoBpm = 0x0f,
	FxSetGlobalVol = 0x10,
	FxGlobalVolSlide = 0x11,
	FxSetEnvPos = 0x15,
	FxPanSlide = 0x19,
	FxRetrigger = 0x1b,
	FxTremor = 0x1d,
	FxExtraFinePorta = 0x21,
	XfxUp = 1,
	XfxDown = 2
};
enum Special {
	KeyOff = 97
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
		m_basePeriod = m_currentPeriod = m_portaPeriod = 0;
		setActive(false);
	}
	else {
		int tmp = currentSample()->relativeNote() + m_baseNote;
		if(!inRange(tmp, 1, 119)) {
			LOG_WARNING("OUT OF RANGE NOTE: rel=%d base=%d r=%d", currentSample()->relativeNote(), m_baseNote, tmp);
			setActive(false);
			return;
		}
		m_realNote = tmp;
		if(m_currentCell.getEffect()==FxExtended && highNibble(m_currentCell.getEffectValue())==EfxSetFinetune) {
			m_finetune = (lowNibble(m_currentCell.getEffectValue())<<4)-0x80;
		}
		else {
			m_finetune = currentSample()->finetune();
		}
		uint16_t newPer = m_module->noteToPeriod( m_realNote, m_finetune );
		if(newPer != 0) {
			m_basePeriod = newPer;
			m_portaPeriod = m_currentPeriod = m_basePeriod;
		}
		if(m_currentCell.getEffect() == FxOffset) {
			fxOffset( m_currentCell.getEffectValue() );
		}
		else {
			setPosition(0);
		}
		setActive(true);
	}
}

void XmChannel::doKeyOff()
{
	return setActive(false); // FIXME

	m_volFadeoutVal = 0;
	XmInstrument::Ptr instr = currentInstrument();
	
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
//     setActive(false);
}

void XmChannel::update(const XmCell::Ptr cell)
{
// 	m_currentCell = *cell;
    if(m_module->getPlaybackInfo().tick == 0) {
		m_currentPeriod = m_basePeriod;
		m_currentCell = *cell;
		if(m_currentCell.getInstr() != 0 && m_currentCell.getInstr()<0x80) {
			m_instrumentIndex = m_currentCell.getInstr();
		}
		if(m_currentCell.getEffect()==FxExtended) {
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
		if(highNibble(m_currentCell.getVolume()) == 0x0f) {
			if(lowNibble(m_currentCell.getVolume()) != 0) {
				m_portaSpeed = lowNibble(m_currentCell.getVolume())<<4;
			}
			if(m_currentCell.getNote()!=0 && m_currentCell.getNote()!=KeyOff) {
				uint16_t newPer = m_module->noteToPeriod(m_currentCell.getNote(), m_finetune);
				if(newPer != 0) {
					m_portaTargetPeriod = newPer;
					if(m_portaTargetPeriod == m_basePeriod) {
						m_portaDir = PortaDir::Off;
					}
					else if(m_portaTargetPeriod < m_basePeriod) {
						m_portaDir = PortaDir::Down;
					}
					else {
						m_portaDir = PortaDir::Up;
					}
				}
			}
		}
		else if(m_currentCell.getEffect()==FxPorta) {
			m_portaSpeed = m_currentCell.getEffectValue()<<2;
			if(m_currentCell.getNote()!=0 && m_currentCell.getNote()!=KeyOff) {
				uint16_t newPer = m_module->noteToPeriod(m_currentCell.getNote(), m_finetune);
				if(newPer != 0) {
					m_portaTargetPeriod = newPer;
					if(m_portaTargetPeriod == m_basePeriod) {
						m_portaDir = PortaDir::Off;
					}
					else if(m_portaTargetPeriod < m_basePeriod) {
						m_portaDir = PortaDir::Down;
					}
					else {
						m_portaDir = PortaDir::Up;
					}
				}
			}
		}
		if(m_currentCell.getNote() == KeyOff) {
			doKeyOff();
		}
		else if(m_currentCell.getNote() != 0) {
			triggerNote();
			applySampleDefaults();
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
			case 0x08:
				vfxFineVolSlideDown(m_currentCell.getVolume());
				break;
			case 0x09:
				vfxFineVolSlideUp(m_currentCell.getVolume());
				break;
			case 0x0a:
				vfxSetVibratoSpeed(m_currentCell.getVolume());
				break;
			case 0x0c:
				vfxSetPan(m_currentCell.getVolume());
		}
		switch(m_currentCell.getEffect()) {
			case FxSetPanning:
				fxSetPan(m_currentCell.getEffectValue());
				break;
			case FxSetVolume:
				fxSetVolume(m_currentCell.getEffectValue());
				break;
			case FxExtended:
				fxExtended(m_currentCell.getEffectValue());
				break;
			case FxSetTempoBpm:
				fxSetTempoBpm( m_currentCell.getEffectValue() );
				break;
			case FxSetGlobalVol:
				fxSetGlobalVolume( m_currentCell.getEffectValue() );
				break;
			case FxExtraFinePorta:
// 				fxExtraFinePorta(m_currentCell.getEffectValue());
				break;
		}
	}
	else { // tick 1+
		switch(highNibble(m_currentCell.getVolume())) {
			case 0x06:
				vfxSlideDown(m_currentCell.getVolume());
				break;
			case 0x07:
				vfxSlideUp(m_currentCell.getVolume());
				break;
			case 0x0d:
				vfxPanSlideLeft(m_currentCell.getVolume());
				break;
			case 0x0e:
				vfxPanSlideRight(m_currentCell.getVolume());
				break;
			case 0x0f:
// 				fxPorta();
				break;
		}
		switch(m_currentCell.getEffect()) {
			case FxPortaUp:
				fxPortaUp(m_currentCell.getEffectValue());
				break;
			case FxPortaDown:
				fxPortaDown(m_currentCell.getEffectValue());
				break;
			case FxVolSlide:
				fxVolSlide(m_currentCell.getEffectValue());
				break;
			case FxExtended:
				fxExtended(m_currentCell.getEffectValue());
				break;
			case FxPorta:
// 				fxPorta();
				break;
			case FxPortaVolSlide:
// 				fxPorta();
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
	if(m_portaDir == PortaDir::Off) {
		return;
	}
	uint16_t tmp;
	if(m_portaDir == PortaDir::Up) {
		tmp = m_basePeriod+m_portaSpeed;
		if(tmp>m_portaTargetPeriod) {
			tmp = m_portaTargetPeriod;
			m_portaDir = PortaDir::Up;
		}
	}
	else if(m_portaDir == PortaDir::Down) {
		tmp = m_basePeriod-m_portaSpeed;
		if(tmp<m_portaTargetPeriod) {
			tmp = m_portaTargetPeriod;
			m_portaDir = PortaDir::Up;
		}
	}
	m_basePeriod = tmp;
	// TODO glissando
	m_portaPeriod = m_currentPeriod = m_basePeriod;
}
