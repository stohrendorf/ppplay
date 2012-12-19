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

#include "sample.h"
#include "breseninter.h"

namespace ppp
{

/**
 * @ingroup GenMod
 * @{
 */

Sample::Sample() noexcept :
	m_loopStart( 0 ), m_loopEnd( 0 ), m_volume( 0 ),
	m_frequency( 0 ), m_data(), m_filename(), m_title(), m_looptype( LoopType::None )
{
}

uint16_t Sample::frequency() const noexcept
{
	return m_frequency;
}

uint8_t Sample::volume() const noexcept
{
	return m_volume;
}

std::string Sample::title() const
{
	return m_title;
}

bool Sample::isLooped() const noexcept
{
	return m_looptype != LoopType::None;
}

std::streamsize Sample::length() const noexcept
{
	return m_data.size();
}

Sample::LoopType Sample::loopType() const noexcept
{
	return m_looptype;
}

void Sample::setFrequency( uint16_t f ) noexcept
{
	m_frequency = f;
}

void Sample::setLoopType( LoopType l ) noexcept
{
	m_looptype = l;
}

void Sample::setTitle( const std::string& t )
{
	m_title = t;
}

void Sample::setFilename( const std::string& f )
{
	m_filename = f;
}

void Sample::setLoopStart( std::streamoff s ) noexcept
{
	m_loopStart = s;
}

void Sample::setLoopEnd( std::streamoff e ) noexcept
{
	m_loopEnd = e;
}

void Sample::setVolume( uint8_t v ) noexcept
{
	m_volume = v;
}

light4cxx::Logger* Sample::logger()
{
	return light4cxx::Logger::get( "sample" );
}

bool Sample::mixNonInterpolated( BresenInterpolation* bresen, MixerFrameBuffer* buffer, int factorLeft, int factorRight, int rightShift ) const
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

bool Sample::mixLinearInterpolated( BresenInterpolation* bresen, MixerFrameBuffer* buffer, int factorLeft, int factorRight, int rightShift ) const
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

namespace
{
	constexpr int interpolateCubicLevel0(int p0, int p1, int p2, int p3, int bias)
	{
		return (bias*(3*(p1 - p2) + p3 - p0))>>8;
	}
	constexpr int interpolateCubicLevel1(int p0, int p1, int p2, int p3, int bias)
	{
		return (bias*((p0<<1) - 5*p1 + (p2<<2) - p3 + interpolateCubicLevel0(p0,p1,p2,p3,bias)))>>8;
	}
	constexpr MixerSample interpolateCubic(int p0, int p1, int p2, int p3, int bias)
	{
		return p1 + ((bias*(p2 - p0 + interpolateCubicLevel1(p0,p1,p2,p3,bias)))>>9);
	}
}

bool Sample::mixCubicInterpolated( BresenInterpolation* bresen, MixerFrameBuffer* buffer, int factorLeft, int factorRight, int rightShift ) const
{
	BOOST_ASSERT( bresen!=nullptr && buffer!=nullptr && rightShift>=0 );
	for( MixerSampleFrame & frame : **buffer ) {
		*bresen = adjustPosition( *bresen );
		if(!bresen->isValid()) {
			return false;
		}
		
		BasicSampleFrame samples[4];
		for(int i=0; i<4; i++) {
			samples[i] = sampleAt(adjustPosition(i+*bresen-1));
		}
		frame.left  += (factorLeft* interpolateCubic(samples[0].left,  samples[1].left,  samples[2].left,  samples[3].left,  bresen->bias()))>>rightShift;
		frame.right += (factorRight*interpolateCubic(samples[0].right, samples[1].right, samples[2].right, samples[3].right, bresen->bias()))>>rightShift;
		
		bresen->next();
	}
	return true;
}

bool Sample::mix( Sample::Interpolation inter, BresenInterpolation* bresen, MixerFrameBuffer* buffer, int factorLeft, int factorRight, int rightShift ) const
{
	switch(inter) {
		case Interpolation::None:
			return mixNonInterpolated(bresen,buffer,factorLeft,factorRight,rightShift);
		case Interpolation::Linear:
			return mixLinearInterpolated(bresen,buffer,factorLeft,factorRight,rightShift);
		case Interpolation::Cubic:
			return mixCubicInterpolated(bresen,buffer,factorLeft,factorRight,rightShift);
		default:
			return false;
	}
}

inline BasicSampleFrame Sample::sampleAt( std::streamoff pos ) const
{
	if( pos==BresenInterpolation::InvalidPosition || pos<0 ) {
		return BasicSampleFrame();
	}
	return m_data[makeRealPos( pos )];
}

std::streamoff Sample::adjustPosition( std::streamoff pos ) const
{
	if( pos<0 || pos == BresenInterpolation::InvalidPosition ) {
		return BresenInterpolation::InvalidPosition;
	}
	if( m_looptype == LoopType::None ) {
		if( pos >= length() ) {
			return BresenInterpolation::InvalidPosition;
		}
		return pos;
	}
	std::streamoff vLoopLen = m_loopEnd - m_loopStart;
	std::streamoff vLoopEnd = m_loopEnd;
	if( m_looptype == LoopType::Pingpong ) {
		vLoopLen *= 2;
		vLoopEnd = m_loopStart + vLoopLen;
	}
	while( pos >= vLoopEnd ) {
		pos -= vLoopLen;
	}
	return pos;
}

std::streamoff Sample::makeRealPos( std::streamoff pos ) const
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
