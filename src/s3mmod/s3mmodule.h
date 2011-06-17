/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#ifndef S3MMODULE_H
#define S3MMODULE_H

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
				DISABLE_COPY( S3mModule )
				S3mModule() = delete;
			public:
				typedef std::shared_ptr<S3mModule> Ptr;
				static GenModule::Ptr factory(const std::string& filename, uint32_t frequency, uint8_t maxRpt = 2);
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
				bool m_amigaLimits;
				bool m_fastVolSlides;
				bool m_st2Vibrato;
				bool m_zeroVolOpt;
				S3mPattern::Ptr getPattern( size_t idx ) const {
					if( idx >= m_patterns.size() ) return S3mPattern::Ptr();
					return m_patterns[idx];
				}
				/**
				 * @brief Apply global effects
				 * @note Time-critical
				 */
				void checkGlobalFx() throw( PppException );
				/**
				 * @brief Adjust the playback position
				 * @param[in] increaseTick Whether to increase the tick value
				 * @param[in] doStore Set this to @c true to store the current state, and to @c false to restore it
				 */
				bool adjustPosition( bool increaseTick, bool doStore ) throw( PppException );
			protected:
				virtual IArchive& serialize( IArchive* data );
			public:
				/**
				 * @copydoc GenModule::GenModule
				 */
				S3mModule( uint8_t maxRpt = 2 ) throw( PppException );
				virtual ~S3mModule() throw();
				virtual uint8_t channelCount() const;
				bool load( const std::string& fn ) throw( PppException );
				bool existsSample( int16_t idx ) throw();
				virtual uint16_t tickBufferLength() const throw( PppException );
				virtual void buildTick( AudioFrameBuffer& buf ) throw( PppException );
				virtual void simulateTick( std::size_t& bufLen ) throw( PppException );
				virtual GenOrder::Ptr mapOrder( int16_t order ) throw();
				virtual std::string channelStatus( int16_t idx ) throw();
				virtual bool jumpNextTrack() throw( PppException );
				virtual bool jumpPrevTrack() throw( PppException );
				virtual bool jumpNextOrder() throw();
				virtual bool jumpPrevOrder() throw();
				virtual std::string channelCellString( int16_t idx ) throw();
				uint8_t tick() const { return playbackInfo().tick; }
				bool hasAmigaLimits() const { return m_amigaLimits; }
				bool hasFastVolSlides() const { return m_fastVolSlides; }
				uint8_t globalVolume() const { return playbackInfo().globalVolume; }
				bool st2Vibrato() const { return m_st2Vibrato; }
				std::size_t numSamples() const { return m_samples.size(); }
				S3mSample::Ptr sampleAt(std::size_t idx) const { return m_samples.at(idx); }
				virtual void setGlobalVolume(int16_t v);
				bool hasZeroVolOpt() const { return m_zeroVolOpt; }
				virtual bool initialize(uint32_t frq);
		};
	} // namespace s3m
} // namespace ppp

#endif
