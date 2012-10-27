#ifndef PPP_OPL_CHANNEL2OP_H
#define PPP_OPL_CHANNEL2OP_H

#include "abstractchannel.h"

namespace opl
{
class Channel2Op : public AbstractChannel
{
private:
	Operator* m_op1;
	Operator* m_op2;

public:
	Channel2op( int baseAddress, Operator* o1, Operator* o2 )
		: AbstractChannel( baseAddress ), m_op1( o1 ), m_op2( o2 ) {
	}

	std::vector<double> getChannelOutput() {
		double channelOutput = 0, op1Output = 0, op2Output = 0;
		std::vector<double> output;
		// The feedback uses the last two outputs from
		// the first operator, instead of just the last one.
		double feedbackOutput = ( m_feedback[0] + m_feedback[1] ) / 2;


		switch( cnt ) {
				// CNT = 0, the operators are in series, with the first in feedback.
			case 0:
				if( m_op2->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF )
					return getInFourChannels( 0 );
				op1Output = m_op1->getOperatorOutput( feedbackOutput );
				channelOutput = m_op2->getOperatorOutput( op1Output * toPhase );
				break;
				// CNT = 1, the operators are in parallel, with the first in feedback.
			case 1:
				if( m_op1->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF &&
						m_op2->envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF )
					return getInFourChannels( 0 );
				op1Output = m_op1->getOperatorOutput( feedbackOutput );
				op2Output = m_op2->getOperatorOutput( Operator.noModulator );
				channelOutput = ( op1Output + op2Output ) / 2;
		}

		m_feedback[0] = m_feedback[1];
		m_feedback[1] = ( op1Output * ChannelData.feedback[fb] ) % 1;
		output = getInFourChannels( channelOutput );
		return output;
	}

protected:
	void keyOn() {
		m_op1->keyOn();
		m_op2->keyOn();
		feedback[0] = feedback[1] = 0;
	}

	void keyOff() {
		m_op1->keyOff();
		m_op2->keyOff();
	}

	void updateOperators() {
		// Key Scale Number, used in EnvelopeGenerator.setActualRates().
		int keyScaleNumber = block * 2 + ( ( m_fnumh >> OPL3.nts ) & 0x01 );
		int f_number = ( m_fnumh << 8 ) | m_fnuml;
		m_op1->updateOperator( keyScaleNumber, f_number, block );
		m_op2->updateOperator( keyScaleNumber, f_number, block );
	}

//     @Override
//     public String toString() {
//         StringBuffer str = new StringBuffer();
//
//         int f_number = (fnumh<<8)+fnuml;
//
//         str.append(String.format("channelBaseAddress: %d\n", channelBaseAddress));
//         str.append(String.format("f_number: %d, block: %d\n", f_number, block));
//         str.append(String.format("cnt: %d, feedback: %d\n", cnt, fb));
//         str.append(String.format("op1:\n%s", m_op1->toString()));
//         str.append(String.format("op2:\n%s", m_op2->toString()));
//
//         return str.toString();
//     }
};
}

#endif
