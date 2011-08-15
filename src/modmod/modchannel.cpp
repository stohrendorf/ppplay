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

#include <cmath>
#include <boost/assert.hpp>

namespace ppp {
namespace mod {

#ifdef MOD_USE_NTSC_FREQUENCY
static const uint32_t FrequencyBase = 7159090.5/2;
#else
static const uint32_t FrequencyBase = 7093789.2/2;
#endif

static inline double finetuneMultiplicator(uint8_t finetune)
{
	return pow(2.0, -(finetune>7?finetune-16:finetune)/(12*8));
}

/**
 * @todo According to some Amiga assembler sources, periods
 * are clipped to a range of 0x71..0x358 (113..856 decimal respectively).
 */

ModChannel::ModChannel(ModModule* parent) : m_module(parent), m_currentCell(), m_volume(0),
	m_finetune(0), m_tremoloWaveform(0), m_vibratoWaveform(0), m_vibratoPhase(0), m_glissando(false),
	m_period(0), m_portaTarget(0), m_lastVibratoFx(0), m_lastPortaFx(0), m_sampleIndex(0), m_bresen(1,1),
	m_effectDescription("      ")
{
	BOOST_ASSERT(parent!=nullptr);
}

ModChannel::~ModChannel() = default;

void ModChannel::update(const ModCell::Ptr& cell, bool patDelay)
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

		if(m_currentCell.sampleNumber()!=0) {
			m_sampleIndex = m_currentCell.sampleNumber();
			if(currentSample()) {
				m_volume = currentSample()->volume();
				m_finetune = currentSample()->finetune();
			}
		}
		if(m_currentCell.period() != 0) {
			if(m_currentCell.effect()!=3 && m_currentCell.effect()!=5) {
				// 			triggerNote();
				m_period = m_currentCell.period();
				setPosition(0);
				setActive(true);
			}
			else {
				m_portaTarget = m_currentCell.period();
				if(m_period==0) {
					setPosition(0);
					m_period = m_portaTarget;
				}
				setActive(true);
			}
			if((m_vibratoWaveform&4) == 0) {
				// reset phase to 0 on a new note
				m_vibratoPhase = 0;
			}
		}
		setActive(m_period!=0 && currentSample());
	} // endif(tick==0)
	//if(!isActive()) {
		//return;
	//}
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
				case 0x01:
					efxFineSlideUp(m_currentCell.effectValue());
					break;
				case 0x02:
					efxFineSlideDown(m_currentCell.effectValue());
					break;
				case 0x03:
					efxGlissando(m_currentCell.effectValue());
					break;
				case 0x04:
					efxSetVibWaveform(m_currentCell.effectValue());
					break;
				case 0x05:
					efxSetFinetune(m_currentCell.effectValue());
					break;
				case 0x06:
					efxPatLoop(m_currentCell.effectValue());
					break;
				case 0x07:
					efxSetTremoloWaveform(m_currentCell.effectValue());
					break;
				case 0x08:
					efxSetPanning(m_currentCell.effectValue());
					break;
				case 0x0a:
					efxFineVolSlideUp(m_currentCell.effectValue());
					break;
				case 0x0b:
					efxFineVolSlideDown(m_currentCell.effectValue());
					break;
				case 0x0c:
					efxNoteCut(m_currentCell.effectValue());
					break;
				case 0x0d:
					efxNoteDelay(m_currentCell.effectValue());
					break;
				case 0x0e:
					efxPatDelay(m_currentCell.effectValue());
					break;
			}
			break;
		case 0x0f:
			fxSetSpeed(m_currentCell.effectValue());
			break;
	}
	m_period = clip<uint16_t>(m_period, 0x71, 0x358);
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
	return m_effectDescription;
}

std::string ModChannel::effectName() const
{
	if(m_currentCell.effect()==0 && m_currentCell.effectValue()==0) {
		return "   ";
	}
	return stringf("%X%.2X",m_currentCell.effect(),m_currentCell.effectValue());
}

std::string ModChannel::noteName()
{
	return "???"; // TODO noteName() -> reverse calc
}

