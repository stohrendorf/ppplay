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

#ifndef XMCHANNEL_H
#define XMCHANNEL_H

#include "genmod/breseninter.h"
#include "genmod/genchannel.h"
#include "xmpattern.h"
#include "xminstrument.h"

/**
 * @ingroup XmModule
 * @{
 */

namespace ppp {
namespace xm {

class XmModule;
class XmChannel : public GenChannel {
		DISABLE_COPY(XmChannel)
		XmChannel() = delete;
	public:
		typedef std::shared_ptr<XmChannel> Ptr;
		typedef std::vector<Ptr> Vector;
	private:
		/** @name Basic channel variables
		 * @{
		 */
		//! @brief Base volume
		uint8_t m_baseVolume;
		//! @brief Current volume (for tremolo or such fx)
		uint8_t m_currentVolume;
		//! @brief Real volume (including envelope)
		uint8_t m_realVolume;
		//! @brief Current panning (0x00..0x80)
		uint8_t m_panning;
		//! @brief Real panning (including envelope)
		uint8_t m_realPanning;
		//! @brief Base period
		uint16_t m_basePeriod;
		//! @brief Current period (for vibrato or such fx)
		uint16_t m_currentPeriod;
		//! @brief Added to m_currentPeriod
		int16_t m_autoVibDeltaPeriod;
		//! @brief Current finetune
		int8_t m_finetune;
		//! @brief Current instrument index
		uint8_t m_instrumentIndex;
		//! @brief Current base note
		uint8_t m_baseNote;
		//! @brief Current real note (including relative note)
		uint8_t m_realNote;
		//! @brief The current note cell
		XmCell m_currentCell;
		/** @} */

		/** @name Envelopes variables
		 * @{
		 */
		//! @brief Volume envelope processor
		XmEnvelopeProcessor m_volumeEnvelope;
		//! @brief Panning envelope processor
		XmEnvelopeProcessor m_panningEnvelope;
		//! @brief Current volume scale factor
		uint16_t m_volScale;
		//! @brief Current volume scale increase/decrease rate
		uint16_t m_volScaleRate;
		//! @brief For sustain points
		bool m_keyOn;
		/** @} */

		/** @name Effect backup variables
		 * @{
		 */
		//! @brief Last volume slide effect value
		uint8_t m_lastVolSlideFx;
		//! @brief Last porta up effect value
		uint8_t m_lastPortaUpFx;
		//! @brief Last porta down effect value
		uint8_t m_lastPortaDownFx;
		//! @brief Last panning slide effect value
		uint8_t m_lastPanSlideFx;
		//! @brief Last offset effect value
		uint8_t m_lastOffsetFx;
		//! @brief Last fine porta up effect value
		uint8_t m_lastFinePortaUpFx;
		//! @brief Last fine porta down effect value
		uint8_t m_lastFinePortaDownFx;
		//! @brief Last fine volume slide up effect value
		uint8_t m_lastFineVolUpFx;
		//! @brief Last fine volume slide down effect value
		uint8_t m_lastFineVolDownFx;
		//! @brief Last extra fine porta up effect value
		uint8_t m_lastXFinePortaUp;
		//! @brief Last extra fine porta down effect value
		uint8_t m_lastXFinePortaDown;
		//! @brief Last global volume slide effect value
		uint8_t m_lastGlobVolSlideFx;
		//! @brief Last tremor effect value
		uint8_t m_lastTremorFx;
		/** @} */

