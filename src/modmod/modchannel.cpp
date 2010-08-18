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

#include "genbase.h"
#include "modchannel.h"
#include "modpattern.h"
#include "breseninter.h"
//#include <cmath>

/**
 * @file
 * @ingroup ModMod
 * @brief PT Mod Channel Definitions, implementation
 */

using namespace std;

namespace ppp {
	namespace mod {
		/**
		 * @ingroup ModMod
		 * @brief Note periodic table for frequency calculations
		 */
		unsigned short Periods[12] = {1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016,  960,  907};
		
		/**
		 * @brief The highest available Amiga note (B-5)
		 * @ingroup ModMod
		 * @remarks In "Return 2the Dream:Nightbeat" this should be C-6.
		 */
		const unsigned char MAX_NOTE_AMIGA = 0x5b;

		/**
		 * @brief The lowest available Amiga note (C-2)
		 * @ingroup ModMod
		 */
		const unsigned char MIN_NOTE_AMIGA = 0x20;
	} // namespace mod
} // namespace ppp

using namespace ppp;
using namespace ppp::mod;

ModChannel::ModChannel(const Frequency frq, GenSampleList::CP smp) throw() : GenChannel(frq, smp),
		aNote(0), aLastFx(0), aLastPortaFx(0), aLastVibratoFx(0), aLastVibVolFx(0), aLastPortVolFx(0),
		aTremorVolume(0), aTargetNote(0), aNoteChanged(false), aDeltaFrq(0),
		aDeltaVolume(0), aMinFrequency(0), aMaxFrequency(0), aGlobalVol(0x40), aNextGlobalVol(0x40),
		aRetrigCount(-1), aTremorCount(-1), a300VolSlides(false), aAmigaLimits(false), aImmediateGlobalVol(false), aZeroVolCounter(-1) {
	aCurrCell.reset(new ModCell());
}

ModChannel::~ModChannel() throw() {
}

void ModChannel::useLastFxData(unsigned char &oldFx, unsigned char &newFx) const throw() {
	if (newFx == 0)
		newFx = oldFx;
	else
		oldFx = newFx;
	newFx = oldFx;
}

void ModChannel::combineLastFxData(unsigned char &oldFx, unsigned char &newFx) const throw() {
	if (newFx == 0)
		newFx = oldFx;
	else if (highNibble(newFx) == 0)
		oldFx = (newFx & 0x0f) | (oldFx & 0xf0);
	else if (lowNibble(newFx) == 0)
		oldFx = (newFx & 0xf0) | (oldFx & 0x0f);
	else
		oldFx = newFx;
	newFx = oldFx;
}

string ModChannel::getNoteName() throw(PppException) {
	return "   ";
}

string ModChannel::getFxName() const throw() {
	return "...";
}

unsigned int ModChannel::getPosition() const throw() {
	return aPosition;
}

Frequency ModChannel::getFrq() const throw() {
	//Frequency r = deltaFrq(aFrequency, aDeltaFrq);
	//return clip<Frequency>(r, aMinFrequency, aMaxFrequency);
	return 1;
}

void ModChannel::update(GenCell::CP const cell, const unsigned char tick, bool noRetrigger) throw() {

}

void ModChannel::doVolumeFx(const unsigned char fx, unsigned char fxVal) throw() {
	int tempVar;
	switch (fx) {
		case modFxVolSlide:
		case modFxVibVolSlide:
		case modFxPortVolSlide:
			if (fx == modFxVibVolSlide) {
				useLastFxData(aLastVibVolFx, fxVal);
				if ((highNibble(fxVal) == 0x0f) || (lowNibble(fxVal) == 0x0f))
					break;
			}
			else if (fx == modFxPortVolSlide) {
				useLastFxData(aLastPortVolFx, fxVal);
				if ((highNibble(fxVal) == 0x0f) || (lowNibble(fxVal) == 0x0f))
					break;
			}
			else
				useLastFxData(aLastFx, fxVal);
			tempVar = aVolume;
			if (highNibble(fxVal) == 0x00) { // slide down
				if ((aTick != 0) || a300VolSlides)
					tempVar = aVolume - lowNibble(fxVal);
			}
			else if (lowNibble(fxVal) == 0x00) { // slide up
				if ((aTick != 0) || a300VolSlides)
					tempVar = aVolume + highNibble(fxVal);
			}
			else if (highNibble(fxVal) == 0x0f) { // fine slide down
				if (aTick == 0)
					tempVar = aVolume - lowNibble(fxVal);
			}
			else if (lowNibble(fxVal) == 0x0f) { // fine slide up
				if (aTick == 0)
					tempVar = aVolume + highNibble(fxVal);
			}
			else { // slide down
				if ((aTick != 0) || a300VolSlides)
					tempVar = aVolume - lowNibble(fxVal);
			}
			aVolume = ppp::clip(tempVar, 0, 0x40);
			break;
		case modFxTremolo:
			combineLastFxData(aLastFx, fxVal);
			if (aTick != 0) {
				aTremolo += highNibble(fxVal)<<2;
				aDeltaVolume = (aTremolo.get()*lowNibble(fxVal)>>7);
			}
			break;
		case modFxChanVolume:
			aVolume = fxVal;
			break;
	}
}

