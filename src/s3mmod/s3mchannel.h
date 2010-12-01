/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef s3mchannelH
#define s3mchannelH

#include "genmod/genchannel.h"
#include "s3mbase.h"
#include "s3msample.h"

#include <array>
#include "s3mpattern.h"

/**
* @file
* @ingroup S3mMod
* @brief S3M Channel Definitions
*/

namespace ppp {
	namespace s3m {
		extern const std::array<const int16_t, 64> S3mWaveSine;
		extern const std::array<const int16_t, 64> S3mWaveRamp;
		extern const std::array<const int16_t, 64> S3mWaveSquare;
		/**
		 * @class S3mChannel
		 * @ingroup S3mMod
		 * @brief The S3M Channel
		 */
		class S3mChannel : public GenChannel {
			public:
				typedef std::shared_ptr<S3mChannel> Ptr;
				typedef std::vector<Ptr> Vector;
			private:
				S3mChannel() = delete; //!< @brief No default constructor
				S3mChannel(const S3mChannel&) = delete; //!< @brief No copy constructor
				S3mChannel& operator=(const S3mChannel&) = delete; //!< @brief No assignment operator
				uint8_t m_note;          //!< @brief Currently playing note
				uint8_t m_lastFx;        //!< @brief Last FX Value
				uint8_t m_lastVibratoData; //!< @brief Last Vibrato FX
				uint8_t m_lastPortaSpeed;
				uint8_t m_tremorVolume;  //!< @brief Backup variable for Tremor FX
				uint8_t m_targetNote;    //!< @brief Target note for pitch slides
				bool m_noteChanged;            //!< @brief @c true when a new note triggered in the current frame
				int16_t m_deltaPeriod;              //!< @brief Vibrato delta period
				int16_t m_deltaVolume;           //!< @brief Tremolo delta volume
				uint8_t m_globalVol;     //!< @brief Global volume
				uint8_t m_nextGlobalVol; //!< @brief Next Global volume to be applied with a new note
				int8_t m_retrigCount;     //!< @brief Used for Retrigger Effect
				int8_t m_tremorCount;     //!< @brief Used for Tremor Effect
				bool m_300VolSlides;           //!< @brief Use STv3.00 Volume Slides
				bool m_amigaLimits;            //!< @brief Limit notes between C-2 and B-5
				bool m_maybeSchism;            //!< @brief Schism Tracker compatibility (when 16 bit or stereo samples are found and the tracker ID is ScreamTracker 3.20)
				int16_t m_zeroVolCounter;        //!< @brief Zero Volume Optimization counter, -1 if disabled
				const S3mSample::Vector* const m_sampleList;
				uint16_t m_basePeriod; //!< @brief The channel's period without the sample's c4speed applied
				uint16_t basePeriod();
				void setBasePeriod(uint16_t per);
				bool m_glissando;
				S3mCell::Ptr m_currentCell;
				S3mSample::Ptr currentSample() throw(PppException);
				/**
				 * @brief Apply Volume Effect
				 * @param[in] fx Effect
				 * @param[in] fxVal Effect data
				 * @note Time-critical
				 */
				void doVolumeFx(const uint8_t fx, uint8_t fxVal) throw();
				/**
				 * @brief Apply Vibrato Effect
				 * @param[in] fx Effect
				 * @param[in] fxVal Effect data
				 * @see ::s3mFxVibVolSlide
				 * @note Time-critical
				 */
				void doVibratoFx(const uint8_t fx, uint8_t fxVal) throw();
				/**
				 * @brief Apply Pitch Effect
				 * @param[in] fx Effect
				 * @param[in] fxVal Effect data
				 * @see ::s3mFxPortVolSlide
				 * @note Time-critical
				 */
				void doPitchFx(uint8_t fx, uint8_t fxVal) throw();
				/**
				 * @brief Apply Special Effect
				 * @param[in] fx Effect
				 * @param[in] fxVal Effect data
				 * @note Time-critical
				 */
				void doSpecialFx(const uint8_t fx, uint8_t fxVal) throw(PppException);
				/**
				 * @brief Pitch up
				 * @param[in] frq The base frequency
				 * @param[in] delta Value to pitch up
				 * @note Time-critical
				 */
				void pitchUp(uint16_t& per, int16_t delta) throw();
				/**
				 * @brief Pitch up
				 * @param[in] delta Value to pitch up
				 * @note Time-critical
				 */
				void pitchUp(int16_t delta) throw();
				/**
				 * @brief Pitch down
				 * @param[in] frq The base frequency
				 * @param[in] delta Value to pitch down
				 * @note Time-critical
				 */
				void pitchDown(uint16_t& per, int16_t delta) throw();
				/**
				 * @brief Pitch down
				 * @param[in] delta Value to pitch down
				 * @note Time-critical
				 */
				void pitchDown(int16_t delta) throw();
				/**
				 * @brief Use the old Effect data if the new one is 0x00
				 * @param[in,out] oldFx Old Effect Data
				 * @param[in,out] newFx New Effect Data
				 * @note Time-critical
				 */
				void useLastFxData(uint8_t &oldFx, uint8_t &newFx) const throw();
				/**
				 * @brief Use the old Effect data nibble if one of the new Effect nibbles is 0
				 * @param[in,out] oldFx Old Effect Data
				 * @param[in,out] newFx New Effect Data
				 * @note Time-critical
				 */
				void combineLastFxData(uint8_t &oldFx, uint8_t &newFx) const throw();
				int m_sampleIndex;
				void setSampleIndex(int32_t idx);
			public:
				/**
				 * @copydoc GenChannel::GenChannel
				 */
				S3mChannel(uint16_t frq, const S3mSample::Vector* const smp) throw();
				virtual ~S3mChannel() throw();
				virtual std::string getNoteName() throw(PppException);
				virtual std::string getFxName() const throw();
				uint16_t getAdjustedPeriod() throw();
				/**
				 * @brief Update the channel
				 * @param[in] cell Pointer to a note cell
				 * @param[in] tick Current tick
				 * @param[in] noRetrigger Don't trigger new notes, only apply effects (i.e. for Pattern Delays)
				 * @remarks A new value in the Instrument Column changes the instrument with the old playback position
				 * @note Time-critical
				 */
				void update(S3mCell::Ptr const cell, const uint8_t tick, bool noRetrigger = false) throw();
				virtual void mixTick(MixerFrameBuffer& mixBuffer, const uint8_t volume) throw(PppException);
				virtual void simTick(const std::size_t bufSize, const uint8_t volume);
				virtual void updateStatus() throw();
				virtual std::string getFxDesc() const throw(PppException);
				/**
				 * @brief Enable volume slides on tick 0
				 * @see S3mChannel::m_300VolSlides
				 */
				void enable300VolSlides() throw() {
					m_300VolSlides = true;
				}
				/**
				 * @brief Enable Amiga limits
				 * @see S3mChannel::m_amigaLimits
				 */
				void enableAmigaLimits() throw() {
					m_amigaLimits = true;
				}
				/**
				 * @brief Enable Zero Volume Optimization
				 * @see S3mChannel::m_zeroVolCounter
				 */
				void enableZeroVol() throw() {
					m_zeroVolCounter = 0;
				}
				/**
				 * @brief Enable Schism Tracker compatibility
				 * @see S3mChannel::m_maybeSchism
				 */
				void maybeSchism() throw() {
					m_maybeSchism = true;
				}
				/**
				 * @brief Sets global volume
				 * @param[in] gVol Global volume
				 * @param[in] applyNow Set to @c true to apply @a gVol instantly
				 * @see ppp::s3m::s3mFxGlobalVol
				 */
				void setGlobalVolume(const uint8_t gVol, const bool applyNow = false) throw();
				virtual BinStream &saveState(BinStream &str) const throw(PppException);
				virtual BinStream &restoreState(BinStream &str) throw(PppException);
				virtual std::string getCellString();
		};
	} // namespace s3m
} // namespace ppp

#endif
