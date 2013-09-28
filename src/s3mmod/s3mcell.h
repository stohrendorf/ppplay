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

#ifndef S3MCELL_H
#define S3MCELL_H

/**
 * @ingroup S3mMod
 * @{
 */

#include "genmod/ipatterncell.h"

class Stream;

namespace ppp
{
namespace s3m
{

/**
 * @class S3mCell
 * @brief A ScreamTracker cell
 */
class S3mCell : public IPatternCell
{
private:
    uint8_t m_note; //!< @brief Note value
    uint8_t m_instr; //!< @brief Instrument value
    uint8_t m_volume; //!< @brief Volume value
    uint8_t m_effect; //!< @brief Effect
    uint8_t m_effectValue; //!< @brief Effect value
public:
    S3mCell();
    /**
     * @brief Load this cell from a stream
     * @param[in,out] str Reference to the stream to load from
     * @retval true on success
     * @retval false if an error occured
     */
    bool load( Stream* str );
    virtual void clear();
    virtual std::string trackerString() const;
    /**
     * @brief Get the cell's note
     * @return m_note
     */
    uint8_t note() const;
    /**
     * @brief Get the cell's instrument
     * @return m_instr
     */
    uint8_t instrument() const;
    /**
     * @brief Get the cell's volume
     * @return m_volume
     */
    uint8_t volume() const;
    /**
     * @brief Get the cell's effect
     * @return m_effect
     */
    uint8_t effect() const;
    /**
     * @brief Get the cell's effect value
     * @return m_effectValue
     */
    uint8_t effectValue() const;
    virtual AbstractArchive& serialize( AbstractArchive* data );
protected:
    /**
     * @brief Get the logger
     * @return Child logger with attached ".s3m"
     */
    static light4cxx::Logger* logger();
};

}
}

/**
 * @}
 */

#endif
