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

#include <boost/assert.hpp>

namespace ppp {
namespace mod {

ModChannel::ModChannel(ModModule* parent) : m_parent(parent), m_currentCell()
{
	BOOST_ASSERT(parent!=nullptr);
}

ModChannel::~ModChannel() = default;

std::string ModChannel::cellString()
{
	return m_currentCell.trackerString();
}

std::string ModChannel::effectDescription() const
{
	// TODO
}

std::string ModChannel::effectName() const
{
	// TODO
}

void ModChannel::mixTick(MixerFrameBuffer& mixBuffer)
{
	// TODO
}

std::string ModChannel::noteName()
{
	return "???"; // TODO
}

IArchive& ModChannel::serialize(IArchive* data)
{
    // TODO return ppp::GenChannel::serialize(data);
}

void ModChannel::simTick(size_t bufsize)
{
	// TODO
}

void ModChannel::fxSetSpeed(uint8_t fxByte)
{
	if(fxByte==0) {
		return;
	}
	else if(fxByte<=0x1f) {
		m_parent->setSpeed(fxByte);
	}
	else {
		m_parent->setTempo(fxByte);
	}
}

void ModChannel::efxNoteCut(uint8_t fxByte)
{
	fxByte = lowNibble(fxByte);
	if(fxByte == m_parent->playbackInfo().tick) {
		setActive(false);
	}
}

void ModChannel::efxFineVolSlideDown(uint8_t fxByte)
{
	if(m_parent->playbackInfo().tick != 0) {
		return;
	}
	fxByte = lowNibble(fxByte);
	m_volume = std::max<int>(0, m_volume-fxByte);
}

void ModChannel::efxFineVolSlideUp(uint8_t fxByte)
{
	if(m_parent->playbackInfo().tick != 0) {
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
	if(m_parent->playbackInfo().tick != 0) {
		return;
	}
	// TODO clip period
	m_period += lowNibble(fxByte);
}

void ModChannel::efxFineSlideUp(uint8_t fxByte)
{
	if(m_parent->playbackInfo().tick != 0) {
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
	if(m_parent->playbackInfo().tick == 0) {
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

}
}
