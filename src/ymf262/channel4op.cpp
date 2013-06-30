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

#include "channel4op.h"

#include "opl3.h"

namespace opl
{
void Channel4Op::nextSample( std::array< int16_t, 4 >* dest )
{
	const int secondChannelBaseAddress = baseAddress() + 3;
	const bool secondCnt = opl()->readReg( secondChannelBaseAddress + AbstractChannel::CH4_FB3_CNT1_Offset ) & 0x1;

	const uint16_t feedbackOutput = avgFeedback();
	int16_t channelOutput = op(1)->nextSample( feedbackOutput );
	pushFeedback(channelOutput);

	/*
	 * Below: "~" means "modulates",
	 * F is first connection, S is second connection
	 * FS
	 * 00  Op1 ~ Op2  ~  Op3 ~ Op4
	 * 01 (Op1 ~ Op2) + (Op3 ~ Op4)
	 * 10  Op1 +(Op2  ~  Op3 ~ Op4)
	 * 11  Op1 +(Op2  ~  Op3)+ Op4
	 */
	if(cnt()) {
		int16_t tmp = op(3)->nextSample( op(2)->nextSample() );
		if(secondCnt) {
			channelOutput += op(4)->nextSample() + tmp;
		}
		else {
			channelOutput += op(4)->nextSample( tmp );
		}
	}
	else {
		channelOutput = op(2)->nextSample( channelOutput );
		if(secondCnt) {
			channelOutput += op(4)->nextSample( op(3)->nextSample() );
		}
		else {
			channelOutput = op(4)->nextSample( op(3)->nextSample( channelOutput ) );
		}
	}

	getInFourChannels( dest, channelOutput );
}
light4cxx::Logger* Channel4Op::logger()
{
	return light4cxx::Logger::get("opl.channel4op");
}
}
