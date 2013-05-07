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

#ifndef PPP_OPL_DISABLEDCHANNEL_H
#define PPP_OPL_DISABLEDCHANNEL_H
#include "abstractchannel.h"

namespace opl
{

class Opl3;
class DisabledChannel : public AbstractChannel
{
public:
	DisabledChannel(Opl3* opl) : AbstractChannel( opl, 0, {} ) {
	}
	void nextSample(std::array<int16_t,4>* dest) {
		getInFourChannels( dest, 0 );
	}

    virtual AbstractArchive& serialize(AbstractArchive* archive) {
		return AbstractChannel::serialize(archive);
	}
};
}

#endif
