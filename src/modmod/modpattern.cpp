#include "modpattern.h"

#include <boost/assert.hpp>

namespace ppp
{
namespace mod
{

ModPattern::ModPattern() : m_channels()
{
}

ModCell::Ptr ModPattern::createCell( uint16_t trackIndex, int16_t row )
{
	BOOST_ASSERT( ( row >= 0 ) && ( row <= 63 ) );
	BOOST_ASSERT( trackIndex < m_channels.size() );
	ModCell::Vector& track = m_channels.at( trackIndex );
	ModCell::Ptr& cell = track.at( row );
	if( cell ) {
		return cell;
	}
	cell.reset( new ModCell() );
	return cell;
}

ModCell::Ptr ModPattern::cellAt( uint16_t chanIdx, int16_t row )
{
	if( row < 0 ) {
		return ModCell::Ptr();
	}
	if( chanIdx >= m_channels.size() ) {
		return ModCell::Ptr();
	}
	return m_channels.at( chanIdx ).at( row );
}

bool ModPattern::load( BinStream& str, uint8_t numChans )
{
	m_channels.resize( numChans, ModCell::Vector( 64 ) );
	for( int i = 0; i < 64; i++ ) {
		for( size_t j = 0; j < numChans; j++ ) {
			ModCell::Ptr p = createCell( j, i );
			if( !p->load( str ) ) {
				return false;
			}
		}
	}
	return str.good();
}

}
}
