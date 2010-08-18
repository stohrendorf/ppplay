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

#include "stmpattern.h"
#include "genpattern.h"

/**
* @file
* @brief STM Pattern class
* @ingroup StmMod
*/

using namespace ppp;
using namespace ppp::stm;

StmCell::StmCell() throw() : GenCell(), aNote(stmEmptyNote), aInstr(stmEmptyInstr), aVolume(stmEmptyVolume),
		aEffect(stmEmptyEffect), aEffectValue(0x00) {
}

StmCell::~StmCell() throw() {
}

bool StmCell::load(BinStream &str) throw(PppException) {
	/// @test Is this right?
	try {
		reset();
		unsigned char buf;
		str.read(buf);
		m_active = true;
		if (buf == 253) {
			aNote = stmEmptyNote;
			aInstr = 0;
			aEffect = 0;
			aEffectValue = 0;
		}
		else if (buf == 252) {
			aNote = stmKeyOffNote;
			aInstr = 0;
			aEffect = 0;
			aEffectValue = 0;
		}
		else if (buf == 251) {
			aNote = 0;
			aInstr = 0;
			aEffect = 0;
			aEffectValue = 0;
		}
		else {
			aNote = buf;
			str.read(buf);
			aInstr = buf >> 3;
			aVolume = buf & 0x07;
			str.read(buf);
			aVolume |= (buf & 0xf0) >> 1;
			aEffect = buf & 0x0f;
			str.read(buf);
			aEffectValue = buf;
		}
	}
	PPP_CATCH_ALL();
	return true;
}

void StmCell::reset() throw() {
	GenCell::reset();
	aNote = stmEmptyNote;
	aInstr = stmEmptyInstr;
	aVolume = stmEmptyVolume;
	aEffect = stmEmptyEffect;
	aEffectValue = 0x00;
}

std::string StmCell::trackerString() const throw() {
	if (!isActive())
		return "... .. .. ...";
	std::string xmsg = "";
	if (aNote != stmEmptyNote)
		xmsg += stringf("%s%d ", NoteNames[aNote&0x0f], aNote >> 4);
	else
		xmsg += "... ";
	if (aInstr != stmEmptyInstr)
		xmsg += stringf("%.2d ", aInstr);
	else
		xmsg += ".. ";
	if (aVolume != stmEmptyVolume)
		xmsg += stringf("%.2d ", aVolume);
	else
		xmsg += ".. ";
	if (aEffect != stmEmptyEffect)
		xmsg += stringf("%c%.2x", 'A' -1 + aEffect, aEffectValue);
	else
		xmsg += stringf(".%.2x", aEffectValue);
	return xmsg;
}

StmCell &StmCell::operator=(const StmCell & src) throw() {
	GenCell::operator=(src);
	aNote = src.aNote;
	aInstr = src.aInstr;
	aVolume = src.aVolume;
	aEffect = src.aEffect;
	aEffectValue = src.aEffectValue;
	return *this;
}

unsigned char StmCell::getNote() const throw() {
	return aNote;
}

unsigned char StmCell::getInstr() const throw() {
	return aInstr;
}

unsigned char StmCell::getVolume() const throw() {
	return aVolume;
}

unsigned char StmCell::getEffect() const throw() {
	return aEffect;
}

unsigned char StmCell::getEffectValue() const throw() {
	return aEffectValue;
}


StmPattern::StmPattern() throw(PppException) : GenPattern() {
	try {
		for (int i = 0; i < 4; i++) {
			GenTrack::Ptr track(new GenTrack());
			for (int i = 0; i < 64; i++)
				track->push_back(GenCell::Ptr());
			m_tracks.push_back(track);
		}
	}
	catch (...) {
		PPP_THROW("Unknown Exception");
	}
}

StmPattern::~StmPattern() throw() {
}

GenCell::Ptr StmPattern::createCell(int16_t trackIndex, int16_t row) throw() {
	PPP_TEST((row < 0) || (row > 63));
	GenTrack::Ptr track = getTrack(trackIndex);
	PPP_TEST( !track );
	GenCell::Ptr cell = track->at(row);
	if (cell)
		return cell;
	track->at(row).reset(new StmCell());
	return track->at(row);
}

bool StmPattern::load( BinStream& str, std::size_t /*pos*/) throw(PppException) {
	try {
		for (int row = 0; row < 64; row++) {
			for (int chan = 0; chan < 4; chan++) {
				GenCell::Ptr cell = createCell(chan, row);
				if (!cell->load(str))
					return false;
			}
		}
		return true;
	}
	PPP_RETHROW()
	catch (...) {
		PPP_THROW("Unknown Exception");
	}
}
