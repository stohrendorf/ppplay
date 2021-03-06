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
 * @brief Some sort of "scaled increment"
 * @details
 * Thanks to Mr. Bresenham for giving me the idea for this discrete linear interpolation algorithm... @n
 * Instead of using an interpolation formula like @code dy*n/dx @endcode this is an incremental
 * algorithm that uses only additions and substractions instead of divisions and multiplications, it's a
 * heavy speed-up.
 */
template<typename T = int_fast32_t>
class StepperBase
{
private:
    //! @brief Width of the line
    uint_fast32_t m_denominator;
    //! @brief Height of the line
    int_fast32_t m_nominator;
    //! @brief Error variable (or fractional part). Range is [0, m_denominator-1]
    int_fast32_t m_fractionPart{0};
    T m_intPart{0};
public:
    StepperBase() = delete;

    /**
     * @brief Constructor
     * @param[in] denominator Width of the interpolation line
     * @param[in] nominator Height of the interpolation line
     * @pre denominator>1
     * @pre nominator>0
     */
    constexpr StepperBase(uint_fast32_t denominator, int_fast32_t nominator) noexcept
        : m_denominator{denominator}, m_nominator{nominator}
    {
        BOOST_ASSERT(m_denominator > 0);
    }

    constexpr operator T() const noexcept
    {
        return m_intPart;
    }

    constexpr StepperBase<T>& operator=(T val) noexcept
    {
        m_intPart = val;
        m_fractionPart = 0;
        return *this;
    }

    /**
     * @brief Calculates the next interpolation step
     * @param[in,out] pos Interpolation Y point to adjust
     * @post 0 <= m_fraction < m_denominator
     */
    constexpr T next()
    {
        BOOST_ASSERT(m_denominator > 0 && m_fractionPart >= 0 && static_cast<uint_fast32_t>(m_fractionPart) < m_denominator);
        if( m_nominator > 0 )
        {
            for( m_fractionPart += m_nominator; m_fractionPart >= static_cast<int_fast32_t>(m_denominator); m_fractionPart -= m_denominator )
            {
                ++m_intPart;
            }
        }
        else
        {
            for( m_fractionPart += m_nominator; m_fractionPart < 0; m_fractionPart += m_denominator )
            {
                --m_intPart;
            }
        }
        BOOST_ASSERT(m_fractionPart >= 0 && static_cast<uint_fast32_t>(m_fractionPart) < m_denominator);
        return m_intPart;
    }

    constexpr T prev()
    {
        BOOST_ASSERT(m_denominator > 0 && m_fractionPart >= 0 && static_cast<uint_fast32_t>(m_fractionPart) < m_denominator);
        if( m_nominator > 0 )
        {
            for( m_fractionPart -= m_nominator; m_fractionPart < 0; m_fractionPart += m_denominator )
            {
                --m_intPart;
            }
        }
        else
        {
            for( m_fractionPart -= m_nominator; m_fractionPart >= static_cast<int_fast32_t>(m_denominator); m_fractionPart -= m_denominator )
            {
                ++m_intPart;
            }
        }
        BOOST_ASSERT(m_fractionPart >= 0 && static_cast<uint_fast32_t>(m_fractionPart) < m_denominator);
        return m_intPart;
    }

    constexpr StepperBase<T>& operator++()
    {
        next();
        return *this;
    }

    constexpr StepperBase<T>& operator--()
    {
        prev();
        return *this;
    }

    /**
     * @brief Sets width and height of the interpolation line
     * @param[in] denominator New value for m_denominator
     * @param[in] nominator New value for m_nominator
     */
    constexpr void setStepSize(uint_fast32_t denominator, int_fast32_t nominator)
    {
        BOOST_ASSERT(denominator > 0);
        m_fractionPart = m_fractionPart * denominator / m_denominator;
        m_denominator = denominator;
        m_nominator = nominator;
        BOOST_ASSERT(m_fractionPart >= 0 && static_cast<uint_fast32_t>(m_fractionPart) < m_denominator);
    }

    constexpr float floatFraction() const
    {
        BOOST_ASSERT(m_fractionPart >= 0 && static_cast<uint_fast32_t>(m_fractionPart) < m_denominator);
        return float(m_fractionPart) / m_denominator;
    }

    /**
     * @brief Mix two values using the fractional part m_fraction
     * @return Mixed value
     */
    constexpr int16_t biased(int16_t v1, int16_t v2) const noexcept
    {
        float b = floatFraction();
        auto v1b = v1 * b;
        auto v2b = v2 * (1 - b);
        return ppp::clip<int>(v1b + v2b, -32768, 32767);
    }

    constexpr BasicSampleFrame biased(const BasicSampleFrame& a, const BasicSampleFrame& b) const noexcept
    {
        return {
            biased(a.left, b.left),
            biased(a.right, b.right)
        };
    }
};

using Stepper = StepperBase<int_fast32_t>;

/**
 * @}
 */

}
