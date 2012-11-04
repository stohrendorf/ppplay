#include "channel2op.h"
#include "operator.h"
#include "opl3.h"

namespace opl
{
std::vector< double > Channel2Op::nextSample()
{
	double channelOutput = 0, op1Output = 0, op2Output = 0;
	std::vector<double> output;
	// The feedback uses the last two outputs from
	// the first operator, instead of just the last one.
	const int feedbackOutput = avgFeedback()*1024;


	switch( cnt() ) {
			// CNT = 0, the operators are in series, with the first in feedback.
		case 0:
			if( m_op2->envelopeGenerator()->isOff() )
				return getInFourChannels( 0 );
			op1Output = m_op1->nextSample( feedbackOutput );
			channelOutput = m_op2->nextSample( op1Output * toPhase * 1024 );
			break;
			// CNT = 1, the operators are in parallel, with the first in feedback.
		case 1:
			if( m_op1->envelopeGenerator()->isOff() && m_op2->envelopeGenerator()->isOff() )
				return getInFourChannels( 0 );
			op1Output = m_op1->nextSample( feedbackOutput );
			op2Output = m_op2->nextSample( Operator::noModulator );
			channelOutput = ( op1Output + op2Output ) / 2;
	}

	pushFeedback( std::fmod( op1Output * AbstractChannel::feedback[fb()], 1) );
	output = getInFourChannels( channelOutput );
	return output;
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
	int f_number = ( fnumh() << 8 ) | fnuml();
	m_op1->updateOperator( f_number, block() );
	m_op2->updateOperator( f_number, block() );
}
}
