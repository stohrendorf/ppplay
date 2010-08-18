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

#ifndef modmoduleH
#define modmoduleH

#include "genmodule.h"
#include "modchannel.h"
#include "modpattern.h"
#include "modsample.h"

/**
 * @file
 * @ingroup ModMod
 * @brief Module definitions for Protracker compatible Modules
 */

using namespace std;

namespace ppp {
	namespace mod {

		/**
		 * @class ModModule
		 * @ingroup ModMod
		 * @brief Module class for MOD Modules
		 */
		class ModModule : public GenModule {
			public:
				typedef std::tr1::shared_ptr<ModModule> CP;
			private:
				short aBreakRow;      //!< @brief Row to break to, -1 if unused
				short aBreakOrder;    //!< @brief Order to break to, -1 if unused
				short aPatLoopRow;    //!< @brief Row to loop back, -1 if unused
				short aPatLoopCount;  //!< @brief Loop counter for pattern loop, -1 if unused
				short aPatDelayCount; //!< @brief Delay counter for Pattern Delay, -1 if unused
				bool aCustomData;     //!< @brief @c true if module contains special custom data
				/**
				 * @brief Get a pattern
				 * @param[in] n Pattern index
				 * @return Pointer to the pattern
				 * @note Time-critical
				 */
				GenPattern::CP getPattern(int n) throw();
				/**
				 * @brief Get a channel
				 * @param[in] n Channel index
				 * @return Pointer to the channel
				 * @note Time-critical
				 */
				GenChannel::CP getChannel(int n) throw();
				/**
				 * @brief Get a sample
				 * @param[in] n Sample index
				 * @return Pointer to the sample
				 * @note Time-critical
				 */
				GenSample::CP getSmp(int n) throw();
				/**
				 * @brief Apply global effects
				 * @note Time-critical
				 */
				void checkGlobalFx() throw(PppException);
				/**
				 * @brief Adjust the playback position
				 * @param[in] increaseTick Whether to increase the tick value
				 * @param[in] doStore Set this to @c true to store the current state, and to @c false to restore it
				 */
				bool adjustPosition(const bool increaseTick, const bool doStore) throw(PppException);
				virtual BinStream &saveState() throw(PppException);
				virtual BinStream &restoreState(unsigned short ordindex, unsigned char cnt) throw(PppException);
			public:
				/**
				 * @copydoc GenModule::GenModule
				 */
				ModModule(const unsigned int frq = 44100, const unsigned short maxRpt = 2) throw(PppException);
				virtual ~ModModule() throw();
				virtual bool load(const string fn) throw(PppException);
				virtual bool existsSample(int idx) throw();
				virtual string getSampleName(int idx) throw();
				virtual bool existsInstr(int idx) const throw();
				virtual string getInstrName(int idx) const throw();
				inline unsigned short getTickBufLen() const throw(PppException);
				virtual void getTick(AudioFifo::AudioBuffer &buf, unsigned int &bufLen) throw(PppException);
				virtual void getTickNoMixing(unsigned int& bufLen) throw(PppException);
				virtual GenOrder::CP mapOrder(int order) throw();
				virtual string getChanStatus(int idx) throw();
				virtual bool jumpNextTrack() throw(PppException);
				virtual bool jumpPrevTrack() throw(PppException);
				virtual bool jumpNextOrder() throw();
		};

		inline unsigned short ModModule::getTickBufLen() const throw(PppException) {
			PPP_TEST(aPlaybackInfo.tempo < 0x20);
			return (static_cast<int>(aPlaybackFrequency*5 / aPlaybackInfo.tempo) >> 1);
		}
	} // namespace mod
} // namespace ppp

#endif
