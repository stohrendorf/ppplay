#include "modpattern.h"
#include "modcell.h"

#include "stuff/utils.h"

#include <boost/assert.hpp>

namespace ppp
{
namespace mod
{

ModPattern::ModPattern() : m_channels()
{
}

ModPattern::~ModPattern()
{
	for( auto& chan : m_channels ) {
		deleteAll(chan);
	}
	m_channels.clear();
}

ModCell* ModPattern::createCell( uint16_t chanIdx, int16_t row )
{
	BOOST_ASSERT( ( row >= 0 ) && ( row <= 63 ) );
	BOOST_ASSERT( chanIdx < m_channels.size() );
	auto& track = m_channels.at( chanIdx );
	auto& cell = track.at( row );
	if( cell ) {
		return cell;
	}
	cell = new ModCell();
	return cell;
}

ModCell* ModPattern::cellAt( uint16_t chanIdx, int16_t row )
{
	if( row < 0 ) {
		return nullptr;
	}
	if( chanIdx >= m_channels.size() ) {
		return nullptr;
	}
	return m_channels.at( chanIdx ).at( row );
}

bool ModPattern::load( BinStream& str, uint8_t numChans )
{
	m_channels.resize( numChans, std::vector<ModCell*>( 64, nullptr ) );
	for( int i = 0; i < 64; i++ ) {
		for( size_t j = 0; j < numChans; j++ ) {
			if( !createCell( j, i )->load( str ) ) {
				return false;
			}
		}
	}
	return str.good();
}

}
}
