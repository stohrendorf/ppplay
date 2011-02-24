#ifndef XMMODULE_H
#define XMMODULE_H

#include "genmod/genmodule.h"
#include "xmpattern.h"
#include "xminstrument.h"

namespace ppp {
	namespace xm {
		class XmModule : public GenModule {
				DISABLE_COPY( XmModule )
			private:
				bool m_amiga;
				uint8_t m_channelCount;
				XmPattern::Vector m_patterns;
				XmInstrument::Vector m_instruments;
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
