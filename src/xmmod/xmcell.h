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

#ifndef XMCELL_H
#define XMCELL_H

/**
 * @ingroup XmModule
 * @{
 */

#include "genmod/ipatterncell.h"
#include "xmbase.h"

class BinStream;

namespace ppp
{
namespace xm
{

/**
 * @class XmCell
 * @brief Cell of a pattern
 */
class XmCell : public IPatternCell
{
private:
	uint8_t m_note; //!< @brief Note value
	uint8_t m_instr; //!< @brief Instrument value
	uint8_t m_volume; //!< @brief Volume value
	Effect m_effect; //!< @brief Effect
	uint8_t m_effectValue; //!< @brief Effect value
public:
	XmCell();
	virtual ~XmCell();
	/**
	 * @brief Load the cell data from a stream
	 * @param[in] str The stream to load from
	 * @return @c true on success
	 */
	bool load( BinStream& str );
	virtual void clear();
	virtual std::string trackerString() const;
	/**
	 * @brief Get the note string, e.g. in the form "C#3"
	 * @return The note string
	 */
	std::string noteString() const;
	/**
	 * @brief Get the string representation of the effect/value columns
	 * @return String in the form of e.g. "R0A"
	 */
	std::string fxString() const;
	/**
	 * @brief Get the note value
	 * @return m_note
	 */
	uint8_t note() const;
	/**
	 * @brief Get the instrument value
	 * @return m_instrument
	 */
	uint8_t instrument() const;
	/**
	 * @brief Get the volume value
	 * @return m_volume
	 */
	uint8_t volume() const;
	/**
	 * @brief Get the effect
	 * @return m_effect
	 */
	Effect effect() const;
	/**
	 * @brief Get the effect value
	 * @return m_effectValue
	 */
	uint8_t effectValue() const;
	virtual IArchive& serialize( IArchive* data );
};

}
}

/**
 * @}
 */

#endif
