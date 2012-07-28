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

#ifndef MODSAMPLE_H
#define MODSAMPLE_H

/**
 * @ingroup ModMod
 * @{
 */

#include "genmod/sample.h"

class Stream;

namespace ppp
{
namespace mod
{

class ModSample : public Sample
{
	DISABLE_COPY( ModSample )
public:
	ModSample();
	bool loadHeader( Stream* stream );
	bool loadData( Stream* stream );
	bool loadAdpcmData( Stream* stream );
	uint8_t finetune() const;
private:
	uint8_t m_finetune;
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
