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

#include "genmod/gensample.h"

class BinStream;

namespace ppp {
namespace mod {

class ModSample : public GenSample
{
	DISABLE_COPY(ModSample)
public:
	typedef std::shared_ptr<ModSample> Ptr; //!< @brief Class pointer
	typedef std::vector<Ptr> Vector; //!< @brief Vector of class pointers
	ModSample();
	bool loadHeader(BinStream& stream);
	bool loadData(BinStream& stream);
	int8_t finetune() const;
private:
	int8_t m_finetune;
};

}
}

/**
 * @}
 */

#endif
