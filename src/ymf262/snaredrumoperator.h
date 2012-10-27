#ifndef PPP_OPL_SNAREDRUMOPERATOR_H
#define PPP_OPL_SNAREDRUMOPERATOR_H
#include "operator.h"

namespace opl
{
class SnareDrumOperator : public Operator
{
	static constexpr int snareDrumOperatorBaseAddress = 0x14;

	SnareDrumOperator() : Operator( snareDrumOperatorBaseAddress ) {
	}

	double getOperatorOutput( double modulator ) {
		if( m_envelopeGenerator.stage == EnvelopeGenerator::Stage::OFF ) return 0;

		double envelopeInDB = m_envelopeGenerator.getEnvelope( m_egt, m_am );
		m_envelope = std::pow( 10, envelopeInDB / 10.0 );

		// If it is in OPL2 mode, use first four waveforms only:
		int waveIndex = m_ws & ( ( OPL3._new << 2 ) + 3 );
		const double* waveform = OperatorData.waveforms[m_waveIndex];

		m_phase = OPL3.highHatOperator.phase * 2;

		double operatorOutput = getOutput( modulator, m_phase, waveform );

		double noise = rand() * m_envelope / RAND_MAX;

		if( operatorOutput / m_envelope != 1 && operatorOutput / m_envelope != -1 ) {
			if( operatorOutput > 0 )  operatorOutput = noise;
			else if( operatorOutput < 0 ) operatorOutput = -noise;
			else operatorOutput = 0;
		}

		return operatorOutput * 2;
	}
};
}

#endif
