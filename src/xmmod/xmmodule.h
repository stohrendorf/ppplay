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

#ifndef XMMODULE_H
#define XMMODULE_H

#include "genmod/genmodule.h"
#include "xmpattern.h"
#include "xminstrument.h"
#include "xmchannel.h"

/**
 * @defgroup XmModule XM Module definitions
 * @{
 */

/**
 * @file
 * @brief XM Module class declaration
 */

/**
 * @namespace ppp::xm
 * @brief Namespace for XM code
 */

namespace ppp {
	namespace xm {
		class XmModule : public GenModule {
				DISABLE_COPY( XmModule )
			private:
				//! @brief @c true if amiga period table is used
				bool m_amiga;
				//! @brief Module patterns
				XmPattern::Vector m_patterns;
				//! @brief Module instruments
				XmInstrument::Vector m_instruments;
				//! @brief Module channels
				XmChannel::Vector m_channels;
				//! @brief Maps notes including finetune to their periods
				std::array<uint16_t, 121*16> m_noteToPeriod;
				//! @brief Order list
				std::array<uint8_t, 256> m_orders;
				//! @brief Song length
				uint16_t m_length;
				//! @brief Contains the row to break to, or @c -1 if no break is intended
				int16_t m_patternBreak;
			public:
				//! @brief Class pointer
				typedef std::shared_ptr<XmModule> Ptr;
				/**
				 * @brief Constructor
				 * @param[in] frq Playback frequency
				 * @param[in] maxRpt maximum repeat count per order
				 */
				XmModule( const uint32_t frq = 44100, const uint8_t maxRpt = 2 ) throw( PppException );
				virtual bool load( const std::string& filename ) throw( PppException );
				virtual uint16_t getTickBufLen() const throw( PppException );
				virtual void getTick( AudioFrameBuffer& buffer );
				virtual void getTickNoMixing( std::size_t& ) throw( PppException );
				virtual ppp::GenOrder::Ptr mapOrder( int16_t ) throw( PppException );
				virtual std::string getChanStatus( int16_t ) throw();
				virtual bool jumpNextTrack() throw( PppException );
				virtual bool jumpPrevTrack() throw( PppException );
				virtual bool jumpNextOrder() throw();
				virtual bool jumpPrevOrder() throw();
				virtual std::string getChanCellString( int16_t ) throw();
				virtual uint8_t channelCount() const;
				/**
				 * @brief Get an instrument
				 * @param[in] idx 1-based instrument index
				 * @return Instrument pointer or NULL
				 */
				XmInstrument::Ptr getInstrument(int idx) const;
				/**
				 * @brief Map a note and finetune to its base period
				 * @param[in] note Note
				 * @param[in] finetune Finetune
				 * @return Period
				 */
				uint16_t noteToPeriod(uint8_t note, int8_t finetune) const;
				/**
				 * @brief Calculate the frequency from a period
				 * @param[in] period The period
				 * @return The calculated frequency
				 */
				uint32_t periodToFrequency(uint16_t period) const;
				/**
				 * @brief Apply glissando to a period
				 * @param[in] period Input period
				 * @param[in] finetune Input finetune
				 * @param[in] deltaNote Delta note used e.g. for Arpeggio
				 * @return Quantisized period
				 */
				uint16_t glissando(uint16_t period, int8_t finetune, uint8_t deltaNote = 0) const;
				/**
				 * @brief Request pattern break
				 * @param[in] next Row to break to
				 */
				void doPatternBreak(int16_t next);
		};
	} // namespace xm
} // namespace ppp

/**
 * @}
 */

#endif // XMMODULE_H
