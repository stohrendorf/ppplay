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

#ifndef MODCELL_H
#define MODCELL_H

/**
 * @ingroup ModMod
 * @{
 */

#include "stream/stream.h"
#include "genmod/ipatterncell.h"

namespace ppp
{
namespace mod
{

class ModCell : public IPatternCell
{
public:
	ModCell();
	virtual ~ModCell();
	/**
	 * @brief Load this cell from a stream
	 * @param[in,out] str Reference to the stream to load from
	 * @retval true on success
	 * @retval false if an error occured
	 */
	bool load( Stream* str );
	virtual void clear();
	virtual std::string trackerString() const;
	uint8_t sampleNumber() const;
	uint16_t period() const;
	uint8_t effect() const;
	uint8_t effectValue() const;
	void reset();
	virtual AbstractArchive& serialize( AbstractArchive* data );
private:
	uint8_t m_sampleNumber;
	uint16_t m_period;
	uint8_t m_effect;
	uint8_t m_effectValue;
	std::string m_note;
protected:
	/**
	 * @brief Get the logger
	 * @return Child logger with attached ".mod"
	 */
	static light4cxx::Logger* logger();
};

}
}

/**
 * @}
 */

#endif
