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

/**
 * @ingroup GenMod
 * @{
 */

namespace ppp {

GenSample::GenSample() :
	m_loopStart(0), m_loopEnd(0), m_volume(0),
	m_frequency(0), m_data(), m_filename(), m_title(), m_looptype(LoopType::None) {
}

GenSample::~GenSample() = default;

uint16_t GenSample::frequency() const {
	return m_frequency;
}

uint8_t GenSample::volume() const {
	return m_volume;
}

std::string GenSample::title() const {
	return m_title;
}

bool GenSample::isLooped() const {
	return m_looptype != LoopType::None;
}

GenSample::PositionType GenSample::length() const {
	return m_data.size();
}

GenSample::LoopType GenSample::loopType() const {
	return m_looptype;
}

void GenSample::setFrequency(uint16_t f) {
	m_frequency = f;
}

void GenSample::setLoopType(LoopType l) {
	m_looptype = l;
}

void GenSample::setTitle(const std::string& t) {
	m_title = t;
}

void GenSample::setFilename(const std::string& f) {
	m_filename = f;
}

void GenSample::setLoopStart(PositionType s) {
	m_loopStart = s;
}

void GenSample::setLoopEnd( PositionType e ) {
	m_loopEnd = e;
}

void GenSample::setVolume(uint8_t v) {
	m_volume = v;
}

light4cxx::Logger::Ptr GenSample::logger()
{
	return light4cxx::Logger::get("sample");
}

}

/**
 * @}
 */
