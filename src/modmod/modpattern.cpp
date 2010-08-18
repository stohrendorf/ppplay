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

#include "modpattern.h"
#include "genpattern.h"

/**
* @file
* @brief MOD Pattern class
* @ingroup ModMod
*/

using namespace ppp;
using namespace ppp::mod;

ModCell::ModCell() throw() : GenCell(), aNote(modEmptyNote), aInstr(modEmptyInstr), aVolume(modEmptyVolume),
		aEffect(modEmptyCommand), aEffectValue(0x00) {
}

ModCell::~ModCell() throw() {
}

bool ModCell::load(BinStream &str) throw(PppException) {
	try {
		reset();
		unsigned char master = 0;
		unsigned char buf;
		str.read(master);
		aActive = true;
		if (master&0x20) {
			str.read(buf);
			aNote = buf;
			str.read(buf);
			aInstr = buf;
		}
		if (master&0x40) {
			str.read(buf);
			aVolume = buf;
		}
		if (master&0x80) {
			str.read(buf);
			aEffect = buf;
			str.read(buf);
			aEffectValue = buf;
		}
	}
	catch (...) {
		LOG_ERROR("EXCEPTION");
		aActive = false;
		return false;
	}
	return true;
}

void ModCell::reset() throw() {
	GenCell::reset();
	aNote = modEmptyNote;
	aInstr = modEmptyInstr;
	aVolume = modEmptyVolume;
	aEffect = modEmptyCommand;
	aEffectValue = 0x00;
}

string ModCell::trackerString() const throw() {
	if (!isActive())
		return "... .. .. ...";
	string xmsg = "";
	if (aNote != modEmptyNote)
		xmsg += stringf("%s%d ", NoteNames[aNote&0x0f], aNote >> 4);
	else
		xmsg += "... ";
	if (aInstr != modEmptyInstr)
		xmsg += stringf("%.2d ", aInstr);
	else
		xmsg += ".. ";
	if (aVolume != modEmptyVolume)
		xmsg += stringf("%.2d ", aVolume);
	else
		xmsg += ".. ";
	if (aEffect != modEmptyCommand)
		xmsg += stringf("%c%.2x", 'A' -1 + aEffect, aEffectValue);
	else
		xmsg += "...";
	return xmsg;
}

ModCell &ModCell::operator=(const ModCell & src) throw() {
	GenCell::operator=(src);
	aNote = src.aNote;
	aInstr = src.aInstr;
	aVolume = src.aVolume;
	aEffect = src.aEffect;
	aEffectValue = src.aEffectValue;
	return *this;
}

unsigned char ModCell::getNote() const throw() {
	return aNote;
}

unsigned char ModCell::getInstr() const throw() {
	return aInstr;
}

unsigned char ModCell::getVolume() const throw() {
	return aVolume;
}

unsigned char ModCell::getEffect() const throw() {
	return aEffect;
}

unsigned char ModCell::getEffectValue() const throw() {
	return aEffectValue;
}


ModPattern::ModPattern() throw(PppException) : GenPattern() {
	try {
		for (int i = 0; i < 32; i++) {
			GenTrack::CP track(new GenTrack);
			for (int i = 0; i < 64; i++)
				track->push_back(GenCell::CP());
			aTracks.push_back(track);
		}
	}
	catch (...) {
		PPP_THROW("Unknown Exception");
	}
}

ModPattern::~ModPattern() throw() {
}

GenCell::CP ModPattern::createCell(int trackIndex, int row) throw(PppException) {
	PPP_TEST((row < 0) || (row > 63));
	GenTrack::CP track = getTrack(trackIndex);
	PPP_TEST( !track );
	GenCell::CP cell = track->at(row);
	if (cell)
		return cell;
	track->at(row).reset(new ModCell());
	return track->at(row);
}

bool ModPattern::load(BinStream& str, unsigned int pos) throw(PppException) {
	LOG_BEGIN();
	try {
		unsigned short patSize;
		str.seek(pos);
		str.read(patSize);
		unsigned short currRow = 0, currTrack = 0;
		while (currRow < 64) {
			unsigned char master;
			str.read(master);
			if (master == 0) {
				currRow++;
				continue;
			}
			currTrack = master & 31;
			str.seekrel(-1);
			if(str.fail()) {
				LOG_ERROR("str.fail()...");
				return false;
			}
			GenCell::CP cell = createCell(currTrack, currRow);
			if (!cell->load(str)) {
				LOG_ERROR("Cell loading: ERROR");
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
