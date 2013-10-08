/*
    PPPlay - an old-fashioned module player
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

#include "stereopeakbar.h"

#include <boost/assert.hpp>

namespace ppg
{
StereoPeakBar::StereoPeakBar( Widget* parent, int width, int max, int interLen, bool showPeak ) : Label( parent, " " ),
    m_interArrL(), m_interArrR(), m_max( 0 ), m_barLength( 0 ), m_showPeak( false ),
    m_peakPosL( 0 ), m_peakPosR( 0 ), m_peakFalloffSpeedL( 0 ), m_peakFalloffSpeedR( 0 )
{
    width = std::max( width, 8 );
    interLen = std::max( interLen, 1 );
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
                setFgColorRange( mpoint + i + 1, Color::LightRed );
                setFgColorRange( mpoint - i - 1, Color::LightRed );
                break;
            case 2:
                setFgColorRange( mpoint + i + 1, Color::Yellow );
                setFgColorRange( mpoint - i - 1, Color::Yellow );
                break;
            case 1:
                setFgColorRange( mpoint + i + 1, Color::Green );
                setFgColorRange( mpoint - i - 1, Color::Green );
                break;
            case 0:
                setFgColorRange( mpoint + i + 1, Color::LightBlue );
                setFgColorRange( mpoint - i - 1, Color::LightBlue );
                break;
        }
    }
    setFgColorRange( 0, Color::Green );
    setFgColorRange( mpoint, Color::Green );
    setFgColorRange( length() - 1, Color::Green );
}

StereoPeakBar::~StereoPeakBar() = default;

void StereoPeakBar::shift( int lval, int rval )
{
    LockGuard guard( this );
    BOOST_ASSERT( !m_interArrL.empty() && m_interArrR.size() == m_interArrL.size() );
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
            charAt( mpoint - 1 - i ) = static_cast<char>( 0xfe );
        else
            charAt( mpoint - 1 - i ) = static_cast<char>( 0xf9 );
        if( ( m_peakPosL != 0 ) && m_showPeak )
            charAt( mpoint - 1 - m_peakPosL * m_barLength / m_max ) = static_cast<char>( 0x04 );
        if( valR > i * m_max / m_barLength )
            charAt( mpoint + 1 + i ) = static_cast<char>( 0xfe );
        else
            charAt( mpoint + 1 + i ) = static_cast<char>( 0xf9 );
        if( ( m_peakPosR != 0 ) && m_showPeak )
            charAt( mpoint + 1 + m_peakPosR * m_barLength / m_max ) = static_cast<char>( 0x04 );
    }
}

void StereoPeakBar::shiftFrac( float lval, float rval )
{
    LockGuard guard( this );
    shift( static_cast<int>( lval * m_max ), static_cast<int>( rval * m_max ) );
}

int StereoPeakBar::valueLeft() const
{
    LockGuard guard( this );
    int res = 0;
    int div = 0;
    int i = 1;
    for( int v : m_interArrL ) {
        res += i * v;
        div += i++;
    }
    return res / div;
}

int StereoPeakBar::valueRight() const
{
    LockGuard guard( this );
    int res = 0;
    int div = 0;
    int i = 1;
    for( int v : m_interArrR ) {
        res += i * v;
        div += i++;
    }
    return res / div;
}

}
