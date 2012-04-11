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

#ifndef MODULEMETAINFO_H
#define MODULEMETAINFO_H

#include <string>

namespace ppp
{

/**
 * @ingroup GenMod
 * @{
 */

/**
 * @class ModuleMetaInfo
 * @brief Meta information about a module
 */
struct ModuleMetaInfo
{
	explicit ModuleMetaInfo() : filename(), title(), trackerInfo()
	{
	}
	
	//! @brief Filename of the module
	std::string filename;
	//! @brief Title of the module
	std::string title;
	//! @brief Tracker information (Name and Version)
	std::string trackerInfo;
};

/**
 * @}
 */

}

#endif
