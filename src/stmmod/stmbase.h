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


#ifndef stmbaseH
#define stmbaseH

#include "genbase.h"

/**
 * @defgroup StmMod ScreamTracker 2 and before module definitions
 * @brief This module contains the ScreamTracker 2 and before Module Classes
 * @details
 * It seems to be that STM modules have all the effects used by STv3,
 * they only have less channels and less instruments.
 */

/**
 * @file
 * @ingroup StmMod
 * @brief This file contains the base definitions for STM Modules
 */

namespace ppp {
	/**
	 * @namespace ppp::stm
	 * @ingroup StmMod
	 * @brief This namespace contains some consts that do not need to be public
	 */
	namespace stm {
		/**
		 * @ingroup StmMod
		 * @brief General consts
		 */
		enum {
			stmOrderEnd = 63 //!< @brief Song end marker
		};

		/**
		 * @ingroup StmMod
		 * @brief Effects
		 */
		enum {
			stmFxSetTempo = 0x01, //!< @brief A - Set tempo @test high nibble only? this is weird...
			stmFxPatJump = 0x02, //!< @brief B - Pattern jump
			stmFxPatBreak = 0x03, //!< @brief C - Pattern break
			stmFxVolSlide = 0x04, //!< @brief D - Volume slide
			stmFxSlideDown = 0x05, //!< @brief E - Pitch slide down
			stmFxSlideUp = 0x06, //!< @brief F - Pitch slide up
			stmFxPorta = 0x07, //!< @brief G - Porta
			stmFxVibrato = 0x08, //!< @brief H - Vibrato @test Check implementation: retrigger? depth? speed?
			stmFxTremor = 0x09, //!< @brief I - Tremor
			stmFxArpeggio = 0x0a, //!< @brief J - Arpeggio
			stmFxVibVolSlide = 0x0b, //!< @brief K - Combined Vibrato/Volume slide
			stmFxTremolo = 0x0c, //!< @brief L - Tremolo
			stmFxPortVolSlide = 0x0f //!< @brief O - Combined Porta/Volume slide
		};

		/**
		 * @ingroup StmMod
		 * @brief Lookup table for frequency calculations
		 */
		const int Periods [5][12] = {
			{1712, 1616, 1525, 1440, 1357, 1281, 1209, 1141, 1077, 1017, 961, 907},
			{856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453},
			{428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226},
			{214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113},
			{107, 101, 95, 90, 85, 80, 76, 71, 67, 64, 60, 57}
		};

	} // namespace stm
} // namespace ppp

#endif
