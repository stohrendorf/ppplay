/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef PPPLAY_BRESENINTER_H
#define PPPLAY_BRESENINTER_H

#include "ppplay_core_export.h"

#include <stuff/utils.h>
#include "sample.h"

#include <cstdint>

namespace ppp
{

/**
 * @ingroup GenMod
 * @{
 */

/**
 * @class BresenInterpolation
 * @brief Discrete linear interpolation
 * @note Time-critical
 * @note Please note that all methods are inline.
 * @details
 * Thanks to Mr. Bresenham for giving me the idea for this discrete linear interpolation algorithm... @n
 * Instead of using an interpolation formula like @code dy*n/dx @endcode this is an incremental
 * algorithm that uses only additions and substractions instead of divisions and multiplications, it's a
 * heavy speed-up.
 */
class PPPLAY_CORE_EXPORT BresenInterpolation
{
	BresenInterpolation() = delete;
private:
	//! @brief Width of the line
	int_fast32_t m_dx;
	//! @brief Height of the line
	int_fast32_t m_dy;
	//! @brief Error variable (or fractional part). Range is [0, m_dx-1]
	int_fast32_t m_err;
	std::streamoff m_position;
public:
	static constexpr std::streamoff InvalidPosition = std::numeric_limits<std::streamoff>::max();
	
	/**
	 * @brief Constructor
	 * @param[in] dx Width of the interpolation line
	 * @param[in] dy Height of the interpolation line
	 * @pre dx>1
	 * @pre dy>0
	 */
	constexpr BresenInterpolation( int dx, int dy ) noexcept :
		m_dx( dx ),
		m_dy( dy ),
		m_err( dx-1 ),
		m_position(0)
	{
	}
	
	inline operator std::streamoff() const noexcept {
		return m_position;
	}
	
	inline BresenInterpolation& operator=(uint_fast32_t val) noexcept {
		m_position = val;
		return *this;
	}
	
	/**
	 * @brief Calculates the next interpolation step
	 * @param[in,out] pos Interpolation Y point to adjust
	 * @post 0 <= m_err < m_dx
	 */
	inline uint_fast32_t next() {
		BOOST_ASSERT(m_dx>0 && m_dy>=0 && m_err>=0 && m_err<m_dx);
		for( m_err -= m_dy; m_err < 0; m_err += m_dx ) {
			m_position++;
		}
		return m_position;
	}
	
	BresenInterpolation& operator++() {
		next();
		return *this;
	}

	/**
	 * @brief Sets width and height of the interpolation line
	 * @param[in] dx New value for m_dx
	 * @param[in] dy New value for m_dy
	 */
	inline void reset( int dx, int dy ) {
		m_dx = dx;
		m_dy = dy;
		// m_err = dx-1;
		BOOST_ASSERT(dx>0 && dy>=0);
	}
	
	/**
	 * @brief Get the normalized fractional part
	 * @return Fractional part, normalized to be within 0 and 255
	 */
	inline int bias() const
	{
		BOOST_ASSERT( m_err>=0 && m_err<m_dx );
		return (m_err<<8)/m_dx;
	}
	
	/**
	 * @brief Mix two values using the fractional part m_err
	 * @return Mixed value
	 */
	inline int16_t biased(int16_t v1, int16_t v2) const noexcept
	{
		int v1b = v1*m_err;
		int v2b = v2*(m_dx-m_err);
		return ppp::clip<int>((v1b+v2b)/m_dx, -32768, 32767);
	}
	
	inline BasicSampleFrame biased(const BasicSampleFrame& a, const BasicSampleFrame& b) const noexcept
	{
		return BasicSampleFrame(
			biased(a.left, b.left),
			biased(a.right, b.right)
		);
	}
	
	inline bool isValid() const noexcept
	{
		return m_position != InvalidPosition;
	}
};

/**
 * @}
 */

}

#endif // breseninterH
