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

#ifndef outsdlH
#define outsdlH

#include <SDL.h>
//#include <SDL_Sound.h>
#include "outgen.h"

/**
 * @file
 * @ingroup Output
 * @brief SDL output class (definition)
 */

namespace ppp {
	/**
	 * @class OutputSDL
	 * @ingroup Output
	 * @brief Output class for SDL
	 */
	class OutputSDL : public OutputGen {
		/**
		 * @copydoc ppp::OutputGen::OutputGen
		 * @details Initializes the SDL Audio Subsystem
		 */
		OutputSDL() : OutputGen() {
			PPP_TEST(0!=SDL_InitSubSystem(SDL_INIT_AUDIO));
		}
		/**
		 * @brief Destructor; Quits the SDL Audio Subsystem
		 */
		virtual ~OutputSDL() {
			SDL_QuitSubSystem(SDL_INIT_AUDIO);
		}
	}
}


#endif
