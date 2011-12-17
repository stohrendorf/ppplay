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

#ifndef XMBASE_H
#define XMBASE_H

/**
 * @ingroup XmModule
 * @{
 */

namespace ppp
{
namespace xm
{

/**
 * @brief Available XM effects
 */
enum class Effect : uint8_t
{
	Arpeggio = 0,
	PortaUp = 1,
	PortaDown = 2,
	Porta = 3,
	Vibrato = 4,
	PortaVolSlide = 5,
	VibratoVolSlide = 6,
	Tremolo = 7,
	SetPanning = 8,
	Offset = 9,
	VolSlide = 0x0a,
	PosJump = 0x0b,
	SetVolume = 0x0c,
	PatBreak = 0x0d,
	Extended = 0x0e,
	SetTempoBpm = 0x0f,
	SetGlobalVol = 0x10,
	GlobalVolSlide = 0x11,
	KeyOff = 0x14,
	SetEnvPos = 0x15,
	PanSlide = 0x19,
	Retrigger = 0x1b,
	Tremor = 0x1d,
	ExtraFinePorta = 0x21,
	None = 0xff
};

/**
 * @brief Extended and volume effects
 */
enum {
	EfxFinePortaUp = 1,
	EfxFinePortaDown = 2,
	EfxSetGlissCtrl = 3,
	EfxSetVibratoCtrl = 4,
	EfxSetFinetune = 5,
	EfxPatLoop = 6,
	EfxSetTremoloCtrl = 7,
	EfxSetPan = 8,
	EfxRetrigger = 9,
	EfxFineVolSlideUp = 0x0a,
	EfxFineVolSlideDown = 0x0b,
	EfxNoteCut = 0x0c,
	EfxNoteDelay = 0x0d,
	EfxPatDelay = 0x0e,
	VfxNone = 0,
	VfxVolSlideDown = 6,
	VfxVolSlideUp = 7,
	VfxFineVolSlideDown = 8,
	VfxFineVolSlideUp = 9,
	VfxSetVibSpeed = 0xa,
	VfxVibrato = 0xb,
	VfxSetPanning = 0xc,
	VfxPanSlideLeft = 0xd,
	VfxPanSlideRight = 0xe,
	VfxPorta = 0xf,
	KeyOffNote = 97
};

}
}

/**
 * @}
 */

#endif
