/*
    PeePeePlayer - an old-fashioned module player
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

#include "modcell.h"

#include "modbase.h"

#include "genmod/genbase.h"
#include "logger/logger.h"
#include "stream/iarchive.h"

namespace ppp {
namespace mod {

ModCell::ModCell() : m_sampleNumber(0), m_period(0), m_effect(0), m_effectValue(0), m_noteIndex(255), m_note("...")
{
}

ModCell::~ModCell() = default;

bool ModCell::load(BinStream& str)
{
	clear();
	uint8_t tmp;
	str.read(&tmp);
	m_sampleNumber = tmp&0xf0;
	LOG_TEST_WARN(m_sampleNumber>31);
	m_sampleNumber &= 0x1f;
	m_period = (tmp&0x0f)<<8;
	str.read(&tmp);
	m_period |= tmp;
	str.read(&tmp);
	m_sampleNumber |= tmp>>4;
	m_effect = tmp&0x0f;
	str.read(&tmp);
	m_effectValue = tmp;
	if(m_period!=0) {
		uint16_t* p = std::find( fullPeriods.at(0).begin(), fullPeriods.at(0).end(), m_period );
		if(p!=fullPeriods.at(0).end()) {
			m_noteIndex = (p-fullPeriods.at(0).begin());
			m_note = ppp::stringf("%s%u", NoteNames[m_noteIndex%12], m_noteIndex/12);
		}
		else {
			m_note = "???";
			m_period = 0;
			m_noteIndex = 255;
		}
	}
	return str.good();
}

void ModCell::clear()
{
	m_sampleNumber = 0;
	m_period = 0;
	m_effect = 0;
	m_effectValue = 0;
	m_note.assign("...");
}

uint8_t ModCell::sampleNumber() const
{
	return m_sampleNumber;
}

uint16_t ModCell::period() const
{
	return m_period;
}

uint8_t ModCell::effect() const
{
	return m_effect;
}

uint8_t ModCell::effectValue() const
{
	return m_effectValue;
}

std::string ModCell::trackerString() const
{
	std::string res(m_note);
	if(m_sampleNumber!=0) {
		res.append(ppp::stringf(" %.2u", m_sampleNumber));
	}
	else {
		res.append("   ");
	}
	if(m_effect == 0x0f) {
		res.append(" ...");
	}
	else {
		res.append(" %.1X%.2X", m_effect, m_effectValue);
	}
	return res;
}

IArchive& ModCell::serialize(IArchive* data)
{
	*data % m_sampleNumber % m_period % m_effect % m_effectValue;
	if(data->isLoading()) {
		if(m_period!=0) {
			uint16_t* p = std::find( fullPeriods.at(0).begin(), fullPeriods.at(0).end(), m_period );
			if(p!=fullPeriods.at(0).end()) {
				m_noteIndex = (p-fullPeriods.at(0).begin());
				m_note = ppp::stringf("%s%u", NoteNames[m_noteIndex%12], m_noteIndex/12);
			}
			else {
				m_note = "???";
				m_period = 0;
				m_noteIndex = 255;
			}
		}
		else {
			m_note = "...";
		}
	}
	return *data;
}

}
}