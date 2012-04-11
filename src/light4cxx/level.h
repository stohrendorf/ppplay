/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef LEVEL_H
#define LEVEL_H

namespace light4cxx
{

/**
 * @ingroup light4cxx
 * @{
 */

/**
 * @brief Logging levels
 * @see light4cxx::Logger
 */
enum class Level
{
	All, //!< @brief Everything will be logged
	Trace, //!< @brief Trace messages and less verbose ones will be logged
	Debug, //!< @brief Debug messages and less verbose ones will be logged
	Info, //!< @brief Informational messages and less verbose ones will be logged
	Warn, //!< @brief Warning messages and less verbose ones will be logged
	Error, //!< @brief Error messages and less verbose ones will be logged
	Fatal, //!< @brief Only fatal messages will be logged
	Off //!< @brief Nothing will be logged
};

/**
 * @}
 */

}

#endif