		/** @name Effect state variables
		 * @{
		 */
		//! @brief Current porta rate
		uint16_t m_portaSpeed;
		//! @brief Target period of porta effect
		uint16_t m_portaTargetPeriod;
		//! @brief Vibrato speed
		uint8_t m_vibratoSpeed;
		//! @brief Vibrato amplitude
		uint8_t m_vibratoDepth;
		//! @brief Vibrato phase
		uint8_t m_vibratoPhase;
		//! @brief Vibrato waveform and retrigger selector
		uint8_t m_vibratoCtrl;
		//! @brief @c true if Glissando is enabled
		bool m_glissandoCtrl;
		//! @brief Tremolo amplitude
		uint8_t m_tremoloDepth;
		//! @brief Tremolo speed
		uint8_t m_tremoloSpeed;
		//! @brief Tremolo phase
		uint8_t m_tremoloPhase;
		//! @brief Tremolo waveform selector
		uint8_t m_tremoloCtrl;
		//! @brief Countdown for the Tremor effect
		uint8_t m_tremorCountdown;
		//! @brief Counter for the Multi Retrigger Effect
		uint8_t m_retriggerCounter;
		//! @brief Retrigger delay in ticks
		uint8_t m_retriggerLength;
		//! @brief Volume change type for the Multi Retrigger Effect
		uint8_t m_retriggerVolumeType;
		uint8_t m_patLoopCounter;
		uint8_t m_patLoopRow;
		uint8_t m_autoVibType;
		uint8_t m_autoVibSweep;
		uint8_t m_autoVibSweepRate;
		uint16_t m_autoVibDepth;
		uint8_t m_autoVibPhase;
		/** @} */

		//! @brief Output rate controller
		BresenInterpolation m_bres;
		//! @brief Module this channel belongs to
		XmModule* m_module;
		std::string m_fxString;
		/**
		 * @brief Get the current sample
		 * @return Pointer to the current sample or NULL
		 */
		XmSample::Ptr currentSample();
		/**
		 * @brief Get the current instrument
		 * @return Pointer to the current instrument or NULL
		 */
		XmInstrument::Ptr currentInstrument();
	public:
		XmChannel(XmModule* module);
		virtual std::string noteName();
		virtual std::string effectName() const;
		virtual void mixTick(MixerFrameBuffer& mixBuffer);
		virtual void simTick(std::size_t bufSize);
		virtual void updateStatus();
		virtual std::string effectDescription() const;
		virtual std::string cellString();
		void update(XmCell::Ptr const cell);
		virtual IArchive& serialize(IArchive* data);
	private:
		/** @name Effect handlers
		 * @{
		 */
		void fxPortaUp(uint8_t fxByte);
		void fxPortaDown(uint8_t fxByte);
		void fxVolSlide(uint8_t fxByte);
		void fxPanSlide(uint8_t fxByte);
		void fxSetVolume(uint8_t fxByte);
		void fxSetPan(uint8_t fxByte);
		void fxSetTempoBpm(uint8_t fxByte);
		void fxOffset(uint8_t fxByte);
		void fxSetGlobalVolume(uint8_t fxByte);
		void fxExtraFinePorta(uint8_t fxByte);
		void fxExtended(uint8_t fxByte);
		void fxPorta();
		void fxArpeggio(uint8_t fxByte);
		void fxVibrato(uint8_t fxByte);
		void fxGlobalVolSlide(uint8_t fxByte);
		void fxTremolo(uint8_t fxByte);
		void fxTremor(uint8_t fxByte);
		void fxRetrigger(uint8_t fxByte);
		/** @} */

		/** @name Volume column effect handlers
		 * @{
		 */
		void vfxFineVolSlideDown(uint8_t fxByte);
		void vfxFineVolSlideUp(uint8_t fxByte);
		void vfxSetPan(uint8_t fxByte);
		void vfxSlideDown(uint8_t fxByte);
		void vfxSlideUp(uint8_t fxByte);
		void vfxSetVibratoSpeed(uint8_t fxByte);
		void vfxPanSlideLeft(uint8_t fxByte);
		void vfxPanSlideRight(uint8_t fxByte);
		void vfxVibrato(uint8_t fxByte);
		/** @} */

		/** @name Extended effect handlers
		 * @{
		 */
		void efxFinePortaUp(uint8_t fxByte);
		void efxFinePortaDown(uint8_t fxByte);
		void efxFineVolUp(uint8_t fxByte);
		void efxFineVolDown(uint8_t fxByte);
		void efxPatLoop(uint8_t fxByte);
		/** @} */

		void triggerNote();
		void retriggerNote();
		void doKeyOff();
		void doKeyOn();
		void doVibrato();
		void applySampleDefaults();
		void calculatePortaTarget(uint8_t targetNote);
};

}
}

/**
 * @}
 */

#endif
