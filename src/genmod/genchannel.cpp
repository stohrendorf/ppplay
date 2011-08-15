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

/**
 * @ingroup GenMod
 * @{
 */

#include "genchannel.h"
#include "stream/iarchive.h"

namespace ppp {

GenChannel::GenChannel() :
	m_active(false), m_disabled(true),
	m_position(0),
	m_statusString(),
	m_statusStringMutex() {
}

GenChannel::~GenChannel() = default;

IArchive& GenChannel::serialize(IArchive* data) {
	*data % m_active % m_disabled % m_position;
	return *data;
}

std::string GenChannel::statusString() {
	std::lock_guard<std::mutex> lock(m_statusStringMutex);
	return m_statusString;
}

void GenChannel::setStatusString(const std::string& s) {
	std::lock_guard<std::mutex> lock(m_statusStringMutex);
	m_statusString = s;
}

void GenChannel::setActive(bool a) {
	m_active = a;
}

void GenChannel::setPosition(int32_t p) {
	m_position = p;
}

void GenChannel::enable() {
	m_disabled = false;
}

void GenChannel::disable() {
	m_disabled = true;
}

int32_t GenChannel::position() const {
	return m_position;
}

bool GenChannel::isDisabled() const {
	return m_disabled;
}

bool GenChannel::isActive() const {
	return m_active;
}

log4cxx::LoggerPtr GenChannel::logger()
{
	return log4cxx::Logger::getLogger("genchannel");
}

}
