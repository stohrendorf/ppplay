/*
 * PeePeePlayer - an old-fashioned module player
 * Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Original Java Code: Copyright (C) 2008 Robson Cozendey <robson@cozendey.com>
 * 
 * Some code based on forum posts in: http://forums.submarine.org.uk/phpBB/viewforum.php?f=9,
 * Copyright (C) 2010-2013 by carbon14 and opl3
 */

#ifndef PPP_OPL_CHANNEL4OP_H
#define PPP_OPL_CHANNEL4OP_H

#include <stuff/utils.h>

#include "abstractchannel.h"
#include <memory>

namespace opl
{

class Opl3;

class Operator;
class Channel4Op : public AbstractChannel
{
	DISABLE_COPY(Channel4Op)
private:
	static light4cxx::Logger* logger();

public:
	typedef std::shared_ptr<Channel4Op> Ptr;
	
	Channel4Op( Opl3* opl, int baseAddress, Operator* o1, Operator* o2, Operator* o3, Operator* o4 )
		: AbstractChannel( opl, baseAddress, {o1,o2,o3,o4} )
	{
	}

	void nextSample(std::array<int16_t,4>* dest);

    virtual AbstractArchive& serialize(AbstractArchive* archive) {
		return AbstractChannel::serialize(archive);
	}
};
}

#endif
