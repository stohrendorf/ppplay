/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "gensample.h"

/**
* @file
* @ingroup GenMod
* @brief General Sample definitions
*/

using namespace ppp;

GenSample::GenSample() throw() :
		m_16bit( false ), m_length( 0 ), m_loopStart( 0 ), m_loopEnd( 0 ), m_volume( 0 ),
		m_baseFrq( 0 ), m_dataL(NULL), m_dataR(NULL), m_filename( "" ), m_title( "" ), m_looptype( LoopType::None ) {
}

GenSample::GenSample( const GenSample &src ) throw( PppException ) :
		m_16bit( src.m_16bit ), m_length( src.m_length ), m_loopStart( src.m_loopStart ),
		m_loopEnd( src.m_loopEnd ), m_volume( src.m_volume ), m_baseFrq( src.m_baseFrq ), m_dataL(NULL), m_dataR(NULL),
		m_filename( src.m_filename ), m_title( src.m_title ), m_looptype( src.m_looptype ) {
	try {
		if(src.isStereo()) {
			setDataL(src.m_dataL);
			setDataR(src.m_dataR);
		}
		else
			setDataMono(src.m_dataL);
	}
	PPP_CATCH_ALL()
}

GenSample &GenSample::operator=( const GenSample & src ) throw( PppException ) {
	try {
		m_16bit = src.m_16bit;
		m_length = src.m_length;
		m_loopStart = src.m_loopStart;
		m_loopEnd = src.m_loopEnd;
		m_volume = src.m_volume;
		m_baseFrq = src.m_baseFrq;
		if(src.isStereo()) {
			setDataL(src.m_dataL);
			setDataR(src.m_dataR);
		}
		else
			setDataMono(src.m_dataL);
		m_filename = src.m_filename;
		m_title = src.m_title;
		m_looptype = src.m_looptype;
		return *this;
	}
	PPP_CATCH_ALL()
}

GenSample::~GenSample() throw() {
	if(isStereo())
		delete[] m_dataR;
	delete[] m_dataL;
}

void GenSample::setDataL(const int16_t data[]) throw() {
	if(m_dataL)
		delete[] m_dataL;
	m_dataL = new int16_t[m_length];
	std::copy(data, data+m_length, m_dataL);
}

void GenSample::setDataR(const int16_t data[]) throw() {
	if(m_dataR)
		delete[] m_dataR;
	m_dataR = new int16_t[m_length];
	std::copy(data, data+m_length, m_dataR);
}

void GenSample::setDataMono(const int16_t data[]) throw() {
	if(isStereo())
		delete[] m_dataR;
	m_dataR = NULL;
	delete[] m_dataL;
	m_dataL = NULL;
	setDataL(data);
	m_dataR = m_dataL;
}

PVECTOR_TEMPLATE_IMPL(ppp::GenSample)

SHARED_PTR_IMPL(ppp::GenSample)
