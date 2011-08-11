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

#include "modchannel.h"

#include "modmodule.h"

#include "logger/logger.h"

#include <boost/assert.hpp>

namespace ppp {
namespace mod {

#ifdef MOD_USE_NTSC_FREQUENCY
static const uint32_t FrequencyBase = 7159090.5/2;
#else
static const uint32_t FrequencyBase = 7093789.2/2;
#endif

/**
 * @todo According to some Amiga assembler sources, periods
 * are clipped to a range of 0x71..0x358 (113..856 decimal respectively).
 */

ModChannel::ModChannel(ModModule* parent) : m_module(parent), m_currentCell(), m_volume(0),
	m_finetune(0), m_tremoloWaveform(0), m_vibratoWaveform(0), m_glissando(false),
	m_period(0), m_lastVibratoFx(0), m_lastPortaFx(0), m_sampleIndex(0), m_bresen(1,1)
{
	BOOST_ASSERT(parent!=nullptr);
}

ModChannel::~ModChannel() = default;

void ModChannel::update(const ppp::mod::ModCell::Ptr& cell, bool patDelay)
{
// 	if(isDisabled())
// 		return;
	if(m_module->tick() == 0) {
// 		m_noteChanged = false;
// 		m_currentFxStr = "      ";
		m_currentCell.clear();
		if(cell && !patDelay) {
			m_currentCell = *cell;
		}

		if(m_currentCell.period() != 0 || m_currentCell.sampleNumber() != 0 || m_currentCell.effect() != 0xff) {
// 			triggerNote();
			m_period = m_currentCell.period();
			m_sampleIndex = m_currentCell.sampleNumber();
			setActive(true);
		}
	} // endif(tick==0)
	switch(m_currentCell.effect()) {
		case 0x00:
			fxArpeggio(m_currentCell.effectValue());
			break;
		case 0x01:
			fxPortaUp(m_currentCell.effectValue());
			break;
		case 0x02:
			fxPortaDown(m_currentCell.effectValue());
			break;
		case 0x03:
			fxPorta(m_currentCell.effectValue());
			break;
		case 0x04:
			fxVibrato(m_currentCell.effectValue());
			break;
		case 0x05:
			fxPortaVolSlide(m_currentCell.effectValue());
			break;
		case 0x06:
			fxVibVolSlide(m_currentCell.effectValue());
			break;
		case 0x07:
			fxTremolo(m_currentCell.effectValue());
			break;
		case 0x08:
			fxSetFinePan(m_currentCell.effectValue());
			break;
		case 0x09:
			fxOffset(m_currentCell.effectValue());
			break;
		case 0x0a:
			fxVolSlide(m_currentCell.effectValue());
			break;
		case 0x0b:
			fxPosJmp(m_currentCell.effectValue());
			break;
		case 0x0c:
			fxSetVolume(m_currentCell.effectValue());
			break;
		case 0x0d:
			fxPatBreak(m_currentCell.effectValue());
			break;
		case 0x0e:
			switch(highNibble(m_currentCell.effectValue())) {
				case 0x00:
					break;
			}
			break;
		case 0x0f:
			fxSetSpeed(m_currentCell.effectValue());
			break;
	}
	updateStatus();
}

ModSample::Ptr ModChannel::currentSample() const
{
	return m_module->sampleAt(m_sampleIndex);
}

std::string ModChannel::cellString()
{
	return m_currentCell.trackerString();
}

std::string ModChannel::effectDescription() const
{
	// TODO
	return "      ";
}

std::string ModChannel::effectName() const
{
	// TODO
	return "   ";
}

std::string ModChannel::noteName()
{
	return "???"; // TODO
}

IArchive& ModChannel::serialize(IArchive* data)
{
    return GenChannel::serialize(data);
}

void ModChannel::simTick(size_t bufsize)
{
	if(!isActive() || !currentSample() || m_period == 0) {
		return setActive(false);
	}
	BOOST_ASSERT(m_module && m_module->frequency() != 0);
	BOOST_ASSERT(bufsize != 0);
	if(m_period == 0) {
		setActive(false);
		setPosition(0);
		return;
	}
	// TODO glissando
	int32_t pos = position() + (FrequencyBase / m_module->frequency() * bufsize / m_period);
	currentSample()->adjustPosition(pos);
	if(pos == GenSample::EndOfSample) {
		setActive(false);
	}
	setPosition(pos);
}

void ModChannel::mixTick(MixerFrameBuffer& mixBuffer)
{
	if(!isActive() || !currentSample() || m_period == 0) {
		return setActive(false);
	}
	BOOST_ASSERT(m_module && m_module->frequency() != 0);
	LOG_TEST_ERROR(mixBuffer->size() == 0);
	if(m_module->frequency() * mixBuffer->size() == 0) {
		setActive(false);
		return;
	}
	if(m_period == 0) {
		setActive(false);
		setPosition(0);
		return;
	}
	m_bresen.reset(m_module->frequency(), FrequencyBase / m_period);
	// TODO glissando
	ModSample::Ptr currSmp = currentSample();
	int32_t pos = position();
	if(pos == GenSample::EndOfSample) {
		setActive(false);
		return;
	}
	for(MixerSampleFrame& frame : *mixBuffer) {
		// TODO panning
		frame.left += (currSmp->leftSampleAt(pos)*m_volume)>>6;
		frame.right += (currSmp->rightSampleAt(pos)*m_volume)>>6;
		if(pos == GenSample::EndOfSample) {
			break;
		}
		m_bresen.next(pos);
	}
	if(pos != GenSample::EndOfSample) {
		currentSample()->adjustPosition(pos);
	}
	setPosition(pos);
	if(pos == GenSample::EndOfSample) {
		setActive(false);
	}
}

void ModChannel::fxSetSpeed(uint8_t fxByte)
{
	if(fxByte==0) {
		return;
	}
	else if(fxByte<=0x1f) {
		m_module->setSpeed(fxByte);
	}
	else {
		m_module->setTempo(fxByte);
	}
}

void ModChannel::efxNoteCut(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	if(fxByte == m_module->tick()) {
		setActive(false);
	}
}

void ModChannel::efxFineVolSlideDown(uint8_t fxByte)
{
	if(m_module->tick() != 0) {
		return;
	}
	fxByte = lowNibble(fxByte);
	m_volume = std::max<int>(0, m_volume-fxByte);
}

void ModChannel::efxFineVolSlideUp(uint8_t fxByte)
{
	if(m_module->tick() != 0) {
		return;
	}
	fxByte = lowNibble(fxByte);
	m_volume = std::min<int>(0x40, m_volume+fxByte);
}

void ModChannel::efxSetFinetune(uint8_t fxByte)
{
	m_finetune = lowNibble(fxByte);
}

void ModChannel::efxSetTremoloWaveform(uint8_t fxByte)
{
	m_tremoloWaveform = fxByte&0x7;
}

void ModChannel::efxSetVibWaveform(uint8_t fxByte)
{
	m_vibratoWaveform = fxByte&0x7;
}

void ModChannel::efxGlissando(uint8_t fxByte)
{
	m_glissando = lowNibble(fxByte)!=0;
}

void ModChannel::efxFineSlideDown(uint8_t fxByte)
{
	if(m_module->tick() != 0) {
		return;
	}
	// TODO clip period
	m_period += lowNibble(fxByte);
}

void ModChannel::efxFineSlideUp(uint8_t fxByte)
{
	if(m_module->tick() != 0) {
		return;
	}
	// TODO clip period
	m_period -= lowNibble(fxByte);
}

void ModChannel::fxOffset(uint8_t fxByte)
{
	setPosition(fxByte<<8);
}

void ModChannel::fxSetVolume(uint8_t fxByte)
{
	m_volume = std::min<uint8_t>(0x40, fxByte);
}

void ModChannel::fxVolSlide(uint8_t fxByte)
{
	// I assume that there is no slide on tick 0
	if(m_module->tick() == 0) {
		return;
	}
	if(highNibble(fxByte)!=0 && lowNibble(fxByte)==0) {
		// not valid
		return;
	}
	if(highNibble(fxByte)!=0) {
		m_volume = std::min(0x40, m_volume+highNibble(fxByte));
	}
	else if(lowNibble(fxByte)!=0) {
		m_volume = std::max<int>(0x0, m_volume-lowNibble(fxByte));
	}
}

void ModChannel::fxVibVolSlide(uint8_t fxByte)
{
	fxVolSlide(fxByte);
	fxVibrato(m_lastVibratoFx);
}

void ModChannel::fxPortaVolSlide(uint8_t fxByte)
{
	fxVolSlide(fxByte);
	fxPorta(m_lastPortaFx);
}

void ModChannel::fxPortaDown(uint8_t fxByte)
{
	// TODO maybe unconditionally assign?
	reuseIfZero(m_lastPortaFx, fxByte);
	// TODO clip
	m_period += fxByte;
}

void ModChannel::fxPortaUp(uint8_t fxByte)
{
	// TODO maybe unconditionally assign?
	reuseIfZero(m_lastPortaFx, fxByte);
	// TODO clip
	m_period -= fxByte;
}

void ModChannel::fxVibrato(uint8_t fxByte)
{
	// TODO
}

void ModChannel::fxPorta(uint8_t fxByte)
{
	// TODO
}

void ModChannel::updateStatus()
{
	// TODO
}

void ModChannel::fxArpeggio(uint8_t fxByte)
{
	// TODO
}

void ModChannel::fxPatBreak(uint8_t fxByte)
{
	// TODO
}

void ModChannel::fxPosJmp(uint8_t fxByte)
{
	// TODO
}

void ModChannel::fxSetFinePan(uint8_t fxByte)
{
	// TODO
}

void ModChannel::fxTremolo(uint8_t fxByte)
{
	// TODO
}


}
}
