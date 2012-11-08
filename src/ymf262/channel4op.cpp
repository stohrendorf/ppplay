#include "channel4op.h"

#include "opl3.h"

namespace opl
{
std::vector< int16_t > Channel4Op::nextSample()
{
	int channelOutput = 0, op1Output = 0, op2Output = 0, op3Output = 0, op4Output = 0;

	const int secondChannelBaseAddress = baseAddress() + 3;
	const int secondCnt = opl()->readReg( secondChannelBaseAddress + AbstractChannel::CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset ) & 0x1;
	const int cnt4op = ( cnt() << 1 ) | secondCnt;

	const int feedbackOutput = avgFeedback()>>2;

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

			op1Output = m_op1->nextSample( feedbackOutput );
			op2Output = m_op2->nextSample( op1Output );
			op3Output = m_op3->nextSample( op2Output );
			channelOutput = m_op4->nextSample( op3Output );

			break;
		case 1:
			/*
			 * (@Op1 ~> Op2) + (Op3 ~> Op4)
			 */
			if( m_op2->envelopeGenerator()->isOff() && m_op4->envelopeGenerator()->isOff() ) {
				return getInFourChannels( 0 );
			}

			op1Output = m_op1->nextSample( feedbackOutput );
			op2Output = m_op2->nextSample( op1Output );

			op3Output = m_op3->nextSample( Operator::noModulator );
			op4Output = m_op4->nextSample( op3Output );

			channelOutput = ( op2Output + op4Output ) / 2;
			break;
		case 2:
			/*
			 * @Op1 + (Op2 ~> Op3 ~> Op4)
			 */
			if( m_op1->envelopeGenerator()->isOff() && m_op4->envelopeGenerator()->isOff() )
				return getInFourChannels( 0 );

			op1Output = m_op1->nextSample( feedbackOutput );

			op2Output = m_op2->nextSample( Operator::noModulator );
			op3Output = m_op3->nextSample( op2Output );
			op4Output = m_op4->nextSample( op3Output );

			channelOutput = ( op1Output + op4Output ) / 2;
			break;
		case 3:
			/*
			 * (@Op1 ~> Op3) + Op2 + Op4
			 */
			if( m_op1->envelopeGenerator()->isOff() && m_op3->envelopeGenerator()->isOff() && m_op4->envelopeGenerator()->isOff() )
				return getInFourChannels( 0 );

			op1Output = m_op1->nextSample( feedbackOutput );

			op2Output = m_op2->nextSample( Operator::noModulator );
			op3Output = m_op3->nextSample( op2Output );

			op4Output = m_op4->nextSample( Operator::noModulator );

			channelOutput = ( op1Output + op3Output + op4Output ) / 3;
	}

	if( fb() != 0 ) {
		pushFeedback( (op1Output >> AbstractChannel::FeedbackShift[fb()]) & 0xfff );
		// pushFeedback( std::fmod( op1Output * feedback[fb()], 1) );
	}
	else {
		pushFeedback(0);
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
