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

#include <genmod/ppplay_module_base_export.h>

#include <array>
#include <cstdint>

namespace ppp
{

/**
 * @ingroup GenMod
 * @{
 */

/**
 * @brief General note names
 */
extern PPPLAY_MODULE_BASE_EXPORT const std::array<const char*, 12> NoteNames;

/**
 * @class RememberByte
 * @brief A single byte refusing assigning zero
 * @tparam TSplitNibbles Set to @c true to handle each nibble separately.
 */
template<bool TSplitNibbles>
class RememberByte
{
private:
    //! @brief Value of the byte
    uint8_t m_value;

public:
    //! @brief Template parameter access
    static constexpr bool SplitNibbles = TSplitNibbles;

    /**
     * @brief Constructor
     * @param[in] val Initial value
     */
    explicit constexpr RememberByte( uint8_t val = 0 ) noexcept : m_value( val ) {
    }

    /**
     * @brief Implicit cast operator, ease usage
     * @return m_value
     */
    constexpr operator uint8_t() const noexcept {
        return m_value;
    }

    /**
     * @brief Assignment operator
     * @param[in] val Value to assign, no-op when @c val==0
     * @return Reference to @c this
     * @see force()
     * @see noNibbles()
     */
    inline RememberByte<TSplitNibbles>& operator=( uint8_t val ) noexcept;

    /**
     * @brief Assignment function, forces m_value to be set
     * @param[in] val Value to assign
     * @see operator=()
     * @see noNibbles()
     */
    void force( uint8_t val ) noexcept {
        m_value = val;
    }

    /**
     * @brief Like operator=(), but always ignores nibble separation
     * @param[in] val Value to assign
     * @see operator=()
     * @see force()
     */
    void noNibbles( uint8_t val ) noexcept {
        if( val != 0 ) {
            m_value = val;
        }
    }

    /**
     * @brief High nibble of the value
     * @return m_value>>4
     */
    constexpr uint8_t hi() const noexcept {
        return m_value >> 4;
    }

    /**
     * @brief Low nibble of the value
     * @return m_value&0x0f
     */
    constexpr uint8_t lo() const noexcept {
        return m_value & 0x0f;
    }
};

template<>
inline RememberByte<true>& RememberByte<true>::operator=( uint8_t val ) noexcept {
    if( ( val & 0x0f ) != 0 ) {
        m_value = ( m_value & 0xf0 ) | ( val & 0x0f );
    }
    if( ( val & 0xf0 ) != 0 ) {
        m_value = ( m_value & 0x0f ) | ( val & 0xf0 );
    }
    return *this;
}

template<>
inline RememberByte<false>& RememberByte<false>::operator=( uint8_t val ) noexcept {
    if( val != 0 ) {
        m_value = val;
    }
    return *this;
}

/**
 * @}
 */

} // namespace ppp
