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

#include "channel2op.h"
#include "operator.h"
#include "opl3.h"

namespace opl
{
void Channel2Op::nextSample( std::array< int16_t, 4 >* dest )
{
	if(isRhythmChannel()) {
		getInFourChannels( dest, op(1)->nextSample() + op(2)->nextSample() );
		return;
	}
	
	const uint16_t feedbackOutput = avgFeedback();
	int16_t channelOutput = op(1)->nextSample( feedbackOutput );
	pushFeedback(channelOutput);

	if( !cnt() ) {
		// CNT = 0, the operators are in series, with the first in feedback.
		channelOutput = op(2)->nextSample( channelOutput );
	}
	else {
		// CNT = 1, the operators are in parallel, with the first in feedback.
		channelOutput += op(2)->nextSample();
	}
	getInFourChannels( dest, channelOutput );
}

light4cxx::Logger* Channel2Op::logger()
{
	return light4cxx::Logger::get("opl.channel2op");
}

}
