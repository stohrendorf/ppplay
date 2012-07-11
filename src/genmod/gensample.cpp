/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#include "gensample.h"
#include "breseninter.h"

namespace ppp
{

/**
 * @ingroup GenMod
 * @{
 */

GenSample::GenSample() :
	m_loopStart( 0 ), m_loopEnd( 0 ), m_volume( 0 ),
	m_frequency( 0 ), m_data(), m_filename(), m_title(), m_looptype( LoopType::None )
{
}

GenSample::~GenSample() = default;

uint16_t GenSample::frequency() const
{
	return m_frequency;
}

uint8_t GenSample::volume() const
{
	return m_volume;
}

std::string GenSample::title() const
{
	return m_title;
}

bool GenSample::isLooped() const
{
	return m_looptype != LoopType::None;
}

GenSample::PositionType GenSample::length() const
{
	return m_data.size();
}

GenSample::LoopType GenSample::loopType() const
{
	return m_looptype;
}

void GenSample::setFrequency( uint16_t f )
{
	m_frequency = f;
}

void GenSample::setLoopType( LoopType l )
{
	m_looptype = l;
}

void GenSample::setTitle( const std::string& t )
{
	m_title = t;
}

void GenSample::setFilename( const std::string& f )
{
	m_filename = f;
}

void GenSample::setLoopStart( PositionType s )
{
	m_loopStart = s;
}

void GenSample::setLoopEnd( PositionType e )
{
	m_loopEnd = e;
}

void GenSample::setVolume( uint8_t v )
{
	m_volume = v;
}

light4cxx::Logger* GenSample::logger()
{
	return light4cxx::Logger::get( "sample" );
}

bool GenSample::mixNonInterpolated( BresenInterpolation* bresen, MixerFrameBuffer* buffer, int factorLeft, int factorRight, int rightShift ) const
{
	BOOST_ASSERT( bresen!=nullptr && buffer!=nullptr && rightShift>=0 );
	for( MixerSampleFrame & frame : **buffer ) {
		*bresen = adjustPosition( *bresen );
		if(!bresen->isValid()) {
			return false;
		}
		BasicSampleFrame sampleVal = sampleAt( *bresen );
		sampleVal.mulRShift(factorLeft, factorRight, rightShift);
		frame += sampleVal;
		bresen->next();
	}
	return true;
}

bool GenSample::mixLinearInterpolated( BresenInterpolation* bresen, MixerFrameBuffer* buffer, int factorLeft, int factorRight, int rightShift ) const
{
	BOOST_ASSERT( bresen!=nullptr && buffer!=nullptr && rightShift>=0 );
	for( MixerSampleFrame & frame : **buffer ) {
		*bresen = adjustPosition( *bresen );
		if(!bresen->isValid()) {
			return false;
		}
		BasicSampleFrame sampleVal = sampleAt( *bresen );
		sampleVal = bresen->biased(sampleVal, sampleAt(adjustPosition(1+*bresen)));
		sampleVal.mulRShift(factorLeft, factorRight, rightShift);
		frame += sampleVal;
		bresen->next();
	}
	return true;
}

inline BasicSampleFrame GenSample::sampleAt( PositionType pos ) const
{
	if( pos == BresenInterpolation::InvalidPosition ) {
		return BasicSampleFrame();
	}
	return m_data[makeRealPos( pos )];
}

inline GenSample::PositionType GenSample::adjustPosition( PositionType pos ) const
{
	if( pos == BresenInterpolation::InvalidPosition ) {
		return BresenInterpolation::InvalidPosition;
	}
	if( m_looptype == LoopType::None ) {
		if( pos >= m_data.size() ) {
			return BresenInterpolation::InvalidPosition;
		}
		return pos;
	}
	PositionType vLoopLen = m_loopEnd - m_loopStart;
	PositionType vLoopEnd = m_loopEnd;
	if( m_looptype == LoopType::Pingpong ) {
		vLoopLen *= 2;
		vLoopEnd = m_loopStart + vLoopLen;
	}
	while( pos >= vLoopEnd ) {
		pos -= vLoopLen;
	}
	return pos;
}

inline GenSample::PositionType GenSample::makeRealPos( PositionType pos ) const
{
	if( m_looptype == LoopType::Pingpong ) {
		if( pos >= m_loopEnd ) {
			pos = 2 * m_loopEnd - pos;
		}
	}
	return pos;
}

/**
 * @}
 */

}
