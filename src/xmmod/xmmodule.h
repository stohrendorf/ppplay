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

namespace ppp {
	namespace xm {
		class XmModule : public GenModule {
				DISABLE_COPY( XmModule )
			private:
				bool m_amiga;
				XmPattern::Vector m_patterns;
				XmInstrument::Vector m_instruments;
				XmChannel::Vector m_channels;
			public:
				typedef std::shared_ptr<XmModule> Ptr;
				XmModule( const uint32_t frq = 44100, const uint8_t maxRpt = 2 ) throw( PppException );
				virtual bool load( const std::string& filename ) throw( PppException );
				virtual uint16_t getTickBufLen() const throw( PppException );
				virtual void getTick( AudioFrameBuffer& buffer ) throw( PppException );
				virtual void getTickNoMixing( std::size_t& ) throw( PppException );
				virtual ppp::GenOrder::Ptr mapOrder( int16_t ) throw( PppException );
				virtual std::string getChanStatus( int16_t ) throw();
				virtual bool jumpNextTrack() throw( PppException );
				virtual bool jumpPrevTrack() throw( PppException );
				virtual bool jumpNextOrder() throw();
				virtual bool jumpPrevOrder() throw();
				virtual std::string getChanCellString( int16_t ) throw();
				virtual uint8_t channelCount() const;
		};
	} // namespace xm
} // namespace ppp

#endif // XMMODULE_H
