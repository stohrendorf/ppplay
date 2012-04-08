/*
    PeePeePlayer - an old-fashioned module player
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

#ifndef S3MCHANNEL_H
#define S3MCHANNEL_H

/**
 * @ingroup S3mMod
 * @{
 */

#include "genmod/genchannel.h"
#include "genmod/breseninter.h"

#include <array>

namespace ppp
{
namespace s3m
{

class S3mModule;
class S3mCell;
class S3mSample;

/**
 * @class S3mChannel
 * @brief The S3M Channel
 *
 * @note Please note that even the S3M tech spec allows the cell volume to be
 * 64, it effectively is clipped to a value between 0 and <b>64</b>.
 */
class S3mChannel : public GenChannel
{
	S3mChannel() = delete; //!< @brief No default constructor
	DISABLE_COPY( S3mChannel )
private:
	uint8_t m_note;          //!< @brief Currently playing note
	uint8_t m_lastFxByte;        //!< @brief Last FX Value
	uint8_t m_lastVibratoData; //!< @brief Last Vibrato FX
	uint8_t m_lastPortaSpeed; //!< @brief Last porta speed
	uint8_t m_tremorVolume;  //!< @brief Backup variable for Tremor FX
	bool m_noteChanged;            //!< @brief @c true when a new note triggered in the current frame
	uint8_t m_currentVolume; //!< @brief Current volume, range: 0..63
	uint8_t m_realVolume; //!< @brief Real volume used when mixing, range 0..63
	uint8_t m_baseVolume; //!< @brief Base volume, range 0..63
	bool m_tremorMute; //!< @brief @c true if tremor is muted
	int8_t m_retrigCount;     //!< @brief Used for Retrigger Effect
	int16_t m_zeroVolCounter;        //!< @brief Zero Volume Optimization counter, -1 if disabled
	S3mModule* const m_module; //!< @brief Pointer to owning module
	uint16_t m_basePeriod; //!< @brief The channel's period without the sample's c4speed applied
	uint16_t m_realPeriod; //!< @brief Mixing period
	uint16_t m_portaTargetPeriod; //!< @brief Porta target period
	uint8_t m_vibratoPhase; //!< @brief Current vibrato phase
	uint8_t m_vibratoWaveform; //!< @brief Vibrato waveform selector
	uint8_t m_tremoloPhase; //!< @brief Current tremolo phase
	uint8_t m_tremoloWaveform; //!< @brief Tremolo waveform selector
	uint8_t m_countdown; //!< @brief A countdown helper shared between several effects
	uint8_t m_tremorCounter; //!< @brief Countdown helper for Tremor effect
	uint16_t m_c2spd; //!< @brief Current C2 frequency
	bool m_glissando; //!< @brief @c true if Glissando control is enabled
	S3mCell* m_currentCell; //!< @brief Current note cell
	BresenInterpolation m_bresen; //!< @brief Output rate controller
	std::string m_currentFxStr; //!< @brief Current effect string
	int m_sampleIndex; //!< @brief Current sample index
	uint8_t m_panning; //!< @brief Current panning (0..64, 0xa4 for surround)

	/**
	 * @brief Get the current sample
	 * @return Pointer to current sample or nullptr
	 */
	const S3mSample* currentSample() const;
	/**
	 * @brief Set the current sample index
	 * @param[in] idx New sample index
	 */
	void setSampleIndex( int32_t idx );

	// new implementation
	void fxPitchSlideUp( uint8_t fxByte );
	void fxPitchSlideDown( uint8_t fxByte );
	void fxVolSlide( uint8_t fxByte );
	void fxPorta( uint8_t fxByte, bool noReuse );
	void fxVibrato( uint8_t fxByte, bool fine, bool noReuse );
	void fxNoteCut( uint8_t fxByte );
	void fxNoteDelay( uint8_t fxByte );
	void fxGlobalVolume( uint8_t fxByte );
	void fxFineTune( uint8_t fxByte );
	void fxSetVibWaveform( uint8_t fxByte );
	void fxSetTremWaveform( uint8_t fxByte );
	void fxRetrigger( uint8_t fxByte );
	void fxOffset( uint8_t fxByte );
	void fxTremor( uint8_t fxByte );
	void fxTempo( uint8_t fxByte );
	void fxSpeed( uint8_t fxByte );
	void fxArpeggio( uint8_t fxByte );
	void fxSpecial( uint8_t fxByte );
	void fxTremolo( uint8_t fxByte );

	void triggerNote();
	void playNote();
	void recalcFrequency();
	uint16_t glissando( uint16_t period );

	virtual std::string internal_noteName() const;
	virtual void internal_mixTick( MixerFrameBuffer* mixBuffer );
	virtual void internal_updateStatus();
	virtual std::string internal_cellString() const;
	virtual std::string internal_effectName() const;
	virtual std::string internal_effectDescription() const;
public:
	/**
	 * @brief Constructor
	 * @param[in] module Owning module
	 */
	S3mChannel( S3mModule* module );
	virtual ~S3mChannel();
	virtual IArchive& serialize( IArchive* data );
	/**
	 * @brief Update the channel
	 * @param[in] cell Pointer to a note cell
	 * @param[in] patDelay For pattern delays
	 * @param[in] estimateOnly Used when estimating track length
	 */
	void update( const S3mCell* cell, bool patDelay, bool estimateOnly );
	/**
	 * @brief Recalculates the real output volume
	 */
	void recalcVolume();
	/**
	 * @brief Set the panning with range check
	 * @param[in] pan Panning value
	 * @pre @a pan must be within 0..64 or equal to 0xa4
	 */
	void setPanning( uint8_t pan );
protected:
	/**
	 * @brief Get the logger
	 * @return Child logger with attached ".s3m"
	 */
	static light4cxx::Logger::Ptr logger();
};
} // namespace s3m
} // namespace ppp

/**
 * @}
 */

#endif
