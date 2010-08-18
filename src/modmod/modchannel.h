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

#ifndef modchannelH
#define modchannelH

#include <array>
#include <initializer_list>

#include "genchannel.h"
#include "modbase.h"

/**
* @file
* @ingroup ModMod
* @brief MOD Channel Definitions
*/

using namespace std;

namespace ppp {
	namespace mod {

		/**
		 * @class ModChannel
		 * @ingroup ModMod
		 * @brief The MOD Channel
		 */
		class ModChannel : public GenChannel {
			public:
				typedef std::tr1::shared_ptr<ModChannel> CP;
			private:
				ModChannel() = delete;
				ModChannel(const ModChannel&) = delete;
				ModChannel& operator=(const ModChannel&) = delete;
			protected:
				unsigned char aNote;          //!< @brief Currently playing note
				unsigned char aLastFx;        //!< @brief Last FX Value
				unsigned char aLastPortaFx;   //!< @brief Last Pitch FX
				unsigned char aLastVibratoFx; //!< @brief Last Vibrato FX
				unsigned char aLastVibVolFx;  //!< @brief Last Vibrato/Volume FX
				unsigned char aLastPortVolFx; //!< @brief Last Porta/Volume FX
				unsigned char aTremorVolume;  //!< @brief Backup variable for Tremor FX
				unsigned char aTargetNote;    //!< @brief Target note for pitch slides
				bool aNoteChanged;            //!< @brief @c true when a new note triggered in the current frame
				short aDeltaFrq;              //!< @brief Vibrato delta period
				short aDeltaVolume;           //!< @brief Tremolo delta volume
				Frequency aMinFrequency;      //!< @brief Minimum frequency (C-0)
				Frequency aMaxFrequency;      //!< @brief Maximum frequency (B-9)
				//unsigned char aTremoloPhase;  //!< @brief Tremolo phase
				unsigned char aGlobalVol;     //!< @brief Global volume
				unsigned char aNextGlobalVol; //!< @brief Next Global volume to be applied with a new note
				signed char aRetrigCount;     //!< @brief Used for Retrigger Effect
				signed char aTremorCount;     //!< @brief Used for Tremor Effect
				bool a300VolSlides;           //!< @brief Use STv3.00 Volume Slides
				bool aAmigaLimits;            //!< @brief Limit notes between C-2 and B-5
				bool aImmediateGlobalVol;     //!< @brief Apply global volume immediately for non-ST3 modules
				short aZeroVolCounter;        //!< @brief Zero Volume Optimization counter, -1 if disabled
				/**
				 * @brief Apply Volume Effect
				 * @param[in] fx Effect
				 * @param[in] fxVal Effect data
				 * @note Time-critical
				 */
				void doVolumeFx(const unsigned char fx, unsigned char fxVal) throw();
				/**
				 * @brief Apply Vibrato Effect
				 * @param[in] fx Effect
				 * @param[in] fxVal Effect data
				 * @see ::modFxVibVolSlide
				 * @note Time-critical
				 */
				void doVibratoFx(const unsigned char fx, unsigned char fxVal) throw();
				/**
				 * @brief Apply Pitch Effect
				 * @param[in] fx Effect
				 * @param[in] fxVal Effect data
				 * @see ::modFxPortVolSlide
				 * @note Time-critical
				 */
				void doPitchFx(unsigned char fx, unsigned char fxVal) throw();
				/**
				 * @brief Apply Special Effect
				 * @param[in] fx Effect
				 * @param[in] fxVal Effect data
				 * @note Time-critical
				 */
				void doSpecialFx(const unsigned char fx, unsigned char fxVal) throw(PppException);
				/**
				 * @brief Pitch up
				 * @param[in] frq The base frequency
				 * @param[in] delta Value to pitch up
				 * @note Time-critical
				 */
				void pitchUp(Frequency& frq, short delta) throw();
				/**
				 * @brief Pitch up
				 * @param[in] delta Value to pitch up
				 * @note Time-critical
				 */
				void pitchUp(short delta) throw();
				/**
				 * @brief Pitch down
				 * @param[in] frq The base frequency
				 * @param[in] delta Value to pitch down
				 * @note Time-critical
				 */
				void pitchDown(Frequency& frq, short delta) throw();
				/**
				 * @brief Pitch down
				 * @param[in] delta Value to pitch down
				 * @note Time-critical
				 */
				void pitchDown(short delta) throw();
				/**
				 * @brief Use the old Effect data if the new one is 0x00
				 * @param[in,out] oldFx Old Effect Data
				 * @param[in,out] newFx New Effect Data
				 * @note Time-critical
				 */
				void useLastFxData(unsigned char &oldFx, unsigned char &newFx) const throw();
				/**
				 * @brief Use the old Effect data nibble if one of the new Effect nibbles is 0
				 * @param[in,out] oldFx Old Effect Data
				 * @param[in,out] newFx New Effect Data
				 * @note Time-critical
				 */
				void combineLastFxData(unsigned char &oldFx, unsigned char &newFx) const throw();
			public:
				/**
				 * @copydoc GenChannel::GenChannel
				 */
				ModChannel(Frequency frq, GenSampleList::CP smp) throw();
				virtual ~ModChannel() throw();
				virtual string getNoteName() throw(PppException);
				virtual string getFxName() const throw();
				virtual unsigned int getPosition() const throw();
				virtual Frequency getFrq() const throw();
				/**
				 * @copydoc GenChannel::update
				 * @remarks A new value in the Instrument Column changes the instrument with the old playback position
				 * @note Time-critical
				 */
				virtual void update(GenCell::CP const cell, const unsigned char tick, bool noRetrigger = false) throw();
				virtual void mixTick(AudioFifo::MixerBuffer &mixBuffer, const unsigned int bufSize, const unsigned char volume) throw(PppException);
				virtual void simTick(const unsigned int bufSize, const unsigned char volume);
				virtual void updateStatus() throw();
				virtual string getFxDesc() const throw(PppException);
				/**
				 * @brief Enable volume slides on tick 0
				 * @see ::a300VolSlides
				 */
				void enable300VolSlides() throw() {
					a300VolSlides = true;
				}
				/**
				 * @brief Enable Amiga limits
				 * @see ::aAmigaLimits
				 */
				void enableAmigaLimits() throw() {
					aAmigaLimits = true;
				}
				/**
				 * @brief Enable Zero Volume Optimization
				 * @see ::aZeroVolCounter
				 */
				void enableZeroVol() throw() {
					aZeroVolCounter = 0;
				}
				/**
				 * @brief Sets global volume
				 * @param[in] gVol Global volume
				 * @param[in] applyNow Set to @c true to apply @a gVol instantly
				 * @see ppp::mod::modFxGlobalVol
				 */
				void setGlobalVolume(const unsigned char gVol, const bool applyNow = false) throw();
				/**
				 * @brief Disables delayed apply of the global volume
				 */
				void disableGlobalVolDelay() { aImmediateGlobalVol = true; }
				virtual BinStream &saveState(BinStream &str) const throw(PppException);
				virtual BinStream &restoreState(BinStream &str) throw(PppException);
		};
	} // namespace mod
} // namespace ppp


#endif
