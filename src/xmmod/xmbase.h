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

#ifndef XMBASE_H
#define XMBASE_H

#include <cstdint>

namespace ppp {
	namespace xm {
		enum class Effect : uint8_t {
			Arpeggio = 0x00,
			PitchUp = 0x01,
			PitchDown = 0x02,
			Porta = 0x03,
			Vibrato = 0x04,
			PortaVolSlide = 0x05,
			VibratoVolSlide = 0x06,
			Tremolo = 0x07,
			SetPan = 0x08,
			Offset = 0x09,
			VolSlide = 0x0a,
			PatJump = 0x0b,
			SetVolume = 0x0c,
			PatBreak = 0x0d,
			Extended = 0x0e,
			SetTempoBPM = 0x0f,
			SetGlobalVolume = 0x10,
			GlobalVolSlide = 0x11,
			KeyOff = 0x14,
			SetEnvelopePos = 0x15,
			PanSlide = 0x19,
			Retrigger = 0x1b,
			Tremor = 0x1d,
			ExtraFinePorta = 0x21
		};
		enum class VolumeEffect : uint8_t {
			VolSlideDown = 0x60,
			VolSlideUp = 0x70,
			FineVolSlideDown = 0x80,
			FineVolSlideUp = 0x90,
			SetVibSpeed = 0xa0,
			Vibrato = 0xb0,
			SetPan = 0xc0,
			PanSlideLeft = 0xd0,
			PanSlideRight = 0xe0,
			Porta = 0xf0
		};
		enum class ExtendedEffect : uint8_t {
			FinePortaUp = 0x01,
			FinePortaDown = 0x02,
			GlissandoCtrl = 0x03,
			VibratoCtrl = 0x04,
			Finetune = 0x05,
			PatLoop 0x06,
			TremoloCtrl = 0x07,
			Retrigger = 0x09,
			FineVolSlideUp = 0x0a,
			FineVolSlideDown = 0x0b,
			NoteCut = 0x0c,
			NoteDelay = 0x0d,
			PatDelay = 0x0e
		};
	}
}

#endif