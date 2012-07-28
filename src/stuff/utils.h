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

#include <boost/checked_delete.hpp>

/**
 * @ingroup Common
 * @{
 */

/**
 * @def DISABLE_COPY(classname)
 * @brief Disables copy operator and constructor of @a classname
 * @param[in] classname Class name to disable copy functions of
 */
#define DISABLE_COPY(classname) \
	classname(const classname&) = delete; \
	classname& operator=(const classname&) = delete;

template<class T>
inline void deleteAll( T& container )
{
	for( auto & val : container ) {
		boost::checked_delete(val);
		val = nullptr;
	}
}

/**
 * @}
 */

#endif
