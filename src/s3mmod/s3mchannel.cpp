/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#include "genmod/genbase.h"
#include <genmod/standardfxdesc.h>
#include "s3mbase.h"
#include "s3mchannel.h"
#include "s3mpattern.h"
#include "s3mmodule.h"
#include "s3msample.h"
#include "s3mcell.h"

#include <cmath>

/**
 * @ingroup S3mMod
 * @{
 */

namespace ppp
{
namespace s3m
{
namespace
{
/**
 * @brief S3M sine wave lookup
 */
constexpr std::array<const int16_t, 64> S3mWaveSine = {
    {
        0, 24, 49, 74, 97, 120, 141, 161,
        180, 197, 212, 224, 235, 244, 250, 253,
        255, 253, 250, 244, 235, 224, 212, 197,
        180, 161, 141, 120, 97, 74, 49, 24,
        0, -24, -49, -74, -97, -120, -141, -161,
        -180, -197, -212, -224, -235, -244, -250, -253,
        -255, -253, -250, -244, -235, -224, -212, -197,
        -180, -161, -141, -120, -97, -74, -49, -24
    }
};

/**
 * @brief S3M ramp wave lookup
 */
constexpr std::array<const int16_t, 64> S3mWaveRamp = {
    {
        0, -0xF8, -0xF0, -0xE8, -0xE0, -0xD8, -0xD0, -0xC8,
        -0xC0, -0xB8, -0xB0, -0xA8, -0xA0, -0x98, -0x90, -0x88,
        -0x80, -0x78, -0x70, -0x68, -0x60, -0x58, -0x50, -0x48, -0x40,
        -0x38, -0x30, -0x28, -0x20, -0x18, -0x10, -0x8, 0x0, 0x8, 0x10, 0x18,
        0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70,
        0x78, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0,
        0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8
    }
};

/**
 * @brief S3M square wave lookup
 */
constexpr std::array<const int16_t, 64> S3mWaveSquare = {
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    }
};

/**
 * @brief S3M finetunes lookup
 */
constexpr std::array<const uint16_t, 16> S3mFinetunes = {
    {
        8363, 8413, 8463, 8529, 8581, 8651, 8723, 8757,
        7895, 7941, 7985, 8046, 8107, 8169, 8232, 8280
    }
};

/**
 * @brief Note periodic table for frequency calculations
 */
constexpr uint16_t Periods[12] = {1712 << 4, 1616 << 4, 1524 << 4, 1440 << 4, 1356 << 4, 1280 << 4, 1208 << 4, 1140 << 4, 1076 << 4, 1016 << 4, 960 << 4,
                                  907 << 4};

/**
 * @brief Calculate the period for a given note, octave and base frequency
 * @param[in] note Note value (without octave)
 * @param[in] oct Note octave
 * @param[in] c4spd Base frequency of the sample
 * @param[in] finetune Optional finetune
 * @return S3M Period for note @a note, base frequency @a c4spd and finetune @a finetune
 */
inline constexpr uint16_t st3PeriodEx(uint8_t note, uint8_t oct, uint16_t c4spd, uint16_t finetune = 8363)
{
    return (Periods[note] >> oct) * finetune / (c4spd == 0 ? 8363 : c4spd);
}

/**
 * @brief Calculate the period for a given note and base frequency
 * @param[in] note Note value (including octave)
 * @param[in] c4spd Base frequency of the sample
 * @param[in] finetune Optional finetune
 * @return S3M Period for note @a note, base frequency @a c4spd and finetune @a finetune
 */
inline constexpr uint16_t st3Period(uint8_t note, uint16_t c4spd, uint16_t finetune = 8363)
{
    return st3PeriodEx(note & 0x0f, note >> 4, c4spd, finetune);
}

/**
 * @brief Reverse calculate a note from a given period and C4 frequency
 * @param[in] per Note period
 * @param[in] c4spd Base frequency of the sample
 * @param[in] finetune Optional finetune
 * @return Note offset (12*octave+note)
 */
inline uint8_t periodToNoteOffset(uint16_t per, uint16_t c4spd, uint16_t finetune = 8363)
{
    return static_cast<uint8_t>(std::round(-12 * std::log2(static_cast<float>(per) * c4spd / (finetune * Periods[0]))));
}

/**
 * @brief Add/subtract semitones to/from a note
 * @param[in] note Base note
 * @param[in] delta Delta value
 * @return New note
 */
inline uint8_t deltaNote(uint8_t note, int8_t delta)
{
    uint16_t x = (note >> 4) * 12 + (note & 0x0f) + delta;
    return ((x / 12) << 4) | (x % 12);
}

/**
 * @brief Clip a period if necessary
 * @param[in] amiga Set to @c true to use amiga limits
 * @param[in] period The period to clip
 * @return Clipped period
 */
uint16_t clipPeriod(bool amiga, uint16_t period)
{
    if( amiga )
    {
        return clip<uint16_t>(period, 0x15c, 0xd60);
    }
    else
    {
        return clip<uint16_t>(period, 0x40, 0x7fff);
    }
}
} // anonymous namespace

void S3mChannel::setSampleIndex(int32_t idx)
{
    m_state.instrument = idx;
    if( !currentSample() || currentSample()->frequency() == 0 )
    {
        m_state.active = false;
    }
}

S3mChannel::S3mChannel(S3mModule* const module)
    :
    m_note(0xff), m_lastFxByte(0), m_lastVibratoData(0), m_lastPortaSpeed(0), m_tremorVolume(0), m_currentVolume(0), m_realVolume(0), m_baseVolume(0)
    , m_tremorMute(false), m_retrigCount(0), m_zeroVolCounter(3), m_module(module), m_basePeriod(0), m_realPeriod(0), m_portaTargetPeriod(0), m_vibratoPhase(0)
    , m_vibratoWaveform(0), m_tremoloPhase(0), m_tremoloWaveform(0), m_countdown(0), m_tremorCounter(0), m_c2spd(0), m_glissando(false), m_currentCell(
    new S3mCell()), m_bresen(1, 1), m_panning(0x20), m_isEnabled(false), m_state()
{
    m_state.instrument = 101;
}

S3mChannel::~S3mChannel()
{
    delete m_currentCell;
}

ChannelState S3mChannel::status() const
{
    return m_state;
}

const S3mSample* S3mChannel::currentSample() const
{
    if( !between<int>(m_state.instrument, 0, m_module->numSamples() - 1) )
    {
        return nullptr;
    }
    return m_module->sampleAt(m_state.instrument);
}

void S3mChannel::update(const S3mCell& cell, bool patDelay, bool estimateOnly)
{
    if( !m_isEnabled )
    {
        return;
    }
    if( m_module->state().tick == 0 )
    {
        if( estimateOnly )
        {
            switch( static_cast<S3mEffects>(cell.effect()) )
            {
                case s3mFxSpeed:
                    fxSpeed(cell.effectValue());
                    break;
                case s3mFxTempo:
                    fxTempo(cell.effectValue());
                    break;
                default:
                    // silence ;-)
                    break;
            }
            return;
        }
        m_state.noteTriggered = false;
        m_state.fxDesc = fxdesc::NullFx;
        m_currentCell->clear();
        if( !patDelay )
        {
            *m_currentCell = cell;
        }

        m_state.cell = m_currentCell->trackerString();
        m_state.fxParam = m_currentCell->effectValue();
        if( m_currentCell->effect() == s3mEmptyCommand )
        {
            m_state.fx = 0;
        }
        else
        {
            uint8_t fx = m_currentCell->effect();
            if( between<int>(fx, 1, 27) )
            {
                m_state.fx = char(fx - 1 + 'A');
            }
            else
            {
                logger()->warn(L4CXX_LOCATION, "Effect out of range: %#x", int(fx));
                m_state.fx = '?';
            }
        }

        if( m_currentCell->note() != s3mEmptyNote || m_currentCell->instrument() != s3mEmptyInstr || m_currentCell->volume() != s3mEmptyVolume ||
            m_currentCell->effect() != s3mEmptyCommand )
        {
            triggerNote();
        }

        if( m_currentCell->effect() != 0 )
        {
            m_zeroVolCounter = 3;
        }
        else if( m_module->hasZeroVolOpt() )
        {
            if( m_currentVolume == 0 && m_currentCell->volume() == 0 && m_currentCell->instrument() == s3mEmptyInstr && m_currentCell->note() == s3mEmptyNote )
            {
                m_zeroVolCounter--;
                if( m_zeroVolCounter == 0 )
                {
                    m_state.active = false;
                    return;
                }
            }
            else
            {
                m_zeroVolCounter = 3;
            }
        }

        m_lastFxByte.noNibbles(m_currentCell->effectValue());
        if( m_currentCell->effect() == s3mEmptyCommand )
        {
            if( m_module->hasAmigaLimits() )
            {
                m_countdown = 0;
            }
            if( m_basePeriod != m_realPeriod )
            {
                m_realPeriod = m_basePeriod;
                recalcFrequency();
            }
        }
        else if( m_currentCell->effect() == s3mFxVolSlide )
        {
            m_countdown = 0;
            if( m_basePeriod != m_realPeriod )
            {
                m_realPeriod = m_basePeriod;
                recalcFrequency();
            }
        }
        else if( m_currentCell->effect() != s3mFxTremor )
        {
            m_tremorCounter = 0;
            m_tremorMute = false;
        }
        else
        {
            // better readable than an if-expression
            switch( m_currentCell->effect() )
            {
                case s3mFxVibrato:
                case s3mFxFineVibrato:
                case s3mFxVibVolSlide:
                case s3mFxTremolo:
                    m_vibratoPhase |= 0x40;
            }
        }

        switch( static_cast<S3mEffects>(m_currentCell->effect()) )
        {
            case s3mFxSpeed:
                fxSpeed(m_currentCell->effectValue());
                break;
            case s3mFxVolSlide:
                fxVolSlide(m_currentCell->effectValue());
                break;
            case s3mFxPitchDown:
                fxPitchSlideDown(m_currentCell->effectValue());
                break;
            case s3mFxPitchUp:
                fxPitchSlideUp(m_currentCell->effectValue());
                break;
            case s3mFxTremor:
                fxTremor(m_currentCell->effectValue());
                break;
            case s3mFxArpeggio:
                fxArpeggio(m_currentCell->effectValue());
                break;
            case s3mFxRetrig:
                fxRetrigger(m_currentCell->effectValue());
                break;
            case s3mFxSpecial:
                fxSpecial(m_currentCell->effectValue());
                break;
            case s3mFxTempo:
                fxTempo(m_currentCell->effectValue());
                break;
            case s3mFxJumpOrder:
                m_state.fxDesc = fxdesc::JumpOrder;
                break;
            case s3mFxBreakPat:
                m_state.fxDesc = fxdesc::PatternBreak;
                break;
            case s3mFxSetPanning:
                // this is a non-standard effect, so we ignore the fx byte
                m_state.fxDesc = fxdesc::SetPanPos;
                if( m_currentCell->effectValue() <= 0x80 )
                {
                    m_panning = m_currentCell->effectValue() >> 1;
                }
                else if( m_currentCell->effectValue() == 0xa4 )
                {
                    m_panning = m_currentCell->effectValue();
                }
                break;
            case s3mFxVibVolSlide:
                m_state.fxDesc = fxdesc::VibVolSlide;
                break;
            case s3mFxPortaVolSlide:
                m_state.fxDesc = fxdesc::PortaVolSlide;
                break;
            case s3mFxPorta:
                m_state.fxDesc = fxdesc::Porta;
                break;
            case s3mFxVibrato:
                m_state.fxDesc = fxdesc::Vibrato;
                break;
            case s3mFxFineVibrato:
                m_state.fxDesc = fxdesc::FineVibrato;
                break;
            case s3mFxTremolo:
                m_state.fxDesc = fxdesc::Tremolo;
                break;
            case s3mFxGlobalVol:
                m_state.fxDesc = fxdesc::GlobalVolume;
                break;
            case s3mFxOffset:
                // handled...
                break;
        }
    } // endif(tick==0)
    else if( m_currentCell->effect() != 0 && !estimateOnly )
    { // if(tick!=0)
        switch( static_cast<S3mEffects>(m_currentCell->effect()) )
        {
            case s3mFxVolSlide:
                fxVolSlide(m_currentCell->effectValue());
                break;
            case s3mFxPitchDown:
                fxPitchSlideDown(m_currentCell->effectValue());
                break;
            case s3mFxPitchUp:
                fxPitchSlideUp(m_currentCell->effectValue());
                break;
            case s3mFxPorta:
                fxPorta(m_currentCell->effectValue(), false);
                break;
            case s3mFxVibrato:
                fxVibrato(m_currentCell->effectValue(), false, false);
                break;
            case s3mFxTremor:
                fxTremor(m_currentCell->effectValue());
                break;
            case s3mFxArpeggio:
                fxArpeggio(m_currentCell->effectValue());
                break;
            case s3mFxVibVolSlide:
                fxVolSlide(m_currentCell->effectValue());
                fxVibrato(m_lastVibratoData, false, true);
                m_state.fxDesc = fxdesc::VibVolSlide;
                break;
            case s3mFxPortaVolSlide:
                fxVolSlide(m_currentCell->effectValue());
                fxPorta(m_lastPortaSpeed, true);
                m_state.fxDesc = fxdesc::PortaVolSlide;
                break;
            case s3mFxRetrig:
                fxRetrigger(m_currentCell->effectValue());
                break;
            case s3mFxTremolo:
                fxTremolo(m_currentCell->effectValue());
                break;
            case s3mFxSpecial:
                fxSpecial(m_currentCell->effectValue());
                break;
            case s3mFxFineVibrato:
                fxVibrato(m_currentCell->effectValue(), true, false);
                break;
            case s3mFxGlobalVol:
                fxGlobalVolume(m_currentCell->effectValue());
                break;
            case s3mFxOffset:
            case s3mFxJumpOrder:
            case s3mFxBreakPat:
            case s3mFxTempo:
            case s3mFxSetPanning:
            case s3mFxSpeed:
                // already handled...
                break;
        }
    }
    updateStatus();
}

void S3mChannel::mixTick(const MixerFrameBufferPtr& mixBuffer)
{
    if( !mixBuffer )
    {
        return;
    }
    if( !m_isEnabled )
    {
        return;
    }
    if( !m_state.active || !currentSample() || m_basePeriod == 0 )
    {
        m_state.active = false;
        return;
    }
    if( m_module->state().tick == 0 && m_zeroVolCounter != -1 && m_state.active )
    {
        if( m_currentVolume == 0 )
        {
            m_zeroVolCounter++;
        }
        else
        {
            m_zeroVolCounter = 0;
        }
        if( m_zeroVolCounter == 3 )
        {
            m_zeroVolCounter = 0;
            m_state.active = false;
            m_note = s3mEmptyNote;
            return;
        }
    }
    if( !m_state.active )
    {
        return;
    }
    if( m_module->frequency() * mixBuffer->size() == 0 )
    {
        m_state.active = false;
        return;
    }
    m_bresen.reset(m_module->frequency(), 8363 * 1712 / m_realPeriod);
    recalcVolume();
    uint16_t currVol = m_realVolume;
    const S3mSample* currSmp = currentSample();
    int volL = 0x20;
    int volR = 0x20;
    if( m_panning > 0x20 && m_panning != 0xa4 )
    {
        volL = 0x40 - m_panning;
    }
    if( m_panning < 0x20 )
    {
        volR = m_panning;
    }
    else if( m_panning == 0xa4 )
    {
        volR *= -1;
    }
    volL *= currVol;
    volR *= currVol;
    m_state.active = mix(*currSmp, m_module->interpolation(), m_bresen, *mixBuffer, volL, volR, 11) != 0;
}

void S3mChannel::updateStatus()
{
    if( m_panning == 0xa4 )
    {
        m_state.panning = ChannelState::Surround;
    }
    else
    {
        m_state.panning = (m_panning - 0x20) * 100 / 0x40;
    }

    m_state.volume = clip<int>(m_currentVolume, 0, 0x3f) * 100 / 0x3f;

    if( m_note == s3mKeyOffNote || m_currentCell->note() == s3mKeyOffNote )
    {
        m_state.note = ChannelState::NoteCut;
    }
    else if( !currentSample() )
    {
        m_state.note = ChannelState::NoNote;
        m_state.instrumentName.clear();
    }
    else
    {
        const S3mSample* smp = currentSample();
        m_state.note = periodToNoteOffset(m_realPeriod, smp->frequency());
        m_state.instrumentName = smp->title();
    }
}

AbstractArchive& S3mChannel::serialize(AbstractArchive* data)
{
    *data
    % m_note
    % m_lastFxByte
    % m_lastVibratoData
    % m_lastPortaSpeed
    % m_tremorVolume
    % m_currentVolume
    % m_realVolume
    % m_baseVolume
    % m_tremorMute
    % m_retrigCount
    % m_zeroVolCounter
    % m_basePeriod
    % m_realPeriod
    % m_portaTargetPeriod
    % m_vibratoPhase
    % m_vibratoWaveform
    % m_tremoloPhase
    % m_tremoloWaveform
    % m_countdown
    % m_tremorCounter
    % m_c2spd
    % m_glissando
    % m_bresen
    % m_currentCell
    % m_isEnabled
    % m_panning
    % m_state;
    return *data;
}

void S3mChannel::fxPitchSlideUp(uint8_t fxByte)
{
    if( m_basePeriod == 0 )
    {
        return;
    }
    m_lastFxByte.noNibbles(fxByte);
    uint16_t delta = 0;
    if( m_module->state().tick == 0 )
    {
        if( m_lastFxByte <= 0xe0 )
        {
            m_state.fxDesc = fxdesc::FastPitchSlideUp;
            return;
        }
        if( m_lastFxByte <= 0xf0 )
        {
            m_state.fxDesc = fxdesc::SlowPitchSlideUp;
            delta = m_lastFxByte.lo();
        }
        else
        {
            m_state.fxDesc = fxdesc::PitchSlideUp;
            delta = m_lastFxByte.lo() << 2;
        }
    }
    else
    {
        if( m_lastFxByte >= 0xe0 )
        {
            return;
        }
        m_state.fxDesc = fxdesc::FastPitchSlideUp;
        delta = m_lastFxByte << 2;
    }
    m_basePeriod = m_realPeriod = std::max(0, m_basePeriod - delta);
    recalcFrequency();
}

void S3mChannel::fxPitchSlideDown(uint8_t fxByte)
{
    if( m_basePeriod == 0 )
    {
        return;
    }
    m_lastFxByte.noNibbles(fxByte);
    uint16_t delta = 0;
    if( m_module->state().tick == 0 )
    {
        if( m_lastFxByte <= 0xe0 )
        {
            m_state.fxDesc = fxdesc::FastPitchSlideDown;
            return;
        }
        if( m_lastFxByte <= 0xf0 )
        {
            m_state.fxDesc = fxdesc::SlowPitchSlideDown;
            delta = m_lastFxByte.lo();
        }
        else
        {
            m_state.fxDesc = fxdesc::PitchSlideDown;
            delta = m_lastFxByte.lo() << 2;
        }
    }
    else
    {
        if( m_lastFxByte >= 0xe0 )
        {
            return;
        }
        m_state.fxDesc = fxdesc::FastPitchSlideDown;
        delta = m_lastFxByte << 2;
    }
    m_basePeriod = m_realPeriod = std::min(0x7fff, m_basePeriod + delta);
    recalcFrequency();
}

void S3mChannel::fxVolSlide(uint8_t fxByte)
{
    m_lastFxByte.noNibbles(fxByte);
    if( m_lastFxByte.lo() == 0x0f )
    {
        if( m_lastFxByte.hi() == 0 )
        {
            m_state.fxDesc = fxdesc::VolSlideDown;
            m_currentVolume = std::max(0, m_currentVolume - m_lastFxByte.lo());
        }
        else
        {
            m_state.fxDesc = fxdesc::SlowVolSlideUp;
            if( m_module->state().tick == 0 )
            {
                m_currentVolume = std::min(63, m_currentVolume + m_lastFxByte.hi());
            }
        }
    }
    else if( m_lastFxByte.hi() == 0x0f )
    {
        if( m_lastFxByte.lo() == 0 )
        {
            m_state.fxDesc = fxdesc::VolSlideUp;
            m_currentVolume = std::min(63, m_currentVolume + m_lastFxByte.hi());
        }
        else
        {
            m_state.fxDesc = fxdesc::SlowVolSlideDown;
            if( m_module->state().tick == 0 )
            {
                m_currentVolume = std::max(0, m_currentVolume - m_lastFxByte.lo());
            }
        }
    }
    else
    {
        if( m_lastFxByte.lo() == 0 )
        {
            m_state.fxDesc = fxdesc::VolSlideUp;
            if( m_module->hasFastVolSlides() || m_module->state().tick != 0 )
            {
                m_currentVolume = std::min(63, m_currentVolume + m_lastFxByte.hi());
            }
        }
        else
        {
            m_state.fxDesc = fxdesc::VolSlideDown;
            if( m_module->hasFastVolSlides() || m_module->state().tick != 0 )
            {
                m_currentVolume = std::max(0, m_currentVolume - m_lastFxByte.lo());
            }
        }
    }
    recalcVolume();
}

void S3mChannel::recalcFrequency()
{
    if( m_module->hasAmigaLimits() )
    {
        m_basePeriod = clipPeriod(true, m_basePeriod);
    }
    uint16_t per = m_realPeriod;
    if( per == 0 )
    {
        per = m_basePeriod;
    }
    m_realPeriod = clipPeriod(m_module->hasAmigaLimits(), per);
}

void S3mChannel::recalcVolume()
{
    m_realVolume = (static_cast<int>(m_module->state().globalVolume) * m_currentVolume) >> 6;
}

void S3mChannel::fxPorta(uint8_t fxByte, bool noReuse)
{
    m_state.fxDesc = fxdesc::Porta;
    if( m_module->state().tick == 0 )
    {
        return;
    }
    if( m_basePeriod == 0 )
    {
        if( m_portaTargetPeriod == 0 )
        {
            return;
        }
        m_realPeriod = m_basePeriod = m_portaTargetPeriod;
        fxByte = m_portaTargetPeriod & 0xff;
    }
    if( !noReuse )
    {
        m_lastPortaSpeed = fxByte;
        fxByte = m_lastPortaSpeed;
    }
    if( m_basePeriod == m_portaTargetPeriod )
    {
        return;
    }
    if( m_basePeriod > m_portaTargetPeriod )
    {
        int tmp = m_basePeriod - (fxByte << 2);
        if( tmp < m_portaTargetPeriod )
        {
            tmp = m_portaTargetPeriod;
        }
        if( m_glissando )
        {
            tmp = glissando(tmp);
        }
        m_realPeriod = m_basePeriod = tmp;
        recalcFrequency();
    }
    else
    {
        int tmp = m_basePeriod + (fxByte << 2);
        if( tmp > m_portaTargetPeriod )
        {
            tmp = m_portaTargetPeriod;
        }
        if( m_glissando )
        {
            tmp = glissando(tmp);
        }
        m_realPeriod = m_basePeriod = tmp;
        recalcFrequency();
    }
}

/**
 * @brief Look up a wave value
 * @param[in] waveform Waveform selector
 * @param[in] phase Wave phase
 * @return Lookup value
 */
static int16_t waveValue(uint8_t waveform, uint8_t phase)
{
    switch( waveform & 7 )
    {
        case 1:
            if( phase & 0x40 )
            {
                return S3mWaveRamp[0];
            }
        case 5:
            return S3mWaveRamp[phase & 0x3f];
        case 2:
            if( phase & 0x40 )
            {
                return S3mWaveSquare[0];
            }
        case 6:
            return S3mWaveSquare[phase & 0x3f];
        case 0:
        case 3:
            if( phase & 0x40 )
            {
                return S3mWaveSine[0];
            }
        case 4:
        case 7:
            return S3mWaveSine[phase & 0x3f];
        default:
            return 0;
    }
}

void S3mChannel::fxVibrato(uint8_t fxByte, bool fine, bool noReuse)
{
    if( !fine )
    {
        m_state.fxDesc = fxdesc::Vibrato;
    }
    else
    {
        m_state.fxDesc = fxdesc::FineVibrato;
    }
    if( !noReuse )
    {
        m_lastVibratoData = fxByte;
    }
    if( m_basePeriod == 0 )
    {
        return;
    }
    int val = waveValue(m_vibratoWaveform, m_vibratoPhase);
    if( (m_vibratoWaveform & 3) == 3 )
    { // random vibrato
        m_vibratoPhase = (m_vibratoPhase + (std::rand() & 0x0f)) & 0x3f;
    }
    m_vibratoPhase = (m_vibratoPhase + m_lastVibratoData.hi()) & 0x3f;
    val = (val * m_lastVibratoData.lo()) >> 4;
    if( m_module->st2Vibrato() )
    {
        val >>= 1;
    }
    if( fine )
    {
        val >>= 2;
    }
    m_realPeriod = m_basePeriod + val;
    recalcFrequency();
}

void S3mChannel::fxNoteCut(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::NoteCut;
    fxByte &= 0x0f;
    if( m_module->state().tick == 0 )
    {
        m_countdown = fxByte;
        return;
    }
    if( m_countdown == 0 )
    {
        return;
    }
    m_countdown--;
    if( m_countdown == 0 )
    {
        m_state.active = false;
    }
}

void S3mChannel::fxNoteDelay(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::NoteDelay;
    fxByte &= 0x0f;
    if( m_module->state().tick == 0 )
    {
        m_countdown = fxByte;
        return;
    }
    if( m_countdown == 0 )
    {
        return;
    }
    m_countdown--;
    if( m_countdown == 0 )
    {
        m_state.active = true;
        playNote();
    }
}

void S3mChannel::fxGlobalVolume(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::GlobalVolume;
    if( fxByte <= 64 )
    {
        m_module->state().globalVolume = fxByte;
    }
}

void S3mChannel::fxFineTune(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::SetFinetune;
    if( m_module->state().tick != 0 )
    {
        return;
    }
    fxByte &= 0x0f;
    m_c2spd = m_basePeriod = m_realPeriod = S3mFinetunes[fxByte];
    recalcFrequency();
}

void S3mChannel::fxSetVibWaveform(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::SetVibWaveform;
    m_vibratoWaveform = fxByte & 0x0f;
}

void S3mChannel::fxSetTremWaveform(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::SetTremWaveform;
    m_tremoloWaveform = fxByte & 0x0f;
}

void S3mChannel::fxRetrigger(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::Retrigger;
    m_lastFxByte.noNibbles(fxByte);
    if( m_lastFxByte.lo() == 0 || m_lastFxByte.lo() > m_countdown )
    {
        m_countdown++;
        return;
    }
    m_countdown = 0;
    m_bresen = 0;
    int nvol = m_currentVolume;
    switch( m_lastFxByte.hi() )
    {
        case 0x0:
            break;
        case 0x1:
            nvol -= 1;
            break;
        case 0x2:
            nvol -= 2;
            break;
        case 0x3:
            nvol -= 4;
            break;
        case 0x4:
            nvol -= 8;
            break;
        case 0x5:
            nvol -= 16;
            break;
        case 0x6:
            nvol = nvol * 2 / 3;
            break;
        case 0x7:
            nvol /= 2;
            break;
        case 0x8:
            break;
        case 0x9:
            nvol += 1;
            break;
        case 0xa:
            nvol += 2;
            break;
        case 0xb:
            nvol += 4;
            break;
        case 0xc:
            nvol += 8;
            break;
        case 0xd:
            nvol += 16;
            break;
        case 0xe:
            nvol = nvol * 3 / 2;
            break;
        case 0xf:
            nvol *= 2;
            break;
    }
    m_currentVolume = clip<int>(nvol, 0, 63);
    recalcVolume();
}

void S3mChannel::fxOffset(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::Offset;
    m_bresen = fxByte << 8;
}

void S3mChannel::fxTremor(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::Tremor;
    m_lastFxByte.noNibbles(fxByte);
    if( m_tremorCounter != 0 )
    {
        m_tremorCounter--;
        return;
    }
    if( m_tremorMute )
    {
        m_tremorMute = false;
        m_currentVolume = 0;
        recalcVolume();
        m_tremorCounter = m_lastFxByte.lo();
    }
    else
    {
        m_tremorMute = true;
        m_currentVolume = m_baseVolume;
        recalcVolume();
        m_tremorCounter = m_lastFxByte.hi();
    }
}

void S3mChannel::fxTempo(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::SetTempo;
    if( fxByte <= 0x20 )
    {
        return;
    }
    m_module->setTempo(fxByte);
}

void S3mChannel::fxSpeed(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::SetSpeed;
    if( fxByte == 0 )
    {
        return;
    }
    m_module->setSpeed(fxByte);
}

void S3mChannel::fxArpeggio(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::Arpeggio;
    if( !currentSample() )
    {
        return;
    }
    m_lastFxByte.noNibbles(fxByte);
    switch( m_module->state().tick % 3 )
    {
        case 0: // normal note
            m_realPeriod = st3Period(m_note, currentSample()->frequency());
            break;
        case 1: // +x half notes...
            m_realPeriod = st3Period(deltaNote(m_note, m_lastFxByte.hi()), currentSample()->frequency());
            break;
        case 2: // +y half notes...
            m_realPeriod = st3Period(deltaNote(m_note, m_lastFxByte.lo()), currentSample()->frequency());
            break;
    }
    recalcFrequency();
}

void S3mChannel::fxSpecial(uint8_t fxByte)
{
    m_lastFxByte.noNibbles(fxByte);
    switch( static_cast<S3mSpecialEffects>(m_lastFxByte.hi()) )
    {
        case s3mSFxNoteDelay:
            fxNoteDelay(m_lastFxByte);
            break;
        case s3mSFxNoteCut:
            fxNoteCut(m_lastFxByte);
            break;
        case s3mSFxSetGlissando:
            m_state.fxDesc = fxdesc::Glissando;
            m_glissando = m_lastFxByte != 0;
            break;
        case s3mSFxSetFinetune:
            fxFineTune(m_lastFxByte);
            break;
        case s3mSFxSetVibWave:
            fxSetVibWaveform(m_lastFxByte);
            break;
        case s3mSFxSetTremWave:
            fxSetTremWaveform(m_lastFxByte);
            break;
        case s3mSFxSetPan:
            m_state.fxDesc = fxdesc::SetPanPos;
            m_panning = m_lastFxByte.lo() * 0x40 / 0x0f;
            break;
        case s3mSFxStereoCtrl:
            m_state.fxDesc = fxdesc::StereoControl;
            if( m_lastFxByte.lo() <= 7 )
            {
                m_panning = (m_lastFxByte.lo() + 8) * 0x40 / 0x0f;
            }
            else
            {
                m_panning = (m_lastFxByte.lo() - 8) * 0x40 / 0x0f;
            }
            break;
        case s3mSFxPatLoop:
        case s3mSFxPatDelay:
            // handled...
            break;
    }
}

void S3mChannel::fxTremolo(uint8_t fxByte)
{
    m_state.fxDesc = fxdesc::Tremolo;
    m_lastFxByte = fxByte;
    if( m_baseVolume == 0 )
    {
        return;
    }
    const int vol = m_baseVolume + ((m_lastFxByte.lo() * waveValue(m_tremoloWaveform, m_tremoloPhase)) >> 7);
    if( (m_tremoloWaveform & 3) == 3 )
    { // random vibrato
        m_tremoloPhase = (m_tremoloPhase + (std::rand() & 0x0f)) & 0x3f;
    }
    m_tremoloPhase = (m_tremoloPhase + m_lastFxByte.hi()) & 0x3f;
    m_currentVolume = clip(vol, 0, 63);
    recalcVolume();
}

uint16_t S3mChannel::glissando(uint16_t period)
{
    uint8_t no = periodToNoteOffset(period, currentSample()->frequency());
    return st3PeriodEx(no % 12, no / 12, currentSample()->frequency());
}

void S3mChannel::triggerNote()
{
    if( m_currentCell->effect() == s3mFxSpecial && (m_currentCell->effectValue() >> 4) == s3mSFxNoteDelay )
    {
        return;
    }
    playNote();
}

void S3mChannel::playNote()
{
    if( m_currentCell->instrument() >= 101 )
    {
        setSampleIndex(-1);
    }
    else if( m_currentCell->instrument() != s3mEmptyInstr )
    {
        setSampleIndex(m_currentCell->instrument() - 1);
        if( currentSample() )
        {
            m_baseVolume = m_currentVolume = std::min<uint8_t>(currentSample()->volume(), 63);
            m_c2spd = currentSample()->frequency();
            recalcVolume();
        }
        else
        {
            m_state.active = false;
            return;
        }
    }

    if( m_currentCell->note() != s3mEmptyNote )
    {
        if( m_currentCell->note() == s3mKeyOffNote )
        {
            m_realPeriod = 0;
            recalcFrequency();
            m_currentVolume = 0;
            recalcVolume();
            m_state.active = false;
        }
        else if( currentSample() )
        {
            m_state.active = true;
            m_portaTargetPeriod = st3Period(m_currentCell->note(), m_c2spd);
            if( (m_basePeriod == 0 || m_portaTargetPeriod == 0) || (m_currentCell->effect() != s3mFxPorta && m_currentCell->effect() != s3mFxPortaVolSlide) )
            {
                m_realPeriod = m_basePeriod = m_portaTargetPeriod;
                m_tremoloPhase = m_vibratoPhase = 0;
                recalcFrequency();
                if( m_currentCell->effect() == s3mFxOffset )
                {
                    fxOffset(m_currentCell->effectValue());
                }
                else
                {
                    m_bresen = 0;
                }
                m_note = m_currentCell->note();
                m_state.noteTriggered = true;
            }
        }
    }
    uint8_t vol = m_currentCell->volume();
    if( vol != s3mEmptyVolume )
    {
        vol = std::min<uint8_t>(vol, 63);
        m_currentVolume = vol;
        recalcVolume();
        m_baseVolume = vol;
    }
}

void S3mChannel::setPanning(uint8_t pan)
{
    if( pan > 0x40 && pan != 0xa4 )
    {
        return;
    }
    m_panning = pan;
}

light4cxx::Logger* S3mChannel::logger()
{
    return light4cxx::Logger::get("channel.s3m");
}
} // namespace s3m
} // namespace ppp

/**
 * @}
 */
