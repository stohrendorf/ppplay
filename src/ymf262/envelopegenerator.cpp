#include "envelopegenerator.h"

namespace opl
{
void EnvelopeGenerator::setActualSustainLevel( int sl )
{
	// If all SL bits are 1, sustain level is set to -93 dB:
	if( sl == 0x0F ) {
		m_sustainLevel = -93;
		return;
	}
	// The datasheet states that the SL formula is
	// sustainLevel = -24*d7 -12*d6 -6*d5 -3*d4,
	// translated as:
	m_sustainLevel = -3 * sl;
}
void EnvelopeGenerator::setTotalLevel( int tl )
{
	// The datasheet states that the TL formula is
	// TL = -(24*d5 + 12*d4 + 6*d3 + 3*d2 + 1.5*d1 + 0.75*d0),
	// translated as:
	m_totalLevel = tl * -0.75;
}
void EnvelopeGenerator::setAtennuation( int f_number, int block, int ksl )
{
	int hi4bits = ( f_number >> 6 ) & 0x0F;
	switch( ksl ) {
		case 0:
			m_attenuation = 0;
			break;
		case 1:
			// ~3 dB/Octave
			m_attenuation = Operator::ksl3dBtable[hi4bits][block];
			break;
		case 2:
			// ~1.5 dB/Octave
			m_attenuation = Operator::ksl3dBtable[hi4bits][block] / 2;
			break;
		case 3:
			// ~6 dB/Octave
			m_attenuation = Operator::ksl3dBtable[hi4bits][block] * 2;
	}
}
void EnvelopeGenerator::setActualAttackRate( int attackRate, int ksr, int keyScaleNumber )
{
	// According to the YMF278B manual's OPL3 section, the attack curve is exponential,
	// with a dynamic range from -96 dB to 0 dB and a resolution of 0.1875 dB
	// per level.
	//
	// This method sets an attack increment and attack minimum value
	// that creates a exponential dB curve with 'period0to100' seconds in length
	// and 'period10to90' seconds between 10% and 90% of the curve total level.
	m_actualAttackRate = calculateActualRate( attackRate, ksr, keyScaleNumber );
	double period0to100inSeconds = EnvelopeGeneratorData.attackTimeValuesTable[m_actualAttackRate][0] / 1000d;
	int period0to100inSamples = ( int )( period0to100inSeconds * OPL3Data.sampleRate );
	double period10to90inSeconds = EnvelopeGeneratorData.attackTimeValuesTable[m_actualAttackRate][1] / 1000d;
	int period10to90inSamples = ( int )( period10to90inSeconds * OPL3Data.sampleRate );
	// The x increment is dictated by the period between 10% and 90%:
	m_xAttackIncrement = OPL3Data.calculateIncrement( percentageToX( 0.1 ), percentageToX( 0.9 ), period10to90inSeconds );
	// Discover how many samples are still from the top.
	// It cannot reach 0 dB, since x is a logarithmic parameter and would be
	// negative infinity. So we will use -0.1875 dB as the resolution
	// maximum.
	//
	// percentageToX(0.9) + samplesToTheTop*xAttackIncrement = dBToX(-0.1875); ->
	// samplesToTheTop = (dBtoX(-0.1875) - percentageToX(0.9)) / xAttackIncrement); ->
	// period10to100InSamples = period10to90InSamples + samplesToTheTop; ->
	int period10to100inSamples = ( int )( period10to90inSamples + ( dBtoX( -0.1875 ) - percentageToX( 0.9 ) ) / m_xAttackIncrement );
	// Discover the minimum x that, through the attackIncrement value, keeps
	// the 10%-90% period, and reaches 0 dB at the total period:
	m_xMinimumInAttack = percentageToX( 0.1 ) - ( period0to100inSamples - period10to100inSamples ) * m_xAttackIncrement;
}
void EnvelopeGenerator::setActualDecayRate( int decayRate, int ksr, int keyScaleNumber )
{
	m_actualDecayRate = calculateActualRate( decayRate, ksr, keyScaleNumber );
	double period10to90inSeconds = EnvelopeGeneratorData.decayAndReleaseTimeValuesTable[m_actualDecayRate][1] / 1000d;
	// Differently from the attack curve, the decay/release curve is linear.
	// The dB increment is dictated by the period between 10% and 90%:
	m_dBdecayIncrement = OPL3Data.calculateIncrement( percentageToDB( 0.1 ), percentageToDB( 0.9 ), period10to90inSeconds );
}
void EnvelopeGenerator::setActualReleaseRate( int releaseRate, int ksr, int keyScaleNumber )
{
	m_actualReleaseRate =  calculateActualRate( releaseRate, ksr, keyScaleNumber );
	double period10to90inSeconds = EnvelopeGeneratorData.decayAndReleaseTimeValuesTable[m_actualReleaseRate][1] / 1000d;
	m_dBreleaseIncrement = OPL3Data.calculateIncrement( percentageToDB( 0.1 ), percentageToDB( 0.9 ), period10to90inSeconds );
}
int EnvelopeGenerator::calculateActualRate( int rate, int ksr, int keyScaleNumber )
{
	int rof = EnvelopeGeneratorData.rateOffset[ksr][keyScaleNumber];
	int actualRate = rate * 4 + rof;
	// If, as an example at the maximum, rate is 15 and the rate offset is 15,
	// the value would
	// be 75, but the maximum allowed is 63:
	if( actualRate > 63 ) actualRate = 63;
	return actualRate;
}
double EnvelopeGenerator::getEnvelope( int egt, int am  const )
{
	// The datasheets attenuation values
	// must be halved to match the real OPL3 output.
	double envelopeSustainLevel = m_sustainLevel / 2;
	double envelopeTremolo =
		OPL3Data.tremoloTable[OPL3.dam][OPL3.tremoloIndex] / 2;
	double envelopeAttenuation = m_attenuation / 2;
	double envelopeTotalLevel = m_totalLevel / 2;

	double envelopeMinimum = -96;
	double envelopeResolution = 0.1875;

	double outputEnvelope;
	//
	// Envelope Generation
	//
	switch( m_stage ) {
		case Stage::ATTACK:
			// Since the attack is exponential, it will never reach 0 dB, so
			// we´ll work with the next to maximum in the envelope resolution.
			if( m_envelope < -envelopeResolution && m_xAttackIncrement != -EnvelopeGeneratorData::INFINITY ) {
				// The attack is exponential.
				m_envelope = -std::pow( 2, x );
				m_x += m_xAttackIncrement;
				break;
			}
			else {
				// It is needed here to explicitly set envelope = 0, since
				// only the attack can have a period of
				// 0 seconds and produce an infinity envelope increment.
				m_envelope = 0;
				m_stage = Stage::DECAY;
			}
		case Stage::DECAY:
			// The decay and release are linear.
			if( m_envelope > envelopeSustainLevel ) {
				m_envelope -= m_dBdecayIncrement;
				break;
			}
			else
				m_stage = Stage::SUSTAIN;
		case Stage::SUSTAIN:
			// The Sustain stage is mantained all the time of the Key ON,
			// even if we are in non-sustaining mode.
			// This is necessary because, if the key is still pressed, we can
			// change back and forth the state of EGT, and it will release and
			// hold again accordingly.
			if( egt == 1 ) break;
			else {
				if( m_envelope > envelopeMinimum )
					m_envelope -= m_dBreleaseIncrement;
				else m_stage = Stage::OFF;
			}
			break;
		case Stage::RELEASE:
			// If we have Key OFF, only here we are in the Release stage.
			// Now, we can turn EGT back and forth and it will have no effect,i.e.,
			// it will release inexorably to the Off stage.
			if( m_envelope > envelopeMinimum )
				m_envelope -= m_dBreleaseIncrement;
			else m_stage = Stage::OFF;
	}

	// Ongoing original envelope
	outputEnvelope = m_envelope;

	//Tremolo
	if( am == 1 ) outputEnvelope += envelopeTremolo;

	//Attenuation
	outputEnvelope += envelopeAttenuation;

	//Total Level
	outputEnvelope += envelopeTotalLevel;

	return outputEnvelope;
}
void EnvelopeGenerator::keyOn()
{
	// If we are taking it in the middle of a previous envelope,
	// start to rise from the current level:
	// envelope = - (2 ^ x); ->
	// 2 ^ x = -envelope ->
	// x = log2(-envelope); ->
	double xCurrent = Operator::log2( -m_envelope );
	x = xCurrent < m_xMinimumInAttack ? xCurrent : m_xMinimumInAttack;
	m_stage = Stage::ATTACK;
}
void EnvelopeGenerator::keyOff()
{
	if( m_stage != Stage::OFF ) m_stage = Stage::RELEASE;
}
double EnvelopeGenerator::dBtoX( double dB )
{
	return Operator::log2( -dB );
}
}

