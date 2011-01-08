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

#include "iaudiosource.h"

/**
 * @defgroup Output Output routines
 * @brief Output routines
 */

namespace ppp {
	/**
	 * @class OutputGen
	 * @ingroup Output
	 * @brief Abstract base class for sound output
	 */
	class OutputGen {
			DISABLE_COPY(OutputGen)
			OutputGen() = delete;
		private:
			IAudioSource* m_source; //!< @brief The audio source
		public:
			explicit OutputGen(IAudioSource* src) : m_source(src) {}
			virtual ~OutputGen();
			/**
			 * @brief Initialize output
			 * @param[in] desiredFrq Desired output frequency
			 * @return Either the real output frequency, or -1 if the call failed
			 */
			virtual int init(int desiredFrq) = 0;
			IAudioSource* const source() const { return m_source; }
	};
	OutputGen::~OutputGen() = default;
}

#endif
