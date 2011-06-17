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

#include "ppg.h"
#include <algorithm>
#include <vector>

namespace ppg {

	StereoPeakBar::StereoPeakBar( Widget* parent, int width, int max, int interLen, bool showPeak ) throw( Exception ) : Label( parent, " " ),
		m_interArrL(), m_interArrR(), m_max( 0 ), m_barLength( 0 ), m_showPeak( false ),
		m_peakPosL( 0 ), m_peakPosR( 0 ), m_peakFalloffSpeedL( 0 ), m_peakFalloffSpeedR( 0 ) {
		PPG_TEST( width < 8 || interLen < 1 );
		m_interArrL = std::vector<int>( interLen, 0 );
		m_interArrR = std::vector<int>( interLen, 0 );
		m_showPeak = showPeak;
		m_barLength = width;
		m_peakPosL = m_peakFalloffSpeedL = 0;
		m_peakPosR = m_peakFalloffSpeedR = 0;
		m_max = max;
		std::string txt = "[";
		for( int i = 0; i < width * 2; i++ ) {
			txt += " ";
			if( i + 1 == width )
				txt += "|";
		}
		txt += "]";
		setText( txt );
		setSize( length(), area().height() );
		int mpoint = length() / 2;
		for( int i = 0; i < width; i++ ) {
			switch( i * 4 / width ) {
				case 3:
					setFgColorRange( mpoint + i + 1, dcLightRed );
					setFgColorRange( mpoint - i - 1, dcLightRed );
					break;
				case 2:
					setFgColorRange( mpoint + i + 1, dcYellow );
					setFgColorRange( mpoint - i - 1, dcYellow );
					break;
				case 1:
					setFgColorRange( mpoint + i + 1, dcGreen );
					setFgColorRange( mpoint - i - 1, dcGreen );
					break;
				case 0:
					setFgColorRange( mpoint + i + 1, dcLightBlue );
					setFgColorRange( mpoint - i - 1, dcLightBlue );
					break;
			}
		}
		setFgColorRange( 0, dcGreen );
		setFgColorRange( mpoint, dcGreen );
		setFgColorRange( length() - 1, dcGreen );
	}

	StereoPeakBar::~StereoPeakBar() throw() {
	}

	void StereoPeakBar::shift( int lval, int rval ) throw( Exception ) {
		PPG_TEST( m_interArrL.size() == 0 || m_interArrR.size() != m_interArrL.size() );
		m_interArrL.erase( m_interArrL.begin() );
		m_interArrR.erase( m_interArrR.begin() );
		m_interArrL.push_back( lval );
		m_interArrR.push_back( rval );
		if( !m_showPeak )
			return;
		if( valueLeft() > m_peakPosL ) {
			m_peakFalloffSpeedL = 0;
			m_peakPosL = valueLeft();
		}
		else {
			m_peakFalloffSpeedL++;
			m_peakPosL = std::max( 0, m_peakPosL - ( m_peakFalloffSpeedL >> 2 ) );
			if( m_peakPosL == 0 )
				m_peakFalloffSpeedL = 0;
		}
		if( valueRight() > m_peakPosR ) {
			m_peakFalloffSpeedR = 0;
			m_peakPosR = valueRight();
		}
		else {
			m_peakFalloffSpeedR++;
			m_peakPosR = std::max( 0, m_peakPosR - ( m_peakFalloffSpeedR >> 2 ) );
			if( m_peakPosR == 0 )
				m_peakFalloffSpeedR = 0;
		}
		int valL = valueLeft();
		int valR = valueRight();
		int mpoint = length() / 2;
		for( int i = 0; i < m_barLength; i++ ) {
			if( valL > i * m_max / m_barLength )
				charAt(mpoint - 1 - i) = static_cast<char>( 0xfe );
			else
				charAt(mpoint - 1 - i) = static_cast<char>( 0xf9 );
			if( ( m_peakPosL != 0 ) && m_showPeak )
				charAt(mpoint - 1 - m_peakPosL * m_barLength / m_max) = static_cast<char>( 0x04 );
			if( valR > i * m_max / m_barLength )
				charAt(mpoint + 1 + i) = static_cast<char>( 0xfe );
			else
				charAt(mpoint + 1 + i) = static_cast<char>( 0xf9 );
			if( ( m_peakPosR != 0 ) && m_showPeak )
				charAt(mpoint + 1 + m_peakPosR * m_barLength / m_max) = static_cast<char>( 0x04 );
		}
	}

	void StereoPeakBar::shiftFrac( float lval, float rval ) throw( Exception ) {
		shift( static_cast<int>( lval * m_max ), static_cast<int>( rval * m_max ) );
	}

	int StereoPeakBar::valueLeft() const throw() {
		int res = 0;
		int div = 0;
		int i = 0;
		std::for_each(
		    m_interArrL.begin(),
		    m_interArrL.end(),
		[&res, &div, &i]( int v ) {
			i++;
			res += v * i;
			div += i;
		}
		);
		return res / div;
	}

	int StereoPeakBar::valueRight() const throw() {
		int res = 0;
		int div = 0;
		int i = 0;
		std::for_each(
		    m_interArrR.begin(),
		    m_interArrR.end(),
		[&res, &div, &i]( int v ) {
			i++;
			res += v * i;
			div += i;
		}
		);
		return res / div;
	}

} // namespace ppg
