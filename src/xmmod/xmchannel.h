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

#ifndef XMCHANNEL_H
#define XMCHANNEL_H

#include "genmod/genchannel.h"
#include "genmod/breseninter.h"
#include "xmpattern.h"
#include "xminstrument.h"

/**
 * @ingroup XmModule
 * @{
 */

namespace ppp
{
namespace xm
{

class XmModule;

/**
 * @class XmChannel
 * @brief XM Channel class declaration
 */
class XmChannel : public GenChannel
{
	DISABLE_COPY( XmChannel )
	XmChannel() = delete;
public:
	//! @brief Class Pointer
	typedef std::shared_ptr<XmChannel> Ptr;
	//! @brief Vector of class pointers
	typedef std::vector<Ptr> Vector;
private:
	/** @name Basic channel variables
	 * @{
	 */
	//! @brief Base volume
	uint8_t m_baseVolume;
	//! @brief Current volume (for tremolo or such fx)
	uint8_t m_currentVolume;
	//! @brief Real volume (including envelope, volume scale and global volume)
	uint8_t m_realVolume;
	//! @brief Current panning (0x00..0x80)
	uint8_t m_panning;
	//! @brief Real panning (including envelope)
	uint8_t m_realPanning;
	//! @brief Base period
	uint16_t m_basePeriod;
	//! @brief Current period (for vibrato or such fx)
	uint16_t m_currentPeriod;
	//! @brief Auto-vibrato delta period added to m_currentPeriod
	int16_t m_autoVibDeltaPeriod;
	//! @brief Current finetune
	int8_t m_finetune;
	//! @brief Current instrument index (1-based)
	uint8_t m_instrumentIndex;
	//! @brief Current base note (0-based)
	uint8_t m_baseNote;
	//! @brief Current real note (0-based, including relative note)
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
	//! @brief Pattern loop counter, 0 when not active
	uint8_t m_patLoopCounter;
	//! @brief Pattern loop start row
	uint8_t m_patLoopRow;
	//! @brief Auto vibrato waveform selector
	uint8_t m_autoVibType;
	//! @brief Auto vibrato sweep value
	uint8_t m_autoVibSweep;
	//! @brief Auto vibrato sweep rate
	uint8_t m_autoVibSweepRate;
	//! @brief Auto vibrato depth
	uint16_t m_autoVibDepth;
	//! @brief Auto vibrato phase
	uint8_t m_autoVibPhase;
	/** @} */

	//! @brief Output rate controller
	BresenInterpolation m_bres;
	//! @brief Module this channel belongs to
	XmModule* m_module;
	//! @brief Current effect string
	std::string m_fxString;
	//! @brief Whether a note was triggered in the current row
	bool m_noteChanged;
	/**
	 * @brief Get the current sample
	 * @return Pointer to the current sample or nullptr
	 */
	XmSample::Ptr currentSample();
	/**
	 * @brief Get the current instrument
	 * @return Pointer to the current instrument or nullptr
	 */
	XmInstrument::Ptr currentInstrument();
public:
	/**
	 * @brief Constructor
	 * @param[in] module Pointer to the owning module
	 */
	XmChannel( XmModule* module );
	virtual std::string noteName();
	virtual std::string effectName() const;
	virtual void mixTick( MixerFrameBuffer* mixBuffer );
	virtual void updateStatus();
	virtual std::string effectDescription() const;
	virtual std::string cellString();
	/**
	 * @brief Update the channel values
	 * @param[in] cell The new cell
	 * @param[in] estimateOnly Set to @c true to skip expensive effects
	 */
	void update( const XmCell::Ptr& cell, bool estimateOnly );
	virtual IArchive& serialize( IArchive* data );
private:
	/** @name Effect handlers
	 * @{
	 */
	/**
	 * @brief Effect 1: Porta up
	 * @param[in] fxByte Effect value byte
	 */
	void fxPortaUp( uint8_t fxByte );
	/**
	 * @brief Effect 2: Porta down
	 * @param[in] fxByte Effect value byte
	 */
	void fxPortaDown( uint8_t fxByte );
	/**
	 * @brief Effect A: Volume slide
	 * @param[in] fxByte Effect value byte
	 */
	void fxVolSlide( uint8_t fxByte );
	/**
	 * @brief Effect P: Panning slide
	 * @param[in] fxByte Effect value byte
	 */
	void fxPanSlide( uint8_t fxByte );
	/**
	 * @brief Effect C: Set volume
	 * @param[in] fxByte Effect value byte
	 */
	void fxSetVolume( uint8_t fxByte );
	/**
	 * @brief Effect 8: Set panning
	 * @param[in] fxByte Effect value byte
	 */
	void fxSetPan( uint8_t fxByte );
	/**
	 * @brief Effect F: Set Tempo/BPM
	 * @param[in] fxByte Effect value byte
	 */
	void fxSetTempoBpm( uint8_t fxByte );
	/**
	 * @brief Effect 9: Sample offset
	 * @param[in] fxByte Effect value byte
	 */
	void fxOffset( uint8_t fxByte );
	/**
	 * @brief Effect G: Set global volume
	 * @param[in] fxByte Effect value byte
	 */
	void fxSetGlobalVolume( uint8_t fxByte );
	/**
	 * @brief Effect X: Extra fine porta
	 * @param[in] fxByte Effect value byte
	 */
	void fxExtraFinePorta( uint8_t fxByte );
	/**
	 * @brief Effect E: Extended effect
	 * @param[in] fxByte Effect value byte
	 * @param[in] estimateOnly Set to @c true to skip expensive effects
	 */
	void fxExtended( uint8_t fxByte, bool estimateOnly );
	/**
	 * @brief Effect 3: Tone porta
	 */
	void fxPorta();
	/**
	 * @brief Effect 0: Arpeggio
	 * @param[in] fxByte Effect value byte
	 */
	void fxArpeggio( uint8_t fxByte );
	/**
	 * @brief Effect 4: Vibrato
	 * @param[in] fxByte Effect value byte
	 */
	void fxVibrato( uint8_t fxByte );
	/**
	 * @brief Effect H: Global volume slide
	 * @param[in] fxByte Effect value byte
	 */
	void fxGlobalVolSlide( uint8_t fxByte );
	/**
	 * @brief Effect 7: Tremolo
	 * @param[in] fxByte Effect value byte
	 */
	void fxTremolo( uint8_t fxByte );
	/**
	 * @brief Effect T: Tremor
	 * @param[in] fxByte Effect value byte
	 */
	void fxTremor( uint8_t fxByte );
	/**
	 * @brief Effect R: Multi retrig note
	 * @param[in] fxByte Effect value byte
	 */
	void fxRetrigger( uint8_t fxByte );
	/** @} */

