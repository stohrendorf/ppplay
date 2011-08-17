/*
    PeePeePlayer - an old-fashioned module player
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
#include "xmmodule.h"

#include <cmath>

namespace ppp {
namespace xm {

XmChannel::XmChannel(ppp::xm::XmModule* module) :
	GenChannel(),
	m_baseVolume(0), m_currentVolume(0), m_realVolume(0), m_panning(0x80),
	m_realPanning(0x80), m_basePeriod(0), m_currentPeriod(0), m_autoVibDeltaPeriod(0),
	m_finetune(0), m_instrumentIndex(0), m_baseNote(0), m_realNote(0), m_currentCell(),
	m_volumeEnvelope(), m_panningEnvelope(), m_volScale(0), m_volScaleRate(0), m_keyOn(false),
	m_lastVolSlideFx(0), m_lastPortaUpFx(0), m_lastPortaDownFx(0), m_lastPanSlideFx(0),
	m_lastOffsetFx(0), m_lastFinePortaUpFx(0), m_lastFinePortaDownFx(0), m_lastFineVolUpFx(0),
	m_lastFineVolDownFx(0), m_lastXFinePortaUp(0), m_lastXFinePortaDown(0), m_lastGlobVolSlideFx(0),
	m_lastTremorFx(0), m_portaSpeed(0), m_portaTargetPeriod(0), m_vibratoSpeed(0), m_vibratoDepth(0),
	m_vibratoPhase(0), m_vibratoCtrl(0), m_glissandoCtrl(0), m_tremoloDepth(0),
	m_tremoloSpeed(0), m_tremoloPhase(0), m_tremoloCtrl(0), m_tremorCountdown(0),
	m_retriggerCounter(0), m_retriggerLength(0), m_retriggerVolumeType(0), m_patLoopCounter(0),
	m_patLoopRow(0), m_autoVibType(0), m_autoVibSweep(0), m_autoVibSweepRate(0),
	m_autoVibDepth(0), m_autoVibPhase(0), m_bres(1, 1), m_module(module), m_fxString() {
}

XmSample::Ptr XmChannel::currentSample() {
	XmInstrument::Ptr instr = currentInstrument();
	if(instr && m_baseNote > 0) {
		return instr->mapNoteSample(m_baseNote - 1);
	}
	return XmSample::Ptr();
}

XmInstrument::Ptr XmChannel::currentInstrument() {
	return m_module->getInstrument(m_instrumentIndex);
}


void XmChannel::fxPortaUp(uint8_t fxByte) {
	reuseIfZero(m_lastPortaUpFx, fxByte);
	int tmp = m_basePeriod - (fxByte << 2);
	if(tmp < 1)
		tmp = 1;
	m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::fxPortaDown(uint8_t fxByte) {
	reuseIfZero(m_lastPortaDownFx, fxByte);
	int tmp = m_basePeriod + (fxByte << 2);
	if(tmp > 0x7cff)
		tmp = 0x7cff;
	m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::fxVolSlide(uint8_t fxByte) {
	reuseIfZero(m_lastVolSlideFx, fxByte);
	int tmp = m_baseVolume;
	if(highNibble(fxByte) == 0)
		tmp -= lowNibble(fxByte);
	else
		tmp += highNibble(fxByte);
	m_baseVolume = m_currentVolume = clip<int>(tmp, 0, 0x40);
}

void XmChannel::fxPanSlide(uint8_t fxByte) {
	reuseIfZero(m_lastPanSlideFx, fxByte);
	int tmp = m_panning;
	if(highNibble(fxByte) == 0)
		tmp -= fxByte;
	else
		tmp += highNibble(fxByte);
	m_panning = clip(tmp, 0, 0xff);
}

void XmChannel::triggerNote() {
	uint8_t note = m_currentCell.note();
	if(note == KeyOffNote) {
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
		int tmp = currentSample()->relativeNote();
		tmp += m_baseNote;
		if(!inRange(tmp, 1, 119)) {
			//LOG_WARNING("OUT OF RANGE NOTE: rel=%d base=%d r=%d", currentSample()->relativeNote(), m_baseNote, tmp);
			setActive(false);
			return;
		}
		m_realNote = tmp - 1;
		if(m_currentCell.effect() == Effect::Extended && highNibble(m_currentCell.effectValue()) == EfxSetFinetune) {
			m_finetune = (lowNibble(m_currentCell.effectValue()) << 4) - 0x80;
		}
		else {
			m_finetune = currentSample()->finetune();
		}
		uint16_t newPer = m_module->noteToPeriod(m_realNote, m_finetune);
		if(newPer != 0) {
			m_basePeriod = newPer;
			m_currentPeriod = m_basePeriod;
		}
		if(m_currentCell.effect() == Effect::Offset) {
			fxOffset(m_currentCell.effectValue());
		}
		else if(!(m_currentCell.effect() == Effect::Porta || m_currentCell.effect() == Effect::PortaVolSlide || highNibble(m_currentCell.volume()) == VfxPorta)) {
			setPosition(0);
		}
		setActive(true);
	}
}

void XmChannel::doKeyOff() {
	m_keyOn = false;
	XmInstrument::Ptr instr = currentInstrument();
	if(!instr)
		return;

	if(!m_volumeEnvelope.enabled()) {
		m_baseVolume = m_currentVolume = 0;
	}
}

void XmChannel::doKeyOn() {
	if(!currentInstrument()) {
		doKeyOff();
		setActive(false);
		return;
	}
	if((m_vibratoCtrl & 4) == 0) {
		m_vibratoPhase = 0;
	}
	m_tremoloPhase = m_retriggerCounter = m_tremorCountdown = 0;
	m_keyOn = true;
	m_volumeEnvelope = currentInstrument()->volumeProcessor();
	m_panningEnvelope = currentInstrument()->panningProcessor();
	m_volScale = 0x8000;
	m_volScaleRate = currentInstrument()->fadeout();
	if(currentInstrument()->vibDepth() == 0) {
		return;
	}
	m_autoVibPhase = 0;
	if(currentInstrument()->vibSweep() == 0) {
		m_autoVibDepth = currentInstrument()->vibDepth() << 8;
		m_autoVibSweepRate = 0;
	}
	else {
		m_autoVibDepth = 0;
		m_autoVibSweepRate = m_autoVibDepth / currentInstrument()->vibSweep();
	}
	setActive(true);
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
static const std::array<int8_t, 256> g_AutoVibTable = {{
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
#endif

void XmChannel::update(const ppp::xm::XmCell::Ptr& cell) {
	if(m_module->tick() == 0 && !m_module->isRunningPatDelay()) {
		if(cell) {
			m_currentCell = *cell;
		}
		else {
			m_currentCell.clear();
		}

		if((m_vibratoCtrl & 4) == 0) {
			// check for vib reset
			if(m_currentCell.effect() != Effect::Vibrato && m_currentCell.effect() != Effect::VibratoVolSlide && highNibble(m_currentCell.volume()) != VfxVibrato) {
				m_vibratoPhase = 0;
			}
		}

		if(m_currentCell.instrument() != 0 && m_currentCell.instrument() < 0x80) {
			m_instrumentIndex = m_currentCell.instrument();
			applySampleDefaults(); // TODO check if it's right here
		}
		if(m_currentCell.effect() == Effect::Extended) {
			if(highNibble(m_currentCell.effectValue()) == EfxNoteDelay && lowNibble(m_currentCell.effectValue()) != 0) {
				return;
			}
			else if(highNibble(m_currentCell.effectValue()) == EfxRetrigger && lowNibble(m_currentCell.effectValue()) == 0) {
				if(m_currentCell.note() != KeyOffNote) {
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
		if(highNibble(m_currentCell.volume()) == VfxPorta) {
			if(lowNibble(m_currentCell.volume()) != 0) {
				m_portaSpeed = lowNibble(m_currentCell.volume()) << 4;
			}
			applySampleDefaults();
			if(m_currentCell.note()!=0 && m_currentCell.note()!=KeyOffNote) {
				doKeyOn();
			}
		}
		else if(m_currentCell.effect() == Effect::Porta) {
			if(m_currentCell.effectValue() != 0) {
				m_portaSpeed = m_currentCell.effectValue() << 2;
			}
			applySampleDefaults();
			if(m_currentCell.note()!=0 && m_currentCell.note()!=KeyOffNote) {
				doKeyOn();
			}
		}
		else if(m_currentCell.effect() == Effect::PortaVolSlide) {
			applySampleDefaults();
			if(m_currentCell.note()!=0 && m_currentCell.note()!=KeyOffNote) {
				doKeyOn();
			}
		}
		else if(m_currentCell.note() != 0) {
			if(m_currentCell.note() == KeyOffNote) {
				doKeyOff();
			}
			else {
				triggerNote();
				applySampleDefaults();
				doKeyOn();
			}
		}
		
		if(m_currentCell.note()!=0 && m_currentCell.note()!=KeyOffNote) {
			calculatePortaTarget(m_currentCell.note());
		}
		
		m_fxString = "      ";
		switch(highNibble(m_currentCell.volume())) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
				m_baseVolume = m_currentCell.volume() - 0x10;
				m_currentVolume = m_baseVolume;
				break;
			case VfxFineVolSlideDown:
				vfxFineVolSlideDown(m_currentCell.volume());
				m_fxString = "VSld \x19";
				break;
			case VfxFineVolSlideUp:
				vfxFineVolSlideUp(m_currentCell.volume());
				m_fxString = "VSld \x18";
				break;
			case VfxSetVibSpeed:
				vfxSetVibratoSpeed(m_currentCell.volume());
				m_fxString = "Vibr \xf7";
				break;
			case VfxSetPanning:
				vfxSetPan(m_currentCell.volume());
				m_fxString = "StPan\x1d";
				break;
			case VfxVolSlideDown:
				m_fxString = "VSld \x1f";
				break;
			case VfxVolSlideUp:
				m_fxString = "VSld \x1e";
				break;
			case VfxPanSlideLeft:
				m_fxString = "PSld \x1b";
				break;
			case VfxPanSlideRight:
				m_fxString = "PSld \x1a";
				break;
			case VfxPorta:
				m_fxString = "Porta\x12";
				break;
			case VfxVibrato:
				m_fxString = "Vibr \xf7";
				break;
		}
		switch(m_currentCell.effect()) {
			case Effect::None:
				break;
				// To get rid of the "enumeration value XY not handled in switch" warnings...
			case Effect::Arpeggio:
				m_fxString = "Arp  \xf0";
				break;
			case Effect::PortaUp:
				m_fxString = "Ptch\x1e\x1e";
				break;
			case Effect::PortaDown:
				m_fxString = "Ptch\x1f\x1f";
				break;
			case Effect::Porta:
				m_fxString = "Porta\x12";
				break;
			case Effect::Vibrato:
				m_fxString = "Vibr \xf7";
				break;
			case Effect::PortaVolSlide:
				m_fxString = "PrtVo\x12";
				break;
			case Effect::VibratoVolSlide:
				m_fxString = "VibVo\xf7";
				break;
			case Effect::Tremolo:
				m_fxString = "Tremo\xec";
				break;
			case Effect::VolSlide:
				if(highNibble(m_currentCell.effectValue()) == 0)
					m_fxString = "VSld \x1f";
				else
					m_fxString = "VSld \x1e";
				break;
			case Effect::GlobalVolSlide:
				if(highNibble(m_currentCell.effectValue()) == 0)
					m_fxString = "GVSld\x1f";
				else
					m_fxString = "GVSld\x1e";
				break;
			case Effect::KeyOff:
				m_fxString = "KOff \xd4";
				break;
			case Effect::PanSlide:
				if(highNibble(m_currentCell.effectValue()) == 0)
					m_fxString = "PSld \x1b";
				else
					m_fxString = "PSld \x1a";
				break;
			case Effect::Tremor:
				m_fxString = "Tremr\xec";
				break;
			case Effect::PosJump:
				m_fxString = "JmOrd\x1a";
				break;
			case Effect::PatBreak:
				// All the above effects are handled in tick 1+
				m_fxString = "PBrk \xf6";
				break;
			case Effect::Offset:
				// this effect is handled in triggerNote()
				m_fxString = "Offs \xaa";
				break;
			case Effect::SetPanning:
				fxSetPan(m_currentCell.effectValue());
				m_fxString = "StPan\x1d";
				break;
			case Effect::SetVolume:
				fxSetVolume(m_currentCell.effectValue());
				m_fxString = "StVol=";
				break;
			case Effect::Extended:
				fxExtended(m_currentCell.effectValue());
				break;
			case Effect::SetTempoBpm:
				fxSetTempoBpm(m_currentCell.effectValue());
				m_fxString = "Tempo\x7f";
				break;
			case Effect::SetGlobalVol:
				fxSetGlobalVolume(m_currentCell.effectValue());
				m_fxString = "GloVol";
				break;
			case Effect::ExtraFinePorta:
				fxExtraFinePorta(m_currentCell.effectValue());
				if(highNibble(m_currentCell.effectValue()) == 1) {
					m_fxString = "Ptch \x18";
				}
				else if(highNibble(m_currentCell.effectValue()) == 2) {
					m_fxString = "Ptch \x19";
				}
				else {
					m_fxString = "      ";
				}
				break;
			case Effect::Retrigger:
				fxRetrigger(m_currentCell.effectValue());
				m_fxString = "Retr \xec";
				break;
			case Effect::SetEnvPos:
				m_volumeEnvelope.setPosition(m_currentCell.effectValue());
				m_panningEnvelope.setPosition(m_currentCell.effectValue());
				m_fxString = "EnvP \x1d";
				break;
		}
	}
	else { // tick 1+
		if(m_currentCell.effect() == Effect::Extended) {
			if(highNibble(m_currentCell.effectValue()) == EfxNoteDelay) {
				if(lowNibble(m_currentCell.effectValue()) == m_module->tick()) {
					triggerNote();
					applySampleDefaults();
					doKeyOn();
				}
			}
		}
		switch(highNibble(m_currentCell.volume())) {
			case VfxVolSlideDown:
				vfxSlideDown(m_currentCell.volume());
				break;
			case VfxVolSlideUp:
				vfxSlideUp(m_currentCell.volume());
				break;
			case VfxPanSlideLeft:
				vfxPanSlideLeft(m_currentCell.volume());
				break;
			case VfxPanSlideRight:
				vfxPanSlideRight(m_currentCell.volume());
				break;
			case VfxPorta:
				fxPorta();
				break;
			case VfxVibrato:
				vfxVibrato(m_currentCell.volume());
				break;
		}
		switch(m_currentCell.effect()) {
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
				fxArpeggio(m_currentCell.effectValue());
				break;
			case Effect::PortaUp:
				fxPortaUp(m_currentCell.effectValue());
				break;
			case Effect::PortaDown:
				fxPortaDown(m_currentCell.effectValue());
				break;
			case Effect::VolSlide:
				fxVolSlide(m_currentCell.effectValue());
				break;
			case Effect::Extended:
				fxExtended(m_currentCell.effectValue());
				break;
			case Effect::Porta:
				fxPorta();
				break;
			case Effect::PortaVolSlide:
				fxPorta();
				fxVolSlide(m_currentCell.effectValue());
				break;
			case Effect::Vibrato:
				fxVibrato(m_currentCell.effectValue());
				break;
			case Effect::VibratoVolSlide:
				fxVibrato(m_currentCell.effectValue());
				fxVolSlide(m_currentCell.effectValue());
				break;
			case Effect::GlobalVolSlide:
				fxGlobalVolSlide(m_currentCell.effectValue());
				break;
			case Effect::PatBreak:
				m_module->doPatternBreak(highNibble(m_currentCell.effectValue()) * 10 + lowNibble(m_currentCell.effectValue()));
				break;
			case Effect::PosJump:
				m_module->doJumpPos(m_currentCell.effectValue());
				break;
			case Effect::Tremolo:
				fxTremolo(m_currentCell.effectValue());
				break;
			case Effect::KeyOff:
				if(m_module->tick() == m_currentCell.effectValue()) {
					doKeyOff();
				}
				break;
			case Effect::Tremor:
				fxTremor(m_currentCell.effectValue());
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
	m_volumeEnvelope.increasePosition(m_keyOn);
	m_realVolume = m_volumeEnvelope.realVolume(m_currentVolume, m_module->playbackInfo().globalVolume, m_volScale);
	m_panningEnvelope.increasePosition(m_keyOn);
	m_realPanning = m_panningEnvelope.realPanning(m_panning);

	if(currentInstrument()) {
		if(m_autoVibSweepRate != 0 && m_keyOn) {
			uint16_t tmp = m_autoVibSweepRate + m_autoVibDepth;
			if(tmp > currentInstrument()->vibDepth()) {
				m_autoVibSweepRate = 0;
				m_autoVibDeltaPeriod = 0;
				tmp = currentInstrument()->vibDepth();
			}
			m_autoVibDepth = tmp;
		}
		m_autoVibPhase += currentInstrument()->vibRate();
		int8_t value = 0;
		switch(m_vibratoCtrl & 3) {
			case 0:
				value = g_AutoVibTable.at(m_autoVibPhase);
				break;
			case 1:
				if((m_autoVibPhase & 0x80) == 0) {
					value = -0x40;
				}
				else {
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
		if(newPeriod > 0x7cff) {
			m_autoVibDeltaPeriod = 0;
		}
		else {
			m_autoVibDeltaPeriod = value * m_autoVibDepth >> 14;
		}
	}
	else {
		m_autoVibDeltaPeriod = 0;
	}

	updateStatus();

	if(m_currentVolume == 0 && m_realVolume == 0 && m_volScale == 0) {
		setActive(false);
	}
}

std::string XmChannel::cellString() {
	return m_currentCell.trackerString();
}

std::string XmChannel::effectDescription() const {
	return m_fxString;
}

std::string XmChannel::effectName() const {
	return m_currentCell.fxString();
}

std::string XmChannel::noteName() {
	if(m_baseNote == 0) {
		return "...";
	}
	else if(m_baseNote == KeyOffNote) {
		return "===";
	}
	else if(m_baseNote < KeyOffNote) {
		return stringf("%s%d", NoteNames.at((m_baseNote - 1) % 12), (m_baseNote - 1) / 12);
	}
	else {
		return "???";
	}
}

void XmChannel::mixTick(MixerFrameBuffer& mixBuffer) {
	if(!isActive())
		return;
	m_bres.reset(m_module->frequency(), m_module->periodToFrequency(m_currentPeriod + m_autoVibDeltaPeriod));
	XmSample::Ptr currSmp = currentSample();
	int32_t pos = position();
	uint8_t volLeft = 0x80;
	if(m_realPanning > 0x80) {
		volLeft = 0xff - m_realPanning;
	}
	uint8_t volRight = 0x80;
	if(m_realPanning < 0x80) {
		volRight = m_realPanning;
	}
	for(MixerSampleFrame& frame : *mixBuffer) {
		int16_t sampleVal = (currSmp->leftSampleAt(pos) * volLeft) >> 7;
		frame.left += (sampleVal * m_realVolume) >> 6;
		sampleVal = (currSmp->rightSampleAt(pos) * volRight) >> 7;
		frame.right += (sampleVal * m_realVolume) >> 6;
		if(pos == GenSample::EndOfSample) {
			break;
		}
		m_bres.next(pos);
	}
	if(pos != GenSample::EndOfSample)
		currentSample()->adjustPosition(pos);
	setPosition(pos);
	if(pos == GenSample::EndOfSample)
		setActive(false);
}

void XmChannel::simTick(size_t bufSize) {
	BOOST_ASSERT(m_module && m_module->initialized() && m_module->frequency() != 0);
	BOOST_ASSERT(bufSize != 0);
	if(!isActive())
		return;
	m_bres.reset(m_module->frequency(), m_module->periodToFrequency(m_currentPeriod + m_autoVibDeltaPeriod));
	XmSample::Ptr currSmp = currentSample();
	int32_t pos = position();
	pos += m_module->periodToFrequency(m_currentPeriod + m_autoVibDeltaPeriod) * bufSize / m_module->frequency();
	currSmp->adjustPosition(pos);
	if(pos != GenSample::EndOfSample)
		currentSample()->adjustPosition(pos);
	setPosition(pos);
	if(pos == GenSample::EndOfSample)
		setActive(false);
}

void XmChannel::updateStatus() {
	if(!isActive()) {
		setStatusString("");
		return;
	}
	std::string panStr;
	if(m_realPanning == 0x00)
		panStr = "Left ";
	else if(m_realPanning == 0x80)
		panStr = "Centr";
	else if(m_realPanning == 0xff)
		panStr = "Right";
	else
		panStr = stringf("%4d%%", (m_realPanning - 0x80) * 100 / 0x80);
	std::string volStr = stringf("%3d%%", clip<int>(m_realVolume , 0, 0x40) * 100 / 0x40);
	setStatusString(stringf("%.2X: %s%s %s %s P:%s V:%s %s",
	                        m_instrumentIndex,
	                        " ",
	                        //(m_noteChanged ? "*" : " "),
	                        noteName().c_str(),
	                        effectName().c_str(),
	                        effectDescription().c_str(),
	                        panStr.c_str(),
	                        volStr.c_str(),
	                        currentInstrument() ? currentInstrument()->title().c_str() : ""
	                       ));
	/*	setStatusString( stringf("vol=%.2u pan=%.2x vfx=%.2x fx=%.2x/%.2x [PE=%s] [VE=%s]",
								 m_realVolume, m_realPanning, m_currentCell.volume(), m_currentCell.effect(), m_currentCell.effectValue(), m_panningEnvelope.toString().c_str(), m_volumeEnvelope.toString().c_str() ));*/
}