IArchive& ModChannel::serialize(IArchive* data)
{
	GenChannel::serialize(data)
	.archive(&m_currentCell)
	% m_volume
	% m_finetune
	% m_tremoloWaveform
	% m_vibratoWaveform
	% m_vibratoPhase
	% m_glissando
	% m_period
	% m_portaTarget
	% m_lastVibratoFx
	% m_lastPortaFx
	% m_sampleIndex;
	return data->archive(&m_bresen);
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
	int32_t pos = position() + (FrequencyBase / m_module->frequency() * bufsize / (m_period*finetuneMultiplicator(m_finetune)+vibDelta()));
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
	m_bresen.reset(m_module->frequency(), FrequencyBase / (m_period*finetuneMultiplicator(m_finetune)+vibDelta()));
// 	setStatusString( statusString() + stringf(" %d +- %u",FrequencyBase/m_period, m_finetune) );
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
	m_effectDescription = "Tempo\x7f";
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
	m_effectDescription = "NCut \xd4";
	fxByte = lowNibble(fxByte);
	if(fxByte == m_module->tick()) {
		setActive(false);
	}
}

void ModChannel::efxFineVolSlideDown(uint8_t fxByte)
{
	m_effectDescription = "VSld \x19";
	if(m_module->tick() != 0) {
		return;
	}
	fxByte = lowNibble(fxByte);
	m_volume = std::max<int>(0, m_volume-fxByte);
}

void ModChannel::efxFineVolSlideUp(uint8_t fxByte)
{
	m_effectDescription = "VSld \x18";
	if(m_module->tick() != 0) {
		return;
	}
	fxByte = lowNibble(fxByte);
	m_volume = std::min<int>(0x40, m_volume+fxByte);
}

void ModChannel::efxSetFinetune(uint8_t fxByte)
{
	m_effectDescription = "FTune\xe6";
	m_finetune = lowNibble(fxByte);
}

void ModChannel::efxSetTremoloWaveform(uint8_t fxByte)
{
	m_effectDescription = "TWave\x9f";
	m_tremoloWaveform = fxByte&0x7;
}

void ModChannel::efxSetVibWaveform(uint8_t fxByte)
{
	m_effectDescription = "VWave\x9f";
	m_vibratoWaveform = fxByte&0x7;
}

void ModChannel::efxGlissando(uint8_t fxByte)
{
	m_effectDescription = "Gliss\xcd";
	m_glissando = lowNibble(fxByte)!=0;
}

void ModChannel::efxFineSlideDown(uint8_t fxByte)
{
	m_effectDescription = "Ptch \x1f";
	if(m_module->tick() != 0) {
		return;
	}
	// TODO clip period
	m_period += lowNibble(fxByte);
}

void ModChannel::efxFineSlideUp(uint8_t fxByte)
{
	m_effectDescription = "Ptch \x1e";
	if(m_module->tick() != 0) {
		return;
	}
	// TODO clip period
	m_period -= lowNibble(fxByte);
}

void ModChannel::fxOffset(uint8_t fxByte)
{
	m_effectDescription = "Offs \xaa";
	if(m_module->tick() != 0) {
		return;
	}
	setPosition(fxByte<<8);
}

void ModChannel::fxSetVolume(uint8_t fxByte)
{
	m_effectDescription = "StVol=";
	m_volume = std::min<uint8_t>(0x40, fxByte);
}

void ModChannel::fxVolSlide(uint8_t fxByte)
{
	// I assume that there is no slide on tick 0
	if(m_module->tick() == 0) {
		return;
	}
	if(highNibble(fxByte)!=0 && lowNibble(fxByte)!=0) {
		// not valid
		return;
	}
	if(highNibble(fxByte)!=0) {
		m_effectDescription = "VSld \x1e";
		m_volume = std::min(0x40, m_volume+highNibble(fxByte));
	}
	else if(lowNibble(fxByte)!=0) {
		m_effectDescription = "VSld \x1f";
		m_volume = std::max<int>(0x0, m_volume-lowNibble(fxByte));
	}
}

void ModChannel::fxVibVolSlide(uint8_t fxByte)
{
	fxVolSlide(fxByte);
	fxVibrato(m_lastVibratoFx);
	m_effectDescription = "VibVo\xf7";
}

void ModChannel::fxPortaVolSlide(uint8_t fxByte)
{
	fxVolSlide(fxByte);
	fxPorta(m_lastPortaFx);
	m_effectDescription = "PrtVo\x12";
}

