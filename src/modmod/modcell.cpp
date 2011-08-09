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

#include "genmod/genbase.h"
#include "logger/logger.h"
#include "stream/iarchive.h"

#include <array>

static std::array<uint16_t, 3*12> g_periods = {{
	856,808,762,720,678,640,604,570,538,508,480,453,
	428,404,381,360,339,320,302,285,269,254,240,226,
	214,202,190,180,170,160,151,143,135,127,120,113
}};

namespace ppp {
namespace mod {

ModCell::ModCell() : m_sampleNumber(0), m_period(0), m_effect(0), m_effectValue(0), m_note("...")
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
		uint16_t* p = std::find( g_periods.begin(), g_periods.end(), m_period );
		if(p!=g_periods.end()) {
			uint8_t o = (p-g_periods.begin());
			m_note = ppp::stringf("%s%u", NoteNames[o%12], o/12);
		}
		else {
			m_note = "???";
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
			uint16_t* p = std::find( g_periods.begin(), g_periods.end(), m_period );
			if(p!=g_periods.end()) {
				uint8_t o = (p-g_periods.begin());
				m_note = ppp::stringf("%s%u", NoteNames[o%12], o/12);
			}
			else {
				m_note = "???";
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