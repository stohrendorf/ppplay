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
 * @brief Discrete linear interpolation
 * @note Time-critical
 * @note Please note that all methods are inline.
 * @details
 * Thanks to Mr. Bresenham for giving me the idea for this discrete linear interpolation algorithm... @n
 * Instead of using an interpolation formula like @code dy*n/dx @endcode this is an incremental
 * algorithm that uses only additions and substractions instead of divisions and multiplications, it's a
 * heavy speed-up.
 */
class PPPLAY_CORE_EXPORT Stepper
{
private:
    //! @brief Width of the line
    uint_fast32_t m_denominator;
    //! @brief Height of the line
    uint_fast32_t m_nominator;
    //! @brief Error variable (or fractional part). Range is [0, m_denominator-1]
    int_fast32_t m_fraction;
    int_fast32_t m_position;
public:
    Stepper() = delete;

    /**
     * @brief Constructor
     * @param[in] denominator Width of the interpolation line
     * @param[in] nominator Height of the interpolation line
     * @pre denominator>1
     * @pre nominator>0
     */
    constexpr Stepper(uint_fast32_t denominator, uint_fast32_t nominator) noexcept
        : m_denominator(denominator), m_nominator(nominator), m_fraction(denominator - 1), m_position(0)
    {
    }

    inline operator int_fast32_t() const noexcept
    {
        return m_position;
    }

    inline Stepper& operator=(int_fast32_t val) noexcept
    {
        m_position = val;
        return *this;
    }

    /**
     * @brief Calculates the next interpolation step
     * @param[in,out] pos Interpolation Y point to adjust
     * @post 0 <= m_fraction < m_denominator
     */
    inline int_fast32_t next()
    {
        BOOST_ASSERT(m_denominator > 0 && m_fraction >= 0 && static_cast<uint_fast32_t>(m_fraction) < m_denominator);
        for( m_fraction -= m_nominator; m_fraction < 0; m_fraction += m_denominator )
        {
            m_position++;
        }
        BOOST_ASSERT(m_fraction >= 0 && static_cast<uint_fast32_t>(m_fraction) < m_denominator);
        return static_cast<int_fast32_t>(m_position);
    }

    inline int_fast32_t prev()
    {
        BOOST_ASSERT(m_denominator > 0 && m_fraction >= 0 && static_cast<uint_fast32_t>(m_fraction) < m_denominator);
        for( m_fraction += m_nominator; m_fraction >= static_cast<int_fast32_t>(m_denominator); m_fraction -= m_denominator )
        {
            m_position--;
        }
        BOOST_ASSERT(m_fraction >= 0 && static_cast<uint_fast32_t>(m_fraction) < m_denominator);
        return static_cast<int_fast32_t>(m_position);
    }

    Stepper& operator++()
    {
        next();
        return *this;
    }

    Stepper& operator--()
    {
        prev();
        return *this;
    }

    /**
     * @brief Sets width and height of the interpolation line
     * @param[in] denominator New value for m_denominator
     * @param[in] nominator New value for m_nominator
     */
    inline void reset(uint_fast32_t denominator, uint_fast32_t nominator)
    {
        BOOST_ASSERT(denominator > 0);
        m_fraction = m_fraction * m_denominator / denominator;
        m_denominator = denominator;
        m_nominator = nominator;
        // m_fraction = denominator-1;
        BOOST_ASSERT(m_fraction >= 0 && static_cast<uint_fast32_t>(m_fraction) < m_denominator);
    }

    /**
     * @brief Get the normalized fractional part
     * @return Fractional part, normalized to be within 0 and 255
     */
    inline uint_fast32_t stepSize() const
    {
        BOOST_ASSERT(m_fraction >= 0 && static_cast<uint_fast32_t>(m_fraction) < m_denominator);
        return (m_fraction * 256) / m_denominator;
    }

    inline float floatStepSize() const
    {
        BOOST_ASSERT(m_fraction >= 0 && static_cast<uint_fast32_t>(m_fraction) < m_denominator);
        return float(m_fraction) / m_denominator;
    }

    /**
     * @brief Mix two values using the fractional part m_fraction
     * @return Mixed value
     */
    inline int16_t biased(int16_t v1, int16_t v2) const noexcept
    {
        int_fast32_t v1b = v1 * m_fraction;
        int_fast32_t v2b = v2 * (m_denominator - m_fraction);
        return ppp::clip<int>((v1b + v2b) / m_denominator, -32768, 32767);
    }

    inline BasicSampleFrame biased(const BasicSampleFrame& a, const BasicSampleFrame& b) const noexcept
    {
        return {
            biased(a.left, b.left),
            biased(a.right, b.right)
        };
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
