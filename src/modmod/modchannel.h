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

#ifndef MODCHANNEL_H
#define MODCHANNEL_H

/**
 * @ingroup ModMod
 * @{
 */
#include "genmod/genchannel.h"

#include "modcell.h"

namespace ppp {
namespace mod {

class ModChannel : public GenChannel
{
	DISABLE_COPY(ModChannel)
private:
	ModCell m_currentCell;
public:
	typedef std::shared_ptr<ModChannel> Ptr; //!< @brief Class pointer
	typedef std::vector<Ptr> Vector; //!< @brief Vector of class pointers
	ModChannel();
	virtual ~ModChannel();
	virtual std::string noteName();
	virtual std::string effectName() const;
	virtual void mixTick(MixerFrameBuffer& mixBuffer);
	virtual void simTick(size_t bufsize);
	virtual void updateStatus();
	virtual std::string effectDescription() const;
	virtual std::string cellString();
	virtual IArchive& serialize(IArchive* data);
};

}
}

/**
 * @}
 */

#endif
