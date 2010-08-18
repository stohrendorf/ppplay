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

#ifndef stmchannelH
#define stmchannelH

#include "genchannel.h"
#include "stmbase.h"

/**
* @file
* @ingroup StmMod
* @brief STM Channel Definitions
*/

namespace ppp {
	namespace stm {

		/**
		 * @class StmChannel
		 * @ingroup StmMod
		 * @brief The STM Channel
		 * @todo Apply the ::Phaser class
		 */
		class StmChannel : public GenChannel {
			public:
				typedef std::shared_ptr<StmChannel> CP;
			protected:
				unsigned char aNote;          //!< @brief Currently playing note
				unsigned char aLastFx;        //!< @brief Last FX Value
				unsigned char aLastPitchFx;   //!< @brief Last Pitch FX
				unsigned char aLastVibratoFx; //!< @brief Last Vibrato FX
				unsigned char aVibratoPhase;  //!< @brief Current vibrato phase
				unsigned char aTremoloPhase;  //!< @brief Current tremolo phase
				unsigned char aTargetNote;    //!< @brief Target note for pitch slides
				bool aNoteChanged;            //!< @brief @c true when a new note triggered in the current frame
				short aDeltaFrq;              //!< @brief Vibrato delta period
				short aDeltaVolume;           //!< @brief Tremolo delta volume
				Frequency aMinFrequency;      //!< @brief Minimum frequency (C-0)
				Frequency aMaxFrequency;      //!< @brief Maximum frequency (B-9)
				unsigned char aGlobalVol;     //!< @brief Global volume
				unsigned char aNextGlobalVol; //!< @brief Next Global volume to be applied with a new note
				signed char aRetrigCount;     //!< @brief Used for Retrigger Effect
				/**
				* @brief Apply Volume Effect
				* @param[in] fx Effect
				* @param[in] fxVal Effect data
				*/
				void doVolumeFx(const unsigned char fx, unsigned char fxVal) throw();
				/**
				* @brief Apply Vibrato Effect
				* @param[in] fx Effect
				* @param[in] fxVal Effect data
				* @param[in] isCombined Set to @c true if this effect is combined with the Volume Slide
				* @see ::s3mFxVibVolSlide
				*/
				void doVibratoFx(const unsigned char fx, unsigned char fxVal, const bool isCombined) throw();
				/**
				* @brief Apply Pitch Effect
				* @param[in] fx Effect
				* @param[in] fxVal Effect data
				* @param[in] isCombined Set to @c true if this effect is combined with the Volume Slide
				* @see ::s3mFxPortVolSlide
				*/
				void doPitchFx(unsigned char fx, unsigned char fxVal, bool isCombined) throw();
				/**
				* @brief Apply Special Effect
				* @param[in] fx Effect
				* @param[in] fxVal Effect data
				*/
				void doSpecialFx(const unsigned char fx, unsigned char fxVal) throw(PppException);
				/**
				* @brief Pitch up
				* @param[in] frq The base frequency
				* @param[in] delta Value to pitch up
				*/
				void pitchUp(Frequency& frq, short delta) throw();
				/**
				* @brief Pitch up
				* @param[in] delta Value to pitch up
				*/
				void pitchUp(short delta) throw();
				/**
				* @brief Pitch down
				* @param[in] frq The base frequency
				* @param[in] delta Value to pitch down
				*/
				void pitchDown(Frequency& frq, short delta) throw();
				/**
				* @brief Pitch down
				* @param[in] delta Value to pitch down
				*/
				void pitchDown(short delta) throw();
				/**
				* @brief Use the old Effect data if the new one is 0x00
				* @param[in,out] oldFx Old Effect Data
				* @param[in,out] newFx New Effect Data
				*/
				void useLastFxData(unsigned char &oldFx, unsigned char &newFx) const throw();
				/**
				* @brief Use the old Effect data nibble if one of the new Effect nibbles is 0
				* @param[in,out] oldFx Old Effect Data
				* @param[in,out] newFx New Effect Data
				*/
				void combineLastFxData(unsigned char &oldFx, unsigned char &newFx) const throw();
			public:
				/**
				 * @brief The constructor
				 * @param[in] frq Playback frequency
				 * @param[in] smp Pointer to the sample list used in this channel
				 * @pre @c smp!=NULL
				 * @see GenModule::GenModule()
				 */
				StmChannel(Frequency frq, const GenSampleList::Ptr &smp) throw();
				virtual ~StmChannel() throw();
				virtual std::string getNoteName() throw(PppException);
				virtual std::string getFxName() const throw(PppException);
				virtual std::size_t getPosition() const throw();
				virtual Frequency getFrq() const throw();
				/**
				 * @brief Update the channel
				 * @param[in] cell Pointer to a note cell
				 * @param[in] tick Current tick
				 * @param[in] noRetrigger Don't trigger new notes, only apply effects (i.e. for Pattern Delays)
				 * @remarks A new value in the Instrument Column changes the instrument with the old playback position
				 */
				virtual void update(GenCell::Ptr const cell, const uint8_t tick, bool noRetrigger = false) throw(PppException);
				virtual void mixTick(AudioFifo::MixerBuffer &mixBuffer, const std::size_t bufSize, const uint8_t volume) throw(PppException);
				virtual void simTick(const std::size_t bufSize, const uint8_t volume);
				virtual void updateStatus() throw(PppException);
				virtual std::string getFxDesc() const throw();
				/**
				 * @brief Sets global volume
				 * @param[in] gVol Global volume
				 * @param[in] applyNow Set to @c true to apply @a gVol instantly
				 * @see ppp::s3m::s3mFxGlobalVol
				 */
				void setGlobalVolume(const uint8_t gVol, const bool applyNow = false) throw();
		};
	} // namespace s3m
} // namespace ppp


#endif
