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

#include "stmbase.h"
#include "stmsample.h"

#include <algorithm>

using namespace ppp;
using namespace ppp::stm;

/**
* @file
* @brief STM Sample class
* @ingroup StmMod
*/

/**
* @brief STM Sample Header
* @ingroup StmMod
*/
typedef struct __attribute((packed)) {
	char title[12]; //!< @brief Sample's title
	unsigned char id; //!< @brief Sample header ID
	unsigned char instrDisk; //!< @brief Instrument disk number
	unsigned short paraPosition; //!< @brief Parapointer to the sample data
	unsigned short lastPos; //!< @brief Sample data length @test it's length-1... or not?
	unsigned short loopStart; //!< @brief Loop start point
	unsigned short loopEnd; //!< @brief Loop end point
	unsigned char volume; //!< @brief Sample's volume
	unsigned char reserved; //!< @brief Some reserved stuff
	unsigned short c3spd; //!< @brief C-3 sample speed
	unsigned char padding[6]; //!< @brief Padding
} stmSampleHeader;

StmSample::StmSample() throw() : GenSample() {
}

StmSample::~StmSample() throw() {
}

bool StmSample::load(BinStream& str, const std::size_t pos) throw(PppException) {
	try {
		LOG_BEGIN();
		stmSampleHeader smpHdr;
		str.seek(pos);
		str.read(smpHdr);
		if (smpHdr.id != 0) {
			LOG_WARNING(stringf("Sample ID not 0x00: 0x%.2x, assuming empty.", smpHdr.id));
			return true;
		}
		if ((smpHdr.lastPos == 0) || (smpHdr.paraPosition == 0)) {
			LOG_WARNING("Position or Length is zero, assuming empty.");
			return true;
		}
		m_length = smpHdr.lastPos + 1;
		m_loopStart = smpHdr.loopStart;
		m_loopEnd = smpHdr.loopEnd;
		m_volume = smpHdr.volume;
		m_baseFrq = smpHdr.c3spd;
		m_title = "";
		if (m_loopEnd == 0xffff) {
			m_loopStart = 0;
			m_loopEnd = 0;
			m_looptype = GenSample::LoopType::ltNone;
		}
		else {
			m_looptype = GenSample::LoopType::ltForward;
		}
		if (m_length == 0) {
			LOG_MESSAGE(" > Sample Length is 0");
			return true;
		}
		PPP_TEST(m_loopEnd > m_length);
		m_title += stringncpy(smpHdr.title, 12);
/*		for (int i = 0; (i < 12) && (smpHdr.title[i] != 0x00); i++)
			m_title += smpHdr.title[i];*/
		m_filename = "";
		// ok, header loaded, now load the sample data
		unsigned int posBackup = str.pos();
		str.seek(smpHdr.paraPosition << 4);
		//PPP_TEST(m_length == 0);
		if (str.fail()) {
			m_baseFrq = 0;
			str.seek(posBackup);
			LOG_WARNING("Seek failed or length is zero, assuming empty.");
			return true;
		}
		m_dataL.reset(new int16_t[m_length]);
		m_dataR = m_dataL;
		//memset(reinterpret_cast<void*>(m_dataL.get()), 0, m_length*2);
		std::fill_n(m_dataL.get(), m_length, 0);
		signed char smp8;
		short* smpPtr = m_dataL.get();
		for (unsigned int i = 0; i < m_length; i++) {
			str.read(smp8);
			if(str.fail()) {
				str.seek(posBackup);
				LOG_WARNING("EOF reached before Sample Data read completely, assuming zeroes.");
				return true;
			}
			*(smpPtr++) = smp8 << 8;
		}
		str.seek(posBackup);
		return true;
	}
	PPP_CATCH_ALL();
}
