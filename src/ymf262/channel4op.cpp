#include "channel4op.h"

#include "opl3.h"

namespace opl
{
std::vector< int16_t > Channel4Op::nextSample()
{

	const int secondChannelBaseAddress = baseAddress() + 3;
	const int secondCnt = opl()->readReg( secondChannelBaseAddress + AbstractChannel::CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset ) & 0x1;
	const int cnt4op = ( cnt() << 1 ) | secondCnt;

	int channelOutput = 0;
	const int feedbackOutput = avgFeedback();

	/*
	 * Below: "@" means feedback, "~>" means "modulates"
	 */
	switch( cnt4op ) {
		case 0:
			/*
			 * @Op1 ~> Op2 ~> Op3 ~> Op4
			 */
			if( m_op4->envelopeGenerator()->isOff() )
				return getInFourChannels( 0 );

			channelOutput = m_op1->nextSample( feedbackOutput );
			pushFeedback(channelOutput);
			
			channelOutput = m_op4->nextSample( m_op3->nextSample( m_op2->nextSample( channelOutput ) ) );

			break;
		case 1:
			/*
			 * (@Op1 ~> Op2) + (Op3 ~> Op4)
			 */
			if( m_op2->envelopeGenerator()->isOff() && m_op4->envelopeGenerator()->isOff() ) {
				return getInFourChannels( 0 );
			}

			channelOutput = m_op1->nextSample( feedbackOutput );
			pushFeedback(channelOutput);
			
			channelOutput = m_op2->nextSample( channelOutput );
			channelOutput += m_op4->nextSample( m_op3->nextSample( Operator::noModulator ) );
			break;
		case 2:
			/*
			 * @Op1 + (Op2 ~> Op3 ~> Op4)
			 */
			if( m_op1->envelopeGenerator()->isOff() && m_op4->envelopeGenerator()->isOff() )
				return getInFourChannels( 0 );

			channelOutput = m_op1->nextSample( feedbackOutput );
			pushFeedback(channelOutput);
			
			channelOutput += m_op4->nextSample( m_op3->nextSample( m_op2->nextSample( Operator::noModulator ) ) );
			break;
		case 3:
			/*
			 * (@Op1 ~> Op3) + Op2 + Op4
			 */
			if( m_op1->envelopeGenerator()->isOff() && m_op3->envelopeGenerator()->isOff() && m_op4->envelopeGenerator()->isOff() )
				return getInFourChannels( 0 );

			channelOutput = m_op1->nextSample( feedbackOutput );
			pushFeedback(channelOutput);
			
			channelOutput += m_op3->nextSample( m_op2->nextSample( Operator::noModulator ) );
			channelOutput += m_op4->nextSample( Operator::noModulator );
	}

	return getInFourChannels( channelOutput );
}
void Channel4Op::keyOn()
{
	m_op1->keyOn();
	m_op2->keyOn();
	m_op3->keyOn();
	m_op4->keyOn();
	clearFeedback();
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
