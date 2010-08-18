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

#ifndef stmmoduleH
#define stmmoduleH

#include "genmodule.h"
#include "stmchannel.h"
#include "stmpattern.h"
#include "stmsample.h"

/**
* @file
* @ingroup StmMod
* @brief Module definitions for ScreamTracker 2 and before Modules
*/

namespace ppp {
	namespace stm {

		/**
		* @class StmModule
		* @ingroup StmMod
		* @brief Module class for STM Modules
		*/
		class StmModule : public GenModule {
			public:
				typedef std::shared_ptr<StmModule> Ptr;
			private:
				short m_breakRow;      //!< @brief Row to break to, -1 if unused
				short m_breakOrder;    //!< @brief Order to break to, -1 if unused
				/**
				* @brief Get a pattern
				* @param[in] n Pattern index
				* @return Pointer to the pattern
				*/
				GenPattern::Ptr getPattern(int n) throw();
				/**
				* @brief Get a channel
				* @param[in] n Channel index
				* @return Pointer to the channel
				*/
				GenChannel::Ptr getChannel(int n) throw();
				/**
				* @brief Get a sample
				* @param[in] n Sample index
				* @return Pointer to the sample
				*/
				GenSample::Ptr getSmp(int n) throw();
				/**
				* @brief Apply global effects
				*/
				void checkGlobalFx() throw(PppException);
				/**
				* @brief Adjust the playback position
				* @param[in] increaseTick Whether to increase the tick value
				*/
				bool adjustPosition(const bool increaseTick) throw(PppException);
			public:
				/**
				 * @brief The constructor
				 * @param[in] frq Playback frequency, clipped to a value between 11025 and 44800
				 * @param[in] maxRpt Maximum repeat count for repeating modules
				 * @pre @c maxRpt>0
				 * @see GenChannel::GenChannel
				 */
				StmModule(const unsigned int frq = 44100, const unsigned short maxRpt = 1) throw(PppException);
				virtual ~StmModule() throw();
				virtual bool load(const std::string &fn) throw(PppException);
				virtual bool existsSample(int16_t idx) throw();
				virtual std::string getSampleName(int16_t idx) throw();
				virtual bool existsInstr(int16_t idx) const throw();
				virtual std::string getInstrName(int16_t idx) const throw();
				inline unsigned short getTickBufLen() const throw(PppException);
				virtual void getTick(AudioFifo::AudioBuffer &buf, std::size_t &bufLen) throw(PppException);
				virtual void getTickNoMixing(std::size_t& bufLen) throw(PppException);
				virtual GenOrder::Ptr mapOrder(int16_t order) throw();
				virtual std::string getChanStatus(int16_t idx) throw();
				virtual bool jumpNextTrack() throw();
				virtual bool jumpPrevTrack() throw();
				virtual bool jumpNextOrder() throw() {
					return true;
				}
				virtual std::string getChanCellString(int16_t /*idx*/) throw() {
					return "";
				}
		};

		inline std::size_t StmModule::getTickBufLen() const throw(PppException) {
			PPP_TEST(getPlaybackInfo().tempo < 0x20);
			return ((m_playbackFrequency*5 / getPlaybackInfo().tempo) >> 1);
		}
	} // namespace stm
} // namespace ppp

#endif
