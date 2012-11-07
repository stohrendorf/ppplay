#include "channel2op.h"
#include "operator.h"
#include "opl3.h"

namespace opl
{
std::vector< int16_t > Channel2Op::nextSample()
{
	int16_t channelOutput = 0, op1Output = 0, op2Output = 0;
	// The feedback uses the last two outputs from
	// the first operator, instead of just the last one.
	const int feedbackOutput = avgFeedback()>>2;

	if( !cnt() ) {
		// CNT = 0, the operators are in series, with the first in feedback.
		if( m_op2->envelopeGenerator()->isOff() )
			return getInFourChannels( 0 );
		op1Output = m_op1->nextSample( feedbackOutput );
		channelOutput = m_op2->nextSample( op1Output );
	}
	else {
		// CNT = 1, the operators are in parallel, with the first in feedback.
		if( m_op1->envelopeGenerator()->isOff() && m_op2->envelopeGenerator()->isOff() ) {
			return getInFourChannels( 0 );
		}
		op1Output = m_op1->nextSample( feedbackOutput );
		op2Output = m_op2->nextSample( Operator::noModulator );
		channelOutput = ( op1Output + op2Output ) >> 1;
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
void Channel2Op::keyOn()
{
	m_op1->keyOn();
	m_op2->keyOn();
	clearFeedback();
}
void Channel2Op::keyOff()
{
	m_op1->keyOff();
	m_op2->keyOff();
}
void Channel2Op::updateOperators()
{
	uint16_t f_number = ( fnumh() << 8 ) | fnuml();
	m_op1->updateOperator( f_number, block() );
	m_op2->updateOperator( f_number, block() );
}
light4cxx::Logger* Channel2Op::logger()
{
	return light4cxx::Logger::get("opl.channel2op");
}
}
