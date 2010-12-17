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

#ifndef s3mmoduleH
#define s3mmoduleH

#include "genmod/genmodule.h"
#include "s3mchannel.h"
#include "s3mpattern.h"
#include "s3msample.h"

#include <array>

/**
 * @file
 * @ingroup S3mMod
 * @brief Module definitions for ScreamTracker 3 Modules
 */

namespace ppp {
	namespace s3m {

		/**
		 * @class S3mModule
		 * @ingroup S3mMod
		 * @brief Module class for S3M Modules
		 */
		class S3mModule : public GenModule {
				DISABLE_COPY(S3mModule)
				S3mModule() = delete;
			public:
				typedef std::shared_ptr<S3mModule> Ptr;
			private:
				int16_t m_breakRow;      //!< @brief Row to break to, -1 if unused
				int16_t m_breakOrder;    //!< @brief Order to break to, -1 if unused
				int16_t m_patLoopRow;    //!< @brief Row to loop back, -1 if unused
				int16_t m_patLoopCount;  //!< @brief Loop counter for pattern loop, -1 if unused
				int16_t m_patDelayCount; //!< @brief Delay counter for Pattern Delay, -1 if unused
				bool m_customData;     //!< @brief @c true if module contains special custom data
				S3mSample::Vector m_samples;
				S3mPattern::Vector m_patterns;
				std::array<S3mChannel::Ptr, 32> m_channels;
				uint8_t m_usedChannels;
				std::array<uint16_t, 256> m_orderPlaybackCounts;
				S3mPattern::Ptr getPattern(size_t idx) const { if(idx>=m_patterns.size()) return S3mPattern::Ptr(); return m_patterns[idx]; }
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
				virtual IArchive& serialize(IArchive* data);
			public:
				/**
				 * @copydoc GenModule::GenModule
				 */
				S3mModule(const uint32_t frq = 44100, const uint8_t maxRpt = 2) throw(PppException);
				virtual ~S3mModule() throw();
				virtual uint8_t channelCount() const;
				virtual bool load(const std::string &fn) throw(PppException);
				virtual bool existsSample(int16_t idx) throw();
				virtual std::string getSampleName(int16_t idx) throw();
				virtual inline uint16_t getTickBufLen() const throw(PppException);
				virtual void getTick(AudioFrameBuffer &buf ) throw(PppException);
				virtual void getTickNoMixing(std::size_t& bufLen) throw(PppException);
				virtual GenOrder::Ptr mapOrder(int16_t order) throw();
				virtual std::string getChanStatus(int16_t idx) throw();
				virtual bool jumpNextTrack() throw(PppException);
				virtual bool jumpPrevTrack() throw(PppException);
				virtual bool jumpNextOrder() throw();
				virtual bool jumpPrevOrder() throw();
				virtual std::string getChanCellString(int16_t idx) throw();
		};
		
		inline uint16_t S3mModule::getTickBufLen() const throw(PppException) {
			PPP_TEST(getPlaybackInfo().tempo < 0x20);
			return getPlaybackFrq()*5 / (getPlaybackInfo().tempo<<1);
		}
	} // namespace s3m
} // namespace ppp

#endif