void XmChannel::fxSetVolume(uint8_t fxByte) {
	if(fxByte > 0x40)
		return;
	m_currentVolume = m_baseVolume = fxByte;
}

void XmChannel::fxSetPan(uint8_t fxByte) {
	m_panning = fxByte;
}

void XmChannel::fxSetTempoBpm(uint8_t fxByte) {
	if(fxByte >= 1 && fxByte <= 0x1f) {
		m_module->setSpeed(fxByte);
	}
	else if(fxByte >= 0x20) {
		m_module->setTempo(fxByte);
	}
}

void XmChannel::vfxFineVolSlideDown(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	m_currentVolume = m_baseVolume = std::max<int>(0, m_baseVolume - fxByte);
}

void XmChannel::vfxFineVolSlideUp(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	m_currentVolume = m_baseVolume = std::min<int>(0x40, m_baseVolume + fxByte);
}

void XmChannel::vfxSetPan(uint8_t fxByte) {
	m_panning = lowNibble(fxByte) << 4;
}

void XmChannel::fxOffset(uint8_t fxByte) {
	reuseIfZero(m_lastOffsetFx, fxByte);
	setPosition(m_lastOffsetFx << 8);
}

void XmChannel::vfxSlideDown(uint8_t fxByte) {
	m_currentVolume = m_baseVolume = std::max<int>(m_baseVolume - lowNibble(fxByte), 0);
}

