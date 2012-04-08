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

#ifndef XMSAMPLE_H
#define XMSAMPLE_H

#include "genmod/gensample.h"

class BinStream;

/**
 * @ingroup XmModule
 * @{
 */

namespace ppp
{
namespace xm
{

/**
 * @class XmSample
 * @brief XM Sample storage class
 */
class XmSample : public GenSample
{
	DISABLE_COPY( XmSample )
private:
	//! @brief Sample finetune
	int8_t m_finetune;
	//! @brief Default panning
	uint8_t m_panning;
	//! @brief Relative note
	int8_t m_relativeNote;
	//! @brief Whether the sample is a 16-bit one
	bool m_16bit;
public:
	//! @brief Default constructor
	XmSample();
	/**
	 * @brief Loads the sample header
	 * @param[in] str The stream to load from
	 * @return @c true on success
	 */
	bool load( BinStream& str );
	/**
	 * @brief Loads the sample data
	 * @param[in] str The stream to load from
	 * @return @c true on success
	 * @pre Make sure you have called load(BinStream&) previously
	 */
	bool loadData( BinStream& str );
	/**
	 * @brief Get the default finetune
	 * @return The default finetune
	 */
	int8_t finetune() const;
	/**
	 * @brief Get the default panning
	 * @return The default panning
	 */
	uint8_t panning() const;
	/**
	 * @brief Get the relative note offset
	 * @return The relative note
	 */
	int8_t relativeNote() const;
	/**
	 * @brief Whether the sample is a 16-bit one
	 * @return @c true when the sample is a 16-bit one, else @c false when it's an 8-bit one
	 */
	bool is16bit() const;
};

}
}

/**
 * @}
 */

#endif // XMSAMPLE_H