	/** @name Volume column effect handlers
	 * @{
	 */
	/**
	 * @brief Volume Effect "D": Fine volume slide down
	 * @param[in] fxByte Effect value byte
	 */
	void vfxFineVolSlideDown( uint8_t fxByte );
	/**
	 * @brief Volume Effect "U": Fine volume slide up
	 * @param[in] fxByte Effect value byte
	 */
	void vfxFineVolSlideUp( uint8_t fxByte );
	/**
	 * @brief Volume Effect "P": Set panning position
	 * @param[in] fxByte Effect value byte
	 */
	void vfxSetPan( uint8_t fxByte );
	/**
	 * @brief Volume Effect "-": Volume slide down
	 * @param[in] fxByte Effect value byte
	 */
	void vfxSlideDown( uint8_t fxByte );
	/**
	 * @brief Volume Effect "+": Volume slide up
	 * @param[in] fxByte Effect value byte
	 */
	void vfxSlideUp( uint8_t fxByte );
	/**
	 * @brief Volume Effect "S": Set vibrato speed
	 * @param[in] fxByte Effect value byte
	 */
	void vfxSetVibratoSpeed( uint8_t fxByte );
	/**
	 * @brief Volume Effect "L": Panning slide left
	 * @param[in] fxByte Effect value byte
	 */
	void vfxPanSlideLeft( uint8_t fxByte );
	/**
	 * @brief Volume Effect "R": Panning slide right
	 * @param[in] fxByte Effect value byte
	 */
	void vfxPanSlideRight( uint8_t fxByte );
	/**
	 * @brief Volume Effect "V": Vibrato
	 * @param[in] fxByte Effect value byte
	 */
	void vfxVibrato( uint8_t fxByte );
	/** @} */

	/** @name Extended effect handlers
	 * @{
	 */
	/**
	 * @brief Extended Effect 1: Fine porta slide up
	 * @param[in] fxByte Effect value byte
	 */
	void efxFinePortaUp( uint8_t fxByte );
	/**
	 * @brief Extended Effect 2: Fine porta slide down
	 * @param[in] fxByte Effect value byte
	 */
	void efxFinePortaDown( uint8_t fxByte );
	/**
	 * @brief Extended Effect A: Fine volume slide up
	 * @param[in] fxByte Effect value byte
	 */
	void efxFineVolUp( uint8_t fxByte );
	/**
	 * @brief Extended Effect B: Fine volume slide down
	 * @param[in] fxByte Effect value byte
	 */
	void efxFineVolDown( uint8_t fxByte );
	/**
	 * @brief Extended Effect 6: Pattern loop
	 * @param[in] fxByte Effect value byte
	 */
	void efxPatLoop( uint8_t fxByte );
	/** @} */

	/** @name Helper functions
	 * @{
	 */
	/**
	 * @brief Trigger note
	 */
	void triggerNote();
	/**
	 * @brief Retrigger note
	 */
	void retriggerNote();
	/**
	 * @brief Do Key Off effect
	 */
	void doKeyOff();
	/**
	 * @brief Do Key On effect
	 */
	void doKeyOn();
	/**
	 * @brief Calculate vibrato effect
	 */
	void doVibrato();
	/**
	 * @brief Applies sample default values if sample is present and valid
	 */
	void applySampleDefaults();
	/**
	 * @brief Calculates the porta target period
	 * @param[in] targetNote The porta target note
	 */
	void calculatePortaTarget( uint8_t targetNote );
	/** @} */
};

}
}

/**
 * @}
 */

#endif
