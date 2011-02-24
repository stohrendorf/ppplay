/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#ifndef breseninterH
#define breseninterH

#include <cstdint>

namespace ppp {
	/**
	 * @class BresenInterpolation
	 * @ingroup Common
	 * @brief Discrete linear interpolation
	 * @note Time-critical
	 * @note Please note that all methods are inline.
	 * @details
	 * Thanks to Mr. Bresenham for giving me the idea for this discrete linear interpolation algorithm... @n
	 * Instead of using an interpolation formula like @code dy*n/dx @endcode this is an incremental
	 * algorithm that uses only additions and substractions instead of divisions and multiplications, it's a
	 * heavy speed-up.
	 */
	class BresenInterpolation {
		private:
			int32_t m_dx; //!< @brief Width of the line
			int32_t m_dy; //!< @brief Height of the line
			int32_t m_err; //!< @brief Error variable
		public:
			/**
			 * @brief Constructor
			 * @param[in] dx Width of the interpolation line
			 * @param[in] dy Height of the interpolation line
			 */
			BresenInterpolation(const int32_t dx, const int32_t dy) : m_dx(dx), m_dy(dy), m_err(m_dx/2) { }
			BresenInterpolation() = delete; //!< @brief No default constructor
			BresenInterpolation(const BresenInterpolation &) = default; //!< @brief Default constructor
			BresenInterpolation &operator=(const BresenInterpolation &) = default; //!< @brief Default assignment operator
			/**
			 * @brief Calculates the next interpolation step
			 * @param[in,out] pos Interpolation Y point to adjust
			 */
			inline void next(int32_t &pos) throw() {
				for(m_err-=m_dy; m_err<0; m_err+=m_dx)
					pos++;
			}
	};
}

#endif // breseninterH
