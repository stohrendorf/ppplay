/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

/**
 * @ingroup Common
 * @{
 */

#include "stateiterator.h"
#include "memarchive.h"

#include "light4cxx/logger.h"
#include <boost/format.hpp>

/**
 * @internal
 * @brief Logger with name "stateiterator"
 */
static light4cxx::Logger::Ptr logger = light4cxx::Logger::get( "stateiterator" );

StateIterator::StateIterator() : m_states(), m_stateIndex( 0 )
{
}

IArchive::Ptr StateIterator::newState()
{
	IArchive::Ptr p( new MemArchive() );
	logger->debug( L4CXX_LOCATION, "Creating new state %d" , m_states.size()+1 );
	m_states.push_back( p );
	return p;
}

IArchive::Ptr StateIterator::nextState()
{
	if( atEnd() ) {
		return IArchive::Ptr();
	}
	m_stateIndex++;
	logger->debug( L4CXX_LOCATION, "Loading state %d of %d", m_stateIndex+1, m_states.size() );
	return m_states.at( m_stateIndex );
}

IArchive::Ptr StateIterator::prevState()
{
	if( atFront() ) {
		return IArchive::Ptr();
	}
	m_stateIndex--;
	logger->debug( L4CXX_LOCATION, "Loading state %d of %d", m_stateIndex+1, m_states.size() );
	return m_states.at( m_stateIndex );
}

IArchive::Ptr StateIterator::currentState() const
{
	BOOST_ASSERT( m_stateIndex < m_states.size() );
	return m_states.at( m_stateIndex );
}

bool StateIterator::atEnd() const
{
	return m_stateIndex + 1 >= m_states.size();
}

bool StateIterator::atFront() const
{
	return m_stateIndex == 0;
}

void StateIterator::gotoFront()
{
	m_stateIndex = 0;
}

/**
 * @}
 */