void XmChannel::vfxSlideUp(uint8_t fxByte) {
	m_currentVolume = m_baseVolume = std::min<int>(m_baseVolume + lowNibble(fxByte), 0x40);
}

void XmChannel::fxSetGlobalVolume(uint8_t fxByte) {
	m_module->setGlobalVolume(std::min<uint8_t>(fxByte, 0x40));
}

void XmChannel::efxFinePortaUp(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	reuseIfZero(m_lastFinePortaUpFx, fxByte);
	int tmp = m_basePeriod - (fxByte << 2);
	if(tmp < 1)
		tmp = 1;
	m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::efxFinePortaDown(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	reuseIfZero(m_lastFinePortaDownFx, fxByte);
	int tmp = m_basePeriod + (fxByte << 2);
	if(tmp > 0x7cff)
		tmp = 0x7cff;
	m_basePeriod = m_currentPeriod = tmp;
}

void XmChannel::efxFineVolDown(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	reuseIfZero(m_lastFineVolDownFx, fxByte);
	m_currentVolume = m_baseVolume = std::max(m_baseVolume - fxByte, 0);
}

void XmChannel::efxFineVolUp(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	reuseIfZero(m_lastFineVolUpFx, fxByte);
	m_currentVolume = m_baseVolume = std::min(m_baseVolume + fxByte, 0x40);
}

void XmChannel::fxExtraFinePorta(uint8_t fxByte) {
	if(highNibble(fxByte) == 1) {
		fxByte = lowNibble(fxByte);
		reuseIfZero(m_lastFinePortaUpFx, fxByte);
		m_basePeriod = m_currentPeriod = std::max(1, m_basePeriod - fxByte);
	}
	else if(highNibble(fxByte) == 2) {
		fxByte = lowNibble(fxByte);
		reuseIfZero(m_lastFinePortaDownFx, fxByte);
		m_basePeriod = m_currentPeriod = std::min(0x7cff, m_basePeriod + fxByte);
	}
}

void XmChannel::fxExtended(uint8_t fxByte) {
	switch(highNibble(fxByte)) {
		case EfxFinePortaUp:
			efxFinePortaUp(fxByte);
			m_fxString = "Ptch \x18";
			break;
		case EfxFinePortaDown:
			efxFinePortaDown(fxByte);
			m_fxString = "Ptch \x19";
			break;
		case EfxFineVolSlideUp:
			efxFineVolUp(fxByte);
			m_fxString = "VSld \x18";
			break;
		case EfxFineVolSlideDown:
			efxFineVolDown(fxByte);
			m_fxString = "VSld \x19";
			break;
		case EfxSetGlissCtrl:
			m_glissandoCtrl = lowNibble(fxByte) != 0;
			m_fxString = "Gliss\xcd";
			break;
		case EfxSetVibratoCtrl:
			m_vibratoCtrl = lowNibble(fxByte);
			m_fxString = "VWave\x9f";
			break;
		case EfxSetTremoloCtrl:
			m_tremoloCtrl = lowNibble(fxByte);
			m_fxString = "TWave\x9f";
			break;
		case EfxNoteCut:
			if(m_module->tick() == lowNibble(fxByte)) {
				m_baseVolume = m_currentVolume = 0;
			}
			m_fxString = "NCut \xd4";
			break;
		case EfxNoteDelay:
			if(m_module->tick() == lowNibble(fxByte)) {
				triggerNote();
				applySampleDefaults();
				doKeyOn();
				if(m_currentCell.volume() >= 0x10 && m_currentCell.volume() <= 0x5f) {
					m_currentVolume = m_baseVolume = m_currentCell.volume() - 0x10;
				}
				else if(highNibble(fxByte) == VfxSetPanning) {
					vfxSetPan(fxByte);
				}
			}
			m_fxString = "Delay\xc2";
			break;
		case EfxRetrigger:
			if(lowNibble(fxByte) != 0) {
				if(m_module->tick() % lowNibble(fxByte) == 0) {
					triggerNote();
					doKeyOn();
				}
			}
			m_fxString = "Retr \xec";
			break;
		case EfxPatLoop:
			if(m_module->tick() == 0) {
				efxPatLoop(fxByte);
			}
			m_fxString = "PLoop\xe8";
			break;
		case EfxPatDelay:
			if(m_module->tick() == 0) {
				m_module->doPatDelay( lowNibble(fxByte) );
			}
			break;
		case EfxSetPan:
			if(m_module->tick() == 0) {
				m_panning = 0xff*lowNibble(fxByte)/0x0f;
			}
			m_fxString = "StPan\x1d";
			break;
	}
}

void XmChannel::applySampleDefaults() {
	XmSample::Ptr smp = currentSample();
	if(m_currentCell.instrument() != 0 && smp) {
		m_baseVolume = m_currentVolume = smp->volume();
		m_panning = smp->panning();
	}
}

void XmChannel::vfxSetVibratoSpeed(uint8_t fxByte) {
	fxByte = lowNibble(fxByte) << 2;
	if(fxByte)
		m_vibratoSpeed = fxByte;
}

void XmChannel::vfxPanSlideLeft(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	m_panning = std::max<int>(0, m_panning - fxByte);
}

void XmChannel::vfxPanSlideRight(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	m_panning = std::min<int>(0xff, m_panning + fxByte);
}

void XmChannel::fxPorta() {
	int tmp = m_basePeriod;
	if(m_portaTargetPeriod < m_basePeriod) {
		tmp = m_basePeriod - m_portaSpeed;
		if(tmp < m_portaTargetPeriod) {
			tmp = m_portaTargetPeriod;
		}
	}
	else if(m_portaTargetPeriod > m_basePeriod) {
		tmp = m_basePeriod + m_portaSpeed;
		if(tmp > m_portaTargetPeriod) {
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

void XmChannel::calculatePortaTarget(uint8_t tarnote) {
	if(!currentSample())
		return;
	if(tarnote != 0 && tarnote != KeyOffNote) {
		uint16_t newPer = m_module->noteToPeriod(tarnote + currentSample()->relativeNote() - 1, currentSample()->finetune());
		if(newPer != 0) {
			m_portaTargetPeriod = newPer;
		}
	}
}

void XmChannel::fxArpeggio(uint8_t fxByte) {
	if(fxByte == 0) {
		return;
	}
	switch(m_module->tick() % 3) {
		case 0:
			m_currentPeriod = m_basePeriod;
			break;
		case 1:
			m_currentPeriod = m_module->glissando(m_basePeriod, m_finetune, highNibble(fxByte));
			break;
		case 2:
			m_currentPeriod = m_module->glissando(m_basePeriod, m_finetune, lowNibble(fxByte));
			break;
	}
}

void XmChannel::fxVibrato(uint8_t fxByte) {
	if(fxByte != 0) {
		if(lowNibble(fxByte) != 0) {
			m_vibratoDepth = lowNibble(fxByte);
		}
		if(highNibble(fxByte) >> 2 != 0) {
			m_vibratoSpeed = highNibble(fxByte) >> 2;
		}
	}
	doVibrato();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
static const std::array<uint8_t, 32> g_VibTable = {{
		0, 24, 49, 74, 97, 120, 141, 161, 180, 197, 212, 224, 235,
		244, 250, 253, 255, 253, 250, 244, 235, 224, 212, 197, 180, 161,
		141, 120, 97, 74, 49, 24
	}
};
#endif

void XmChannel::doVibrato() {
	uint8_t value = (m_vibratoPhase >> 2) & 0x1f;
	switch(m_vibratoCtrl & 3) {
		case 0:
			value = g_VibTable.at(value);
			break;
		case 1:
			value <<= 3;
			if((m_vibratoPhase & 0x80) == 0) {
				value = ~value;
			}
			break;
		case 2:
			value = 0xff;
	}
	uint16_t delta = (value * m_vibratoDepth) >> 5;
	if(m_vibratoPhase & 0x80) {
		m_currentPeriod = m_basePeriod - delta;
	}
	else {
		m_currentPeriod = m_basePeriod + delta;
	}
	m_vibratoPhase += m_vibratoSpeed;
}

void XmChannel::vfxVibrato(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	if(fxByte != 0) {
		m_vibratoDepth = fxByte;
	}
	doVibrato();
}

void XmChannel::fxGlobalVolSlide(uint8_t fxByte) {
	reuseIfZero(m_lastGlobVolSlideFx, fxByte);
	if(highNibble(fxByte) == 0) {
		fxByte = lowNibble(fxByte);
		m_module->setGlobalVolume(std::max(0, m_module->playbackInfo().globalVolume - fxByte));
	}
	else {
		fxByte = highNibble(fxByte);
		m_module->setGlobalVolume(std::min(0x40, m_module->playbackInfo().globalVolume + fxByte));
	}
}

void XmChannel::fxTremolo(uint8_t fxByte) {
	if(highNibble(fxByte) != 0) {
		m_tremoloSpeed = highNibble(fxByte) << 2;
	}
	if(lowNibble(fxByte) != 0) {
		m_tremoloDepth = lowNibble(fxByte);
	}
	uint8_t value = (m_tremoloPhase >> 2) & 0x1f;
	switch(m_tremoloCtrl & 3) {
		case 0:
			value = g_VibTable.at(value);
			break;
		case 1:
			value <<= 3;
			// This is _NOT_ a typo or c&p error!
			if((m_vibratoPhase & 0x80) == 0) {
				value = ~value;
			}
			break;
		case 2:
			value = 0xff;
	}
	uint16_t delta = (value * m_tremoloDepth) >> 6;
	if(m_tremoloPhase & 0x80) {
		m_currentVolume = m_baseVolume - delta;
	}
	else {
		m_currentVolume = m_baseVolume + delta;
	}
	m_tremoloPhase += m_tremoloSpeed;
}

void XmChannel::fxTremor(uint8_t fxByte) {
	reuseIfZero(m_lastTremorFx, fxByte);
	if((m_tremorCountdown & 0x7f) == 0) {
		if((m_tremorCountdown & 0x80) != 0) {
			m_tremorCountdown = lowNibble(fxByte);
		}
		else {
			m_tremorCountdown = 0x80 | highNibble(fxByte);
		}
	}
	else {
		m_tremorCountdown = (m_tremorCountdown & 0x80) | ((m_tremorCountdown & 0x7f) - 1);
	}
	if(m_tremorCountdown & 0x80) {
		m_currentVolume = m_baseVolume;
	}
	else {
		m_currentVolume = 0;
	}
}

void XmChannel::retriggerNote() {
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
			newVol = std::max(0, newVol - 1);
			break;
		case 2:
			newVol = std::max(0, newVol - 2);
			break;
		case 3:
			newVol = std::max(0, newVol - 4);
			break;
		case 4:
			newVol = std::max(0, newVol - 8);
			break;
		case 5:
			newVol = std::max(0, newVol - 16);
			break;
		case 6:
			newVol >>= 1;
			newVol += (newVol >> 2) + (newVol >> 3);
			break;
		case 7:
			newVol >>= 1;
			break;
		case 9:
			newVol = std::min(0x40, newVol + 1);
			break;
		case 10:
			newVol = std::min(0x40, newVol + 2);
			break;
		case 11:
			newVol = std::min(0x40, newVol + 4);
			break;
		case 12:
			newVol = std::min(0x40, newVol + 8);
			break;
		case 13:
			newVol = std::min(0x40, newVol + 16);
			break;
		case 14:
			newVol += newVol >> 1;
			if(newVol > 0x40) {
				newVol = 0x40;
			}
			break;
		case 15:
			newVol <<= 1;
			if(newVol > 0x40) {
				newVol = 0x40;
			}
			break;
	}
	m_currentVolume = m_baseVolume = newVol;
	if(m_currentCell.volume() >= 0x10 && m_currentCell.volume() <= 0x50) {
		m_currentVolume = m_baseVolume = m_currentCell.volume() - 0x10;
	}
	else if(highNibble(m_currentCell.volume()) == VfxSetPanning) {
		vfxSetPan(m_currentCell.volume());
	}
	triggerNote();
	doKeyOn();
}

void XmChannel::fxRetrigger(uint8_t fxByte) {
	if(lowNibble(fxByte) != 0) {
		m_retriggerLength = lowNibble(fxByte);
	}
	if(highNibble(fxByte) != 0) {
		m_retriggerVolumeType = highNibble(fxByte);
	}
	if(m_currentCell.note() != 0) {
		retriggerNote();
	}
}

void XmChannel::efxPatLoop(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	if(fxByte == 0) {
		m_patLoopRow = m_module->playbackInfo().row;
		return;
	}
	if(m_patLoopCounter == 0) {
		m_patLoopCounter = fxByte;
		m_module->doPatLoop(m_patLoopRow);
	}
	m_patLoopCounter--;
	if(m_patLoopCounter != 0) {
// 		m_patLoopCounter = fxByte;
		m_module->doPatLoop(m_patLoopRow);
	}
}

IArchive& XmChannel::serialize(IArchive* data) {
	GenChannel::serialize(data)
	% m_baseVolume
	% m_currentVolume
	% m_realVolume
	% m_panning
	% m_realPanning
	% m_basePeriod
	% m_currentPeriod
	% m_autoVibDeltaPeriod
	% m_finetune
	% m_instrumentIndex
	% m_baseNote
	% m_realNote;
	(*data)
	.archive(&m_currentCell)
	.archive(&m_volumeEnvelope)
	.archive(&m_panningEnvelope);
	*data
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
	% m_autoVibPhase;
	data->archive(&m_bres);
	return *data;
}

}
}

/**
 * @}
 */
