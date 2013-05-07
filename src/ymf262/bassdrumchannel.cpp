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

#include "bassdrumchannel.h"
#include "opl3.h"

namespace opl
{
BassDrumChannel::BassDrumChannel(Opl3* opl): Channel2Op(opl, bassDrumChannelBaseAddress, opl->bassDrumOp1(), opl->bassDrumOp2())
{

}

void BassDrumChannel::nextSample( std::array< int16_t, 4 >* dest )
{
	// Bass Drum ignores first operator, when it is in series.
	if( cnt() ) {
		op(1)->setAr(0);
	}
	Channel2Op::nextSample(dest);
}

}
