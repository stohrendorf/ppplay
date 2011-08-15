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

#ifndef MODCHANNEL_H
#define MODCHANNEL_H

/**
 * @ingroup ModMod
 * @{
 */
#include "genmod/genchannel.h"
#include "genmod/breseninter.h"

#include "modcell.h"
#include "modsample.h"

namespace ppp {
namespace mod {

class ModModule;

class ModChannel : public GenChannel
{
	DISABLE_COPY(ModChannel)
private:
	ModModule* m_module;
	ModCell m_currentCell;
	uint8_t m_volume;
	uint8_t m_finetune;
	uint8_t m_tremoloWaveform;
	uint8_t m_vibratoWaveform;
	uint8_t m_vibratoPhase;
	bool m_glissando;
	uint16_t m_period;
	uint16_t m_portaTarget;
	uint8_t m_lastVibratoFx;
	uint8_t m_lastPortaFx;
	uint8_t m_sampleIndex;
	BresenInterpolation m_bresen;
	std::string m_effectDescription;
public:
	typedef std::shared_ptr<ModChannel> Ptr; //!< @brief Class pointer
	typedef std::vector<Ptr> Vector; //!< @brief Vector of class pointers
	explicit ModChannel(ModModule* parent);
	virtual ~ModChannel();
	virtual std::string noteName();
	virtual std::string effectName() const;
	virtual void mixTick(MixerFrameBuffer& mixBuffer);
	virtual void simTick(size_t bufsize);
	virtual void updateStatus();
	virtual std::string effectDescription() const;
	virtual std::string cellString();
	virtual IArchive& serialize(IArchive* data);
	void update(const ModCell::Ptr& cell, bool patDelay);
private:
	void fxArpeggio(uint8_t fxByte);
	void fxPortaUp(uint8_t fxByte);
	void fxPortaDown(uint8_t fxByte);
	void fxPorta(uint8_t fxByte);
	void fxVibrato(uint8_t fxByte);
	void fxPortaVolSlide(uint8_t fxByte);
	void fxVibVolSlide(uint8_t fxByte);
	void fxTremolo(uint8_t fxByte);
	void fxSetFinePan(uint8_t fxByte);
	void fxOffset(uint8_t fxByte);
	void fxVolSlide(uint8_t fxByte);
	void fxPosJmp(uint8_t fxByte);
	void fxSetVolume(uint8_t fxByte);
	void fxPatBreak(uint8_t fxByte);
	void efxFineSlideUp(uint8_t fxByte);
	void efxFineSlideDown(uint8_t fxByte);
	void efxGlissando(uint8_t fxByte);
	void efxSetVibWaveform(uint8_t fxByte);
	void efxSetFinetune(uint8_t fxByte);
	void efxPatLoop(uint8_t fxByte);
	void efxSetTremoloWaveform(uint8_t fxByte);
	void efxSetPanning(uint8_t fxByte);
	void efxRetrigger(uint8_t fxByte);
	void efxFineVolSlideUp(uint8_t fxByte);
	void efxFineVolSlideDown(uint8_t fxByte);
	void efxNoteCut(uint8_t fxByte);
	void efxNoteDelay(uint8_t fxByte);
	void efxPatDelay(uint8_t fxByte);
	void fxSetSpeed(uint8_t fxByte);
	ModSample::Ptr currentSample() const;
	int16_t vibDelta();
protected:
	static log4cxx::LoggerPtr logger();
};

}
}

/**
 * @}
 */

#endif
