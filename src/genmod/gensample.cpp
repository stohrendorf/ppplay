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

#include "gensample.h"

/**
* @file
* @ingroup GenMod
* @brief General Sample definitions
*/

using namespace ppp;

GenSample::GenSample() throw() :
	m_length( 0 ), m_loopStart( 0 ), m_loopEnd( 0 ), m_volume( 0 ),
	m_baseFrq( 0 ), m_dataL( NULL ), m_dataR( NULL ), m_filename( "" ), m_title( "" ), m_looptype( LoopType::None ) {
}

GenSample::~GenSample() throw() {
	if( isStereo() )
		delete[] m_dataR;
	delete[] m_dataL;
}

void GenSample::setDataL( const BasicSample data[] ) throw() {
	if( m_dataL && isStereo() )
		delete[] m_dataL;
	m_dataL = new BasicSample[m_length];
	std::copy( data, data + m_length, m_dataL );
}

void GenSample::setDataR( const BasicSample data[] ) throw() {
	if( m_dataR && isStereo() )
		delete[] m_dataR;
	m_dataR = new BasicSample[m_length];
	std::copy( data, data + m_length, m_dataR );
}

void GenSample::setDataMono( const BasicSample data[] ) throw() {
	if( isStereo() )
		delete[] m_dataR;
	m_dataR = NULL;
	delete[] m_dataL;
	m_dataL = NULL;
	setDataL( data );
	m_dataR = m_dataL;
}
