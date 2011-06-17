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
	m_frequency( 0 ), m_dataL( NULL ), m_dataR( NULL ), m_filename( ), m_title( ), m_looptype( LoopType::None ) {
}

GenSample::~GenSample() throw() {
	if( isStereo() )
		delete[] m_dataR;
	delete[] m_dataL;
}

void GenSample::setDataLeft( const BasicSample data[] ) throw() {
	if( m_dataL && isStereo() )
		delete[] m_dataL;
	m_dataL = new BasicSample[m_length];
	std::copy( data, data + m_length, m_dataL );
}

void GenSample::setDataRight( const BasicSample data[] ) throw() {
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
	setDataLeft( data );
	m_dataR = m_dataL;
}

bool GenSample::isStereo() const throw()
{
    return m_dataL != m_dataR;
}

uint16_t GenSample::frequency() const throw()
{
    return m_frequency;
}

uint8_t GenSample::volume() const throw()
{
    return m_volume;
}

std::string GenSample::title() const throw()
{
    return m_title;
}

bool GenSample::isLooped() const throw()
{
    return m_looptype != LoopType::None;
}

std::size_t GenSample::length() const throw()
{
    return m_length;
}

GenSample::LoopType GenSample::loopType() const throw()
{
    return m_looptype;
}

void GenSample::setFrequency(uint16_t f) throw()
{
    m_frequency = f;
}

void GenSample::setLoopType(GenSample::LoopType l) throw()
{
    m_looptype = l;
}

const BasicSample *GenSample::dataLeft() const throw()
{
    return m_dataL;
}

const BasicSample *GenSample::dataMono() const throw()
{
    return m_dataL;
}

const BasicSample *GenSample::dataRight() const throw()
{
    return m_dataR;
}

BasicSample *GenSample::nonConstDataL() const throw()
{
    return m_dataL;
}

BasicSample *GenSample::nonConstDataMono() const throw()
{
    return m_dataL;
}

BasicSample *GenSample::nonConstDataR() const throw()
{
    return m_dataR;
}

void GenSample::setTitle(const std::string &t) throw()
{
    m_title = t;
}

void GenSample::setFilename(const std::string &f) throw()
{
    m_filename = f;
}

void GenSample::setLength(std::size_t l) throw()
{
    m_length = l;
}

void GenSample::setLoopStart(std::size_t s) throw()
{
    m_loopStart = s;
}

void GenSample::setLoopEnd(std::size_t e) throw()
{
    m_loopEnd = e;
}

void GenSample::setVolume(uint8_t v) throw()
{
    m_volume = v;
}
