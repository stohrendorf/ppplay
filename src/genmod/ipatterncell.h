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

#include <stream/iserializable.h>
#include <light4cxx/logger.h>

namespace ppp
{

/**
 * @ingroup GenMod
 * @{
 */

/**
 * @interface IPatternCell
 * @ingroup GenMod
 * @brief General interface for pattern note cells
 */
class IPatternCell
    : public ISerializable
{
public:
    /**
     * @brief Destructor
     */
    ~IPatternCell() noexcept override = default;

    /**
     * @brief Clears the cell's data
     */
    virtual void clear() = 0;

    /**
     * @brief Get the tracker-like string representation of this cell
     * @return Tracker-like string
     */
    virtual std::string trackerString() const = 0;

protected:
    /**
     * @brief Get the logger
     * @return Logger with name "cell"
     */
    static light4cxx::Logger* logger();
};

/**
 * @}
 */

} // namespace ppp