void ModChannel::doVibratoFx(const unsigned char fx, unsigned char fxVal) throw() {
	switch (fx) {
		case modFxVibrato:
		case modFxVibVolSlide:
			if (fx == modFxVibVolSlide) {
				if ((highNibble(aLastVibVolFx) == 0x0f) || (lowNibble(aLastVibVolFx) == 0x0f))
					break;
			}
			else {
				combineLastFxData(aLastVibratoFx, fxVal);
				aLastVibVolFx = aLastVibratoFx;
			}
			fxVal = aLastVibratoFx;
			if (aTick != 0) {
				aVibrato += highNibble(fxVal) << 2;
				aDeltaFrq = aVibrato.get()*lowNibble(fxVal)>>5;
				/*switch (aVibratoType) {
					case vtSine:
					case vtSineNoRetrigger:
						aDeltaFrq = (SinLookup[aVibratoPhase] * lowNibble(fxVal)) >> 3;
						break;
					case vtRamp:
					case vtRampNoRetrigger:
						aDeltaFrq = (RampLookup[aVibratoPhase] * lowNibble(fxVal)) >> 3;
						break;
					case vtSquare:
					case vtSquareNoRetrigger:
						aDeltaFrq = (SquareLookup[aVibratoPhase] * lowNibble(fxVal)) >> 3;
						break;
					case vtRandom:
					case vtRandomNoRetrigger:
						switch (aTick % 3) {
							case 0:
								aDeltaFrq = (SinLookup[aVibratoPhase] * lowNibble(fxVal)) >> 3;
								break;
							case 1:
								aDeltaFrq = (RampLookup[aVibratoPhase] * lowNibble(fxVal)) >> 3;
								break;
							case 2:
								aDeltaFrq = (SquareLookup[aVibratoPhase] * lowNibble(fxVal)) >> 3;
								break;
						}
						break;
				}*/
			}
			break;
		case modFxFineVibrato:
			combineLastFxData(aLastVibratoFx, fxVal);
			if (aTick != 0) {
				aVibrato += highNibble(fxVal) << 2;
				aDeltaFrq = aVibrato.get()*lowNibble(fxVal)>>7;
				/*switch (aVibratoType) {
					case vtSine:
					case vtSineNoRetrigger:
						aDeltaFrq = (SinLookup[aVibratoPhase] * lowNibble(fxVal)) >> 5;
						break;
					case vtRamp:
					case vtRampNoRetrigger:
						aDeltaFrq = (RampLookup[aVibratoPhase] * lowNibble(fxVal)) >> 5;
						break;
					case vtSquare:
					case vtSquareNoRetrigger:
						aDeltaFrq = (SquareLookup[aVibratoPhase] * lowNibble(fxVal)) >> 5;
						break;
					case vtRandom:
					case vtRandomNoRetrigger:
						switch (aTick % 3) {
							case 0:
								aDeltaFrq = (SinLookup[aVibratoPhase] * lowNibble(fxVal)) >> 5;
								break;
							case 1:
								aDeltaFrq = (RampLookup[aVibratoPhase] * lowNibble(fxVal)) >> 5;
								break;
							case 2:
								aDeltaFrq = (SquareLookup[aVibratoPhase] * lowNibble(fxVal)) >> 5;
								break;
						}
						break;
				}*/
			}
			break;
	}
}

void ModChannel::doPitchFx(const unsigned char fx, unsigned char fxVal) throw() {

}

void ModChannel::doSpecialFx(const unsigned char fx, unsigned char fxVal) throw(PppException) {

}

void ModChannel::pitchUp(const short delta) throw() {
	pitchUp(aFrequency, delta);
}

void ModChannel::pitchDown(const short delta) throw() {
	pitchDown(aFrequency, delta);
}

void ModChannel::pitchUp(Frequency& frq, const short delta) throw() {
}

void ModChannel::pitchDown(Frequency& frq, const short delta) throw() {
}

void ModChannel::mixTick(AudioFifo::MixerBuffer &mixBuffer, const unsigned int bufSize, const unsigned char volume) throw(PppException) {
}

void ModChannel::simTick(const unsigned int bufSize, const unsigned char volume) {
}

string ModChannel::getFxDesc() const throw(PppException) {
	return "??????";
}


void ModChannel::updateStatus() throw() {
}

void ModChannel::setGlobalVolume(const unsigned char gVol, const bool applyNow) throw() {
	if (aTick == 0) {
		aNextGlobalVol = gVol;
	}
	if (aNoteChanged || applyNow) {
		aGlobalVol = aNextGlobalVol;
	}
}

BinStream &ModChannel::saveState(BinStream &str) const throw(PppException) {
	try {
		GenChannel::saveState(str)
			.write(aNote)
			.write(aLastFx)
			.write(aLastPortaFx)
			.write(aLastVibratoFx)
			.write(aLastVibVolFx)
			.write(aLastPortVolFx)
			.write(aTremorVolume)
			.write(aTargetNote)
			.write(aGlobalVol)
			.write(aNextGlobalVol)
			.write(aRetrigCount)
			.write(aTremorCount)
			.write(aNoteChanged)
			.write(a300VolSlides)
			.write(aAmigaLimits)
			.write(aDeltaFrq)
			.write(aDeltaVolume)
			.write(aZeroVolCounter)
			.write(aMinFrequency)
			.write(aMaxFrequency);
	}
	PPP_CATCH_ALL();
	return str;
}

BinStream &ModChannel::restoreState(BinStream &str) throw(PppException) {
	try {
		GenChannel::restoreState(str)
			.read(aNote)
			.read(aLastFx)
			.read(aLastPortaFx)
			.read(aLastVibratoFx)
			.read(aLastVibVolFx)
			.read(aLastPortVolFx)
			.read(aTremorVolume)
			.read(aTargetNote)
			.read(aGlobalVol)
			.read(aNextGlobalVol)
			.read(aRetrigCount)
			.read(aTremorCount)
			.read(aNoteChanged)
			.read(a300VolSlides)
			.read(aAmigaLimits)
			.read(aDeltaFrq)
			.read(aDeltaVolume)
			.read(aZeroVolCounter)
			.read(aMinFrequency)
			.read(aMaxFrequency);
	}
	PPP_CATCH_ALL();
	return str;
}
