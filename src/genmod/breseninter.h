#pragma once

/*
    PPPlay - an old-fashioned module player
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

#include "ppplay_core_export.h"

#include <output/audiotypes.h>

#include <cstdint>
#include <boost/assert.hpp>

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
private:
    //! @brief Width of the line
    uint_fast32_t m_dx;
    //! @brief Height of the line
    uint_fast32_t m_dy;
    //! @brief Error variable (or fractional part). Range is [0, m_dx-1]
    int_fast32_t m_err;
    int_fast32_t m_position;
public:
    BresenInterpolation() = delete;

    static constexpr int_fast32_t InvalidPosition = std::numeric_limits<int_fast32_t>::max();

    /**
     * @brief Constructor
     * @param[in] dx Width of the interpolation line
     * @param[in] dy Height of the interpolation line
     * @pre dx>1
     * @pre dy>0
     */
    constexpr BresenInterpolation(uint_fast32_t dx, uint_fast32_t dy) noexcept
        : m_dx(dx), m_dy(dy), m_err(dx - 1), m_position(0)
    {
    }

    inline operator int_fast32_t() const noexcept
    {
        return m_position;
    }

    inline BresenInterpolation& operator=(int_fast32_t val) noexcept
    {
        m_position = val;
        return *this;
    }

    /**
     * @brief Calculates the next interpolation step
     * @param[in,out] pos Interpolation Y point to adjust
     * @post 0 <= m_err < m_dx
     */
    inline int_fast32_t next()
    {
        BOOST_ASSERT(m_dx > 0 && m_err >= 0 && static_cast<uint_fast32_t>(m_err) < m_dx);
        for( m_err -= m_dy; m_err < 0; m_err += m_dx )
        {
            m_position++;
        }
        BOOST_ASSERT(m_err >= 0 && static_cast<uint_fast32_t>(m_err) < m_dx);
        return static_cast<int_fast32_t>(m_position);
    }

    inline int_fast32_t prev()
    {
        BOOST_ASSERT(m_dx > 0 && m_err >= 0 && static_cast<uint_fast32_t>(m_err) < m_dx);
        for( m_err += m_dy; m_err >= static_cast<int_fast32_t>(m_dx); m_err -= m_dx )
        {
            m_position--;
        }
        BOOST_ASSERT(m_err >= 0 && static_cast<uint_fast32_t>(m_err) < m_dx);
        return static_cast<int_fast32_t>(m_position);
    }

    BresenInterpolation& operator++()
    {
        next();
        return *this;
    }

    BresenInterpolation& operator--()
    {
        prev();
        return *this;
    }

    /**
     * @brief Sets width and height of the interpolation line
     * @param[in] dx New value for m_dx
     * @param[in] dy New value for m_dy
     */
    inline void reset(uint_fast32_t dx, uint_fast32_t dy)
    {
        BOOST_ASSERT(dx > 0);
        m_err = m_err * m_dx / dx;
        m_dx = dx;
        m_dy = dy;
        // m_err = dx-1;
        BOOST_ASSERT(m_err >= 0 && static_cast<uint_fast32_t>(m_err) < m_dx);
    }

    /**
     * @brief Get the normalized fractional part
     * @return Fractional part, normalized to be within 0 and 255
     */
    inline uint_fast32_t stepSize() const
    {
        BOOST_ASSERT(m_err >= 0 && static_cast<uint_fast32_t>(m_err) < m_dx);
        return (m_err * 256) / m_dx;
    }

    inline float floatStepSize() const
    {
        BOOST_ASSERT(m_err >= 0 && static_cast<uint_fast32_t>(m_err) < m_dx);
        return float(m_err) / m_dx;
    }

    /**
     * @brief Mix two values using the fractional part m_err
     * @return Mixed value
     */
    inline int16_t biased(int16_t v1, int16_t v2) const noexcept
    {
        int_fast32_t v1b = v1 * m_err;
        int_fast32_t v2b = v2 * (m_dx - m_err);
        return ppp::clip<int>((v1b + v2b) / m_dx, -32768, 32767);
    }

    inline BasicSampleFrame biased(const BasicSampleFrame& a, const BasicSampleFrame& b) const noexcept
    {
        return {
            biased(a.left, b.left),
            biased(a.right, b.right)
        };
    }

    inline bool isValid() const noexcept
    {
        return m_position != InvalidPosition;
    }

    inline void setPosition(int_fast32_t p)
    {
        m_position = p;
    }
};

/**
 * @}
 */

}
