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

#ifndef BRESENINTER_H
#define BRESENINTER_H

#include "stuff/utils.h"
#include "stream/iserializable.h"
#include "gensample.h"

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
class BresenInterpolation : public ISerializable
{
	DISABLE_COPY( BresenInterpolation )
	BresenInterpolation() = delete;
private:
	//! @brief Width of the line
	uint_fast32_t m_dx;
	//! @brief Height of the line
	uint_fast32_t m_dy;
	//! @brief Error variable. Range is [0, m_dx-1]
	int_fast32_t m_err;
public:
	/**
	 * @brief Constructor
	 * @param[in] dx Width of the interpolation line
	 * @param[in] dy Height of the interpolation line
	 * @post 0 <= m_err < m_dx
	 */
	constexpr BresenInterpolation( uint32_t dx, uint32_t dy ) :
		m_dx( dx ),
		m_dy( dy ),
		m_err( 0 )
	{
	}
	/**
	 * @brief Calculates the next interpolation step
	 * @param[in,out] pos Interpolation Y point to adjust
	 */
	inline void next( GenSample::PositionType& pos ) {
		for( m_err -= m_dy; m_err < 0; m_err += m_dx ) {
			pos++;
		}
	}
	/**
	 * @brief Sets width and height of the interpolation line
	 * @param[in] dx New value for m_dx
	 * @param[in] dy New value for m_dy
	 */
	inline void reset( uint32_t dx, uint32_t dy ) {
		m_dx = dx;
		m_dy = dy;
		m_err = 0;
	}
	
	inline int bias() const
	{
		return std::max<ulong>(0,std::min<ulong>(255,(m_err<<8)/m_dx));
	}
	
	inline int16_t biased(int16_t v1, int16_t v2) const
	{
		int b = bias();
		int v1b = v1*b;
		int v2b = v2*(255-b);
		return std::max(-32767,std::min(32767,(v1b+v2b)/255));
	}
	
	inline BasicSampleFrame biased(const BasicSampleFrame& a, const BasicSampleFrame& b) const
	{
		return BasicSampleFrame(
			biased(a.left, b.left),
			biased(a.right, b.right)
		);
	}
	
	virtual IArchive& serialize( IArchive* archive );
};

inline ppp::GenSample::PositionType operator+=(ppp::GenSample::PositionType& left, BresenInterpolation& right)
{
	right.next(left);
	return left;
}

/**
 * @}
 */

}

#endif // breseninterH
