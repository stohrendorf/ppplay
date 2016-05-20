#pragma once

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
 * @ingroup ModMod
 * @{
 */
#include <genmod/breseninter.h>
#include <genmod/genbase.h>
#include <genmod/channelstate.h>

#include <stream/iserializable.h>

#include <stuff/utils.h>

namespace light4cxx
{
class Logger;
}

namespace ppp
{
namespace mod
{
class ModModule;
class ModCell;
class ModSample;

class ModChannel : public ISerializable
{
    DISABLE_COPY(ModChannel)
private:
    ModModule* m_module;
    ModCell* m_currentCell;
    uint8_t m_volume;
    uint8_t m_physVolume;
    uint8_t m_finetune;
    uint8_t m_tremoloWaveform;
    uint8_t m_tremoloPhase;
    uint8_t m_vibratoWaveform;
    uint8_t m_vibratoPhase;
    bool m_glissando;
    uint16_t m_period;
    uint16_t m_physPeriod;
    uint16_t m_portaTarget;
    RememberByte<true> m_lastVibratoFx;
    RememberByte<true> m_lastTremoloFx;
    RememberByte<false> m_portaSpeed;
    RememberByte<false> m_lastOffsetFx;
    uint8_t m_sampleIndex;
    uint8_t m_lowMask;
    bool m_portaDirUp;
    BresenInterpolation m_bresen;
    uint8_t m_panning;

    ChannelState m_state;

    void setCellPeriod();
    void setTonePortaTarget();
public:
    explicit ModChannel(ModModule* parent, bool isLeftChan);
    ~ModChannel();
    AbstractArchive& serialize(AbstractArchive* data) override;
    void update(const ModCell& cell, bool patDelay);
    ChannelState status() const;
    void mixTick(MixerFrameBuffer* mixBuffer);
    void updateStatus();
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
    const ModSample* currentSample() const;
    void applyGlissando();
protected:
    /**
     * @brief Get the logger
     * @return Child logger with attached ".mod"
     */
    static light4cxx::Logger* logger();
};
}
}

/**
 * @}
 */
