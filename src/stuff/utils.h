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

#ifndef PPPLAY_UTILS_H
#define PPPLAY_UTILS_H

/**
 * @ingroup Common
 * @{
 */

/**
 * @def DISABLE_COPY(classname)
 * @brief Disables copy/move operators and constructors of @a classname
 * @param[in] classname Class name to disable copy functions of
 */
#define DISABLE_COPY(classname) \
    classname(const classname&) = delete; \
    classname(classname&&) = delete; \
    classname& operator=(const classname&) = delete; \
    classname& operator=(classname&&) = delete;

/**
 * @brief Checked delete for all elements in a container
 * @tparam T Container type
 * @param[in,out] container Reference to the container
 * @note All elements within @a container will be set to @c nullptr, but the size will not change.
 */
template<class T>
inline void deleteAll( T& container )
{
    static_assert(sizeof(**container.begin())>0, "Cannot delete an incomplete type");
    for( auto & val : container ) {
        val = nullptr;
    }
}

/**
 * @}
 */

#endif
