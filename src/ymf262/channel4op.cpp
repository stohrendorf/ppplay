#include "channel4op.h"

namespace opl
{
std::vector< double > Channel4Op::getChannelOutput()
{
	double channelOutput = 0,
		   op1Output = 0, op2Output = 0, op3Output = 0, op4Output = 0;

	std::vector<double> output;

	int secondChannelBaseAddress = m_channelBaseAddress + 3;
	int secondCnt = OPL3.registers[secondChannelBaseAddress + ChannelData.CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset] & 0x1;
	int cnt4op = ( cnt << 1 ) | secondCnt;

	double feedbackOutput = ( m_feedback[0] + m_feedback[1] ) / 2;

	switch( cnt4op ) {
		case 0:
			if( m_op4->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF )
				return getInFourChannels( 0 );

			op1Output = m_op1->getOperatorOutput( feedbackOutput );
			op2Output = m_op2->getOperatorOutput( op1Output * toPhase );
			op3Output = m_op3->getOperatorOutput( op2Output * toPhase );
			channelOutput = m_op4->getOperatorOutput( op3Output * toPhase );

			break;
		case 1:
			if( m_op2->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF &&
					m_op4->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF )
				return getInFourChannels( 0 );

			op1Output = m_op1->getOperatorOutput( feedbackOutput );
			op2Output = m_op2->getOperatorOutput( op1Output * toPhase );

			op3Output = m_op3->getOperatorOutput( Operator.noModulator );
			op4Output = m_op4->getOperatorOutput( op3Output * toPhase );

			channelOutput = ( op2Output + op4Output ) / 2;
			break;
		case 2:
			if( m_op1->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF &&
					m_op4->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF )
				return getInFourChannels( 0 );

			op1Output = m_op1->getOperatorOutput( feedbackOutput );

			op2Output = m_op2->getOperatorOutput( Operator.noModulator );
			op3Output = m_op3->getOperatorOutput( op2Output * toPhase );
			op4Output = m_op4->getOperatorOutput( op3Output * toPhase );

			channelOutput = ( op1Output + op4Output ) / 2;
			break;
		case 3:
			if( m_op1->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF &&
					m_op3->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF &&
					m_op4->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF )
				return getInFourChannels( 0 );

			op1Output = m_op1->getOperatorOutput( feedbackOutput );

			op2Output = m_op2->getOperatorOutput( Operator.noModulator );
			op3Output = m_op3->getOperatorOutput( op2Output * toPhase );

			op4Output = m_op4->getOperatorOutput( Operator.noModulator );

			channelOutput = ( op1Output + op3Output + op4Output ) / 3;
	}

	feedback[0] = feedback[1];
	feedback[1] = ( op1Output * ChannelData.feedback[fb] ) % 1;

	output = getInFourChannels( channelOutput );
	return output;
}
void Channel4Op::keyOn()
{
	m_op1->keyOn();
	m_op2->keyOn();
	m_op3->keyOn();
	m_op4->keyOn();
	feedback[0] = feedback[1] = 0;
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
	// Key Scale Number, used in EnvelopeGenerator.setActualRates().
	int keyScaleNumber = block * 2 + ( ( fnumh >> OPL3.nts ) & 0x01 );
	int f_number = ( fnumh << 8 ) | fnuml;
	m_op1->updateOperator( keyScaleNumber, f_number, block );
	m_op2->updateOperator( keyScaleNumber, f_number, block );
	m_op3->updateOperator( keyScaleNumber, f_number, block );
	m_op4->updateOperator( keyScaleNumber, f_number, block );
}
}

