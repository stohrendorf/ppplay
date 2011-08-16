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
	m_length(0), m_loopStart(0), m_loopEnd(0), m_volume(0),
	m_frequency(0), m_dataL(nullptr), m_dataR(nullptr), m_filename(), m_title(), m_looptype(LoopType::None) {
}

GenSample::~GenSample() {
	if(isStereo())
		delete[] m_dataR;
	delete[] m_dataL;
}

void GenSample::setDataLeft(const BasicSample data[]) {
	if(m_dataL && isStereo())
		delete[] m_dataL;
	m_dataL = new BasicSample[m_length];
	std::copy(data, data + m_length, m_dataL);
}

void GenSample::setDataRight(const BasicSample data[]) {
	if(m_dataR && isStereo())
		delete[] m_dataR;
	m_dataR = new BasicSample[m_length];
	std::copy(data, data + m_length, m_dataR);
}

void GenSample::setDataMono(const BasicSample data[]) {
	if(isStereo())
		delete[] m_dataR;
	m_dataR = nullptr;
	delete[] m_dataL;
	m_dataL = nullptr;
	setDataLeft(data);
	m_dataR = m_dataL;
}

bool GenSample::isStereo() const {
	return m_dataL != m_dataR;
}

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

size_t GenSample::length() const {
	return m_length;
}

GenSample::LoopType GenSample::loopType() const {
	return m_looptype;
}

void GenSample::setFrequency(uint16_t f) {
	m_frequency = f;
}

void GenSample::setLoopType(GenSample::LoopType l) {
	m_looptype = l;
}

const BasicSample* GenSample::dataLeft() const {
	return m_dataL;
}

const BasicSample* GenSample::dataMono() const {
	return m_dataL;
}

const BasicSample* GenSample::dataRight() const {
	return m_dataR;
}

BasicSample* GenSample::nonConstDataL() const {
	return m_dataL;
}

BasicSample* GenSample::nonConstDataMono() const {
	return m_dataL;
}

BasicSample* GenSample::nonConstDataR() const {
	return m_dataR;
}

void GenSample::setTitle(const std::string& t) {
	m_title = t;
}

void GenSample::setFilename(const std::string& f) {
	m_filename = f;
}

void GenSample::setLength(size_t l) {
	m_length = l;
}

void GenSample::setLoopStart(size_t s) {
	m_loopStart = s;
}

void GenSample::setLoopEnd(size_t e) {
	m_loopEnd = e;
}

void GenSample::setVolume(uint8_t v) {
	m_volume = v;
}

log4cxx::LoggerPtr GenSample::logger()
{
	return log4cxx::Logger::getLogger("sample");
}

}

/**
 * @}
 */
