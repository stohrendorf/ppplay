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

#ifndef outgenH
#define outgenH

#include "pppexcept.h"
#include "audiofifo.h"

/**
 * @defgroup Output Output routines
 * @brief Output routines
 */

/**
 * @file
 * @ingroup Output
 * @brief General output class (definition)
 */

namespace ppp {
	/**
	 * @class OutputGen
	 * @ingroup Output
	 * @brief Abstract base class for sound output
	 */
	class OutputGen {
		public:
			/**
			 * @brief Default constructor
			 */
			OutputGen() = default;
			/**
			 * @brief Prepare output
			 * @param[in] desiredFrq Desired output frequency
			 * @return Either the real output frequency, or -1 if init() failed
			 */
			virtual int init(int desiredFrq) = 0;
			/**
			 * @brief Feed data into the output
			 * @param[in] samples Sample data
			 * @param[in] frameCount Sample frames
			 */
			virtual void feedData(AudioFifo::AudioBuffer samples, unsigned int frameCount) = 0;
			/**
			 * @brief Hm...
			 * @return ?
			 */
			virtual int done() = 0;
	}
}

#endif
