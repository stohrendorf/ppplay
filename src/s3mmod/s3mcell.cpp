/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Syron <mr.syron@googlemail.com>

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

#include "s3mcell.h"
#include "s3mbase.h"
#include "stream/iarchive.h"

namespace ppp {
namespace s3m {

S3mCell::S3mCell() : GenCell(), m_note(s3mEmptyNote), m_instr(s3mEmptyInstr), m_volume(s3mEmptyVolume),
	m_effect(s3mEmptyCommand), m_effectValue(0x00) {
}

S3mCell::~S3mCell() {
}

bool S3mCell::load(BinStream& str) {
	try {
		reset();
		uint8_t master = 0;
		uint8_t buf;
		str.read(&master);
		if(master & 0x20) {
			str.read(&buf);
			m_note = buf;
			if((m_note >= 0x9b) && (m_note != s3mEmptyNote) && (m_note != s3mKeyOffNote)) {
				LOG_WARNING("File Position %.8x: Note out of range: %.2x", str.pos(), m_note);
				m_note = s3mEmptyNote;
			}
			str.read(&buf);
			m_instr = buf;
		}
		if(master & 0x40) {
			str.read(&buf);
			m_volume = buf;
			if(buf > 0x40) {
				LOG_WARNING("File Position %.8x: Volume out of range: %d", str.pos(), m_volume);
				m_volume = s3mEmptyVolume;
			}
		}
		if(master & 0x80) {
			str.read(&buf);
			m_effect = buf;
			str.read(&buf);
			m_effectValue = buf;
		}
	}
	catch(...) {
		LOG_ERROR("EXCEPTION");
		return false;
	}
	return true;
}

void S3mCell::reset() {
	m_note = s3mEmptyNote;
	m_instr = s3mEmptyInstr;
	m_volume = s3mEmptyVolume;
	m_effect = s3mEmptyCommand;
	m_effectValue = 0x00;
}

std::string S3mCell::trackerString() const {
	std::string xmsg = "";
	if(m_note == s3mEmptyNote)
		xmsg += "... ";
	else if(m_note == s3mKeyOffNote)
		xmsg += "^^  ";
	else
		xmsg += stringf("%s%d ", NoteNames[m_note & 0x0f], m_note >> 4);
	if(m_instr != s3mEmptyInstr)
		xmsg += stringf("%.2d ", m_instr);
	else
		xmsg += ".. ";
	if(m_volume != s3mEmptyVolume)
		xmsg += stringf("%.2d ", m_volume);
	else
		xmsg += ".. ";
	if(m_effect != s3mEmptyCommand)
		xmsg += stringf("%c%.2x", 'A' - 1 + m_effect, m_effectValue);
	else
		xmsg += "...";
	return xmsg;
}

uint8_t S3mCell::note() const {
	return m_note;
}

uint8_t S3mCell::instrument() const {
	return m_instr;
}

uint8_t S3mCell::volume() const {
	return m_volume;
}

uint8_t S3mCell::effect() const {
	return m_effect;
}

uint8_t S3mCell::effectValue() const {
	return m_effectValue;
}

IArchive& S3mCell::serialize(IArchive* data) {
	*data
	& m_note& m_instr& m_volume& m_effect& m_effectValue;
	return *data;
}

}
}
