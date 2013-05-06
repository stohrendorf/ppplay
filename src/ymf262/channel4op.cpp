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
	const int secondCnt = opl()->readReg( secondChannelBaseAddress + AbstractChannel::CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset ) & 0x1;
	const int cnt4op = ( cnt() << 1 ) | secondCnt;

	int16_t channelOutput(0);
	const uint16_t feedbackOutput = avgFeedback();

	/*
	 * Below: "@" means feedback, "~>" means "modulates"
	 */
	switch( cnt4op ) {
		case 0:
			/*
			 * @Op1 ~> Op2 ~> Op3 ~> Op4
			 */
			channelOutput = m_op1->nextSample( feedbackOutput );
			pushFeedback(channelOutput);
			
			channelOutput = m_op4->nextSample( m_op3->nextSample( m_op2->nextSample( channelOutput ) ) );

			break;
		case 1:
			/*
			 * (@Op1 ~> Op2) + (Op3 ~> Op4)
			 */
			channelOutput = m_op1->nextSample( feedbackOutput );
			pushFeedback(channelOutput);
			
			channelOutput = m_op2->nextSample( channelOutput );
			channelOutput += m_op4->nextSample( m_op3->nextSample( Operator::noModulator ) );
			break;
		case 2:
			/*
			 * @Op1 + (Op2 ~> Op3 ~> Op4)
			 */
			channelOutput = m_op1->nextSample( feedbackOutput );
			pushFeedback(channelOutput);
			
			channelOutput += m_op4->nextSample( m_op3->nextSample( m_op2->nextSample( Operator::noModulator ) ) );
			break;
		case 3:
			/*
			 * (@Op1 ~> Op3) + Op2 + Op4
			 */
			channelOutput = m_op1->nextSample( feedbackOutput );
			pushFeedback(channelOutput);
			
			channelOutput += m_op3->nextSample( m_op2->nextSample( Operator::noModulator ) );
			channelOutput += m_op4->nextSample( Operator::noModulator );
	}

	getInFourChannels( dest, channelOutput );
}
void Channel4Op::keyOn()
{
	m_op1->keyOn();
	m_op2->keyOn();
	m_op3->keyOn();
	m_op4->keyOn();
}
void Channel4Op::keyOff()
{
	m_op1->keyOff();
	m_op2->keyOff();
	m_op3->keyOff();
	m_op4->keyOff();
}
void Channel4Op::updateOperators()
{
	uint16_t f_number = fnum();
	m_op1->updateOperator( f_number, block() );
	m_op2->updateOperator( f_number, block() );
	m_op3->updateOperator( f_number, block() );
	m_op4->updateOperator( f_number, block() );
}
light4cxx::Logger* Channel4Op::logger()
{
	return light4cxx::Logger::get("opl.channel4op");
}
}
