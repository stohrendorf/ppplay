#include "channel2op.h"
#include "operator.h"
#include "opl3.h"

namespace opl
{
void Channel2Op::nextSample( std::array< int16_t, 4 >* dest )
{
	int16_t channelOutput(0);
	const uint16_t feedbackOutput = avgFeedback();

	if( !cnt() ) {
		// CNT = 0, the operators are in series, with the first in feedback.
		if( m_op2->envelopeGenerator()->isOff() ) {
			getInFourChannels( dest, 0 );
			return;
		}
		channelOutput = m_op1->nextSample( feedbackOutput );
		pushFeedback(channelOutput);
		channelOutput = m_op2->nextSample( channelOutput );
	}
	else {
		// CNT = 1, the operators are in parallel, with the first in feedback.
		if( m_op1->envelopeGenerator()->isOff() && m_op2->envelopeGenerator()->isOff() ) {
			getInFourChannels( dest, 0 );
			return;
		}
		channelOutput = m_op1->nextSample( feedbackOutput );
		pushFeedback(channelOutput);
		channelOutput += m_op2->nextSample( Operator::noModulator );
	}
	getInFourChannels( dest, channelOutput );
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
	m_op1->updateOperator( fnum(), block() );
	m_op2->updateOperator( fnum(), block() );
}

light4cxx::Logger* Channel2Op::logger()
{
	return light4cxx::Logger::get("opl.channel2op");
}

}
