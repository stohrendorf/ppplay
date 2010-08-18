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
#include <byteswap.h>
#include "modbase.h"
#include "modsample.h"

using namespace std;
using namespace ppp;
using namespace ppp::mod;

/**
* @file
* @brief MOD Sample class
* @ingroup ModMod
*/

/**
* @brief Flags for modSampleHeader::flags
* @ingroup ModMod
*/
enum {
	modFlagSmpLooped    = 0x01, //!< @brief Sample is looped
	modFlagSmpStereo    = 0x02, //!< @brief Sample is stereo
	modFlagSmp16bit     = 0x04  //!< @brief Sample has 16-bit samples
};

/**
* @brief MOD Sample Header
* @ingroup ModMod
*/
typedef struct __attribute__((packed)) {
	char title[22];
	unsigned short length;
	unsigned char finetune;
	unsigned char volume;
	unsigned short loopStart;
	unsigned short loopLength;
} modSampleHeader;

ModSample::ModSample() throw() : GenSample() {
}

ModSample::~ModSample() throw() {
}

bool ModSample::load(BinStream& str, const unsigned int pos) throw(PppException) {
	try {
		LOG_BEGIN();
		modSampleHeader smpHdr;
		str.seek(pos);
		str.read(smpHdr);
		if (smpHdr.length == 0)
			return true;
		aLength = bswap_16(smpHdr.length);
		aLoopStart = bswap_16(smpHdr.loopStart);
		aLoopEnd = bswap_16(smpHdr.loopStart) + bswap_16(smpHdr.loopLength);
		aVolume = smpHdr.volume;
		aStereo = false;
		aLooptype = smpHdr.loopLength==0 ? ltNone : ltForward;
		aTitle = stringncpy(smpHdr.title, 22);
		aFilename = "";
		return true;
	}
	PPP_CATCH_ALL();
}

bool ModSample::loadSampleData(BinStream &str) throw(PppException) {
	try {
		LOG_BEGIN();
		if(aLength==0)
			return true;
		aDataL.reset(new short[aLength]);
		aDataR = aDataL;
		signed char tmp;
		short *smpPtr = aDataL.get();
		for(unsigned int i=0; i<aLength; i++) {
			str.read(tmp);
			*(smpPtr++) = clip(tmp<<8,-32767,32767);
		}
		return true;
	}
	PPP_CATCH_ALL();
}