void ModChannel::fxPortaDown(uint8_t fxByte)
{
	// TODO clip
	m_effectDescription = "Ptch \x1f";
	m_period += fxByte;
}

void ModChannel::fxPortaUp(uint8_t fxByte)
{
	m_effectDescription = "Ptch \x1e";
	// TODO clip
	m_period -= fxByte;
}

void ModChannel::fxVibrato(uint8_t fxByte)
{
	m_effectDescription = "Vibr \xf7";
	if(m_module->tick() == 0) {
		return;
	}
	reuseNibblesIfZero(m_lastVibratoFx, fxByte);
	m_vibratoPhase += highNibble(fxByte);
}

void ModChannel::fxPorta(uint8_t fxByte)
{
	m_effectDescription = "Porta\x12";
	if(m_module->tick() == 0) {
		return;
	}
	reuseIfZero(m_lastPortaFx, fxByte);
	if(m_portaTarget == 0) {
		m_portaTarget = m_period;
		return;
	}
	if(m_portaTarget == m_period) {
		return;
	}
	else if(m_portaTarget < m_period) {
		m_period = std::max<int>(m_portaTarget, m_period-fxByte);
	}
	else if(m_portaTarget > m_period) {
		m_period = std::min<int>(m_portaTarget, m_period+fxByte);
	}
}

void ModChannel::updateStatus()
{
	if(!isActive()) {
		setStatusString("");
		return;
	}
	std::string volStr = stringf("%3d%%", clip<int>(m_volume, 0, 0x40) * 100 / 0x40);
	setStatusString(stringf("%.2X: %s%s %s %s P:%s V:%s %s",
	                        m_sampleIndex,
	                        " ",
	                        //(m_noteChanged ? "*" : " "),
	                        noteName().c_str(),
	                        effectName().c_str(),
	                        m_effectDescription.c_str(),
	                        "-----",
	                        volStr.c_str(),
	                        currentSample() ? currentSample()->title().c_str() : ""
	                       ));
}

void ModChannel::fxArpeggio(uint8_t fxByte)
{
	if(fxByte == 0) {
		m_effectDescription = "      ";
		return;
	}
	else {
		m_effectDescription = "Arp  \xf0";
	}
	// TODO
}

void ModChannel::fxPatBreak(uint8_t fxByte)
{
	m_effectDescription = "PBrk \xf6";
	// TODO
}

void ModChannel::fxPosJmp(uint8_t fxByte)
{
	m_effectDescription = "JmOrd\x1a";
	// TODO
}

void ModChannel::fxSetFinePan(uint8_t fxByte)
{
	m_effectDescription = "StPan\x1d";
	// TODO
}

void ModChannel::fxTremolo(uint8_t fxByte)
{
	m_effectDescription = "Tremo\xec";
	// TODO
}

void ModChannel::efxPatLoop(uint8_t fxByte)
{
	m_effectDescription = "PLoop\xe8";
	// TODO
}

void ModChannel::efxNoteDelay(uint8_t fxByte)
{
	m_effectDescription = "Delay\xc2";
	// TODO should be handled in update()
}

void ModChannel::efxSetPanning(uint8_t fxByte)
{
	m_effectDescription = "StPan\x1d";
	// TODO
}

void ModChannel::efxPatDelay(uint8_t fxByte)
{
	// TODO
}

static const std::array<const int16_t, 32> WaveSine = {{
		0, 24, 49, 74, 97, 120, 141, 161,
		180, 197, 212, 224, 235, 244, 250, 253,
		255, 253, 250, 244, 235, 224, 212, 197,
		180, 161, 141, 120, 97, 74, 49, 24
}};

int16_t ModChannel::vibDelta()
{
	if(m_currentCell.effect() != 4 && m_currentCell.effect() != 6) {
		return 0;
	}
	m_vibratoPhase &= 0x3f;
	int16_t res = 0;
	switch(m_vibratoWaveform&3) {
		case 0:
			res = WaveSine.at( m_vibratoPhase&0x1f );
			break;
		case 1:
			res = 256-((m_vibratoPhase&0x1f)<<3);
			break;
		case 2:
			res = 256;
			break;
		case 3:
			res = rand()&0xff;
			break;
	}
	if(m_vibratoPhase>=0x20) {
		res = -res;
	}
	return (res*lowNibble(m_lastVibratoFx))>>7;
}

}
}
