#include "operator.h"
#include "opl3.h"

namespace opl
{
int Operator::loadWaveforms()
{
	//OPL3 has eight waveforms:

	int i;
	// 1st waveform: sinusoid.
	double theta = 0, thetaIncrement = 2 * M_PI / 1024;

	for( i = 0, theta = 0; i < 1024; i++, theta += thetaIncrement )
		waveforms[0][i] = std::sin( theta );

	double* sineTable = waveforms[0];
	// 2nd: first half of a sinusoid.
	for( i = 0; i < 512; i++ ) {
		waveforms[1][i] = sineTable[i];
		waveforms[1][512 + i] = 0;
	}
	// 3rd: double positive sinusoid.
	for( i = 0; i < 512; i++ )
		waveforms[2][i] = waveforms[2][512 + i] = sineTable[i];
	// 4th: first and third quarter of double positive sinusoid.
	for( i = 0; i < 256; i++ ) {
		waveforms[3][i] = waveforms[3][512 + i] = sineTable[i];
		waveforms[3][256 + i] = waveforms[3][768 + i] = 0;
	}
	// 5th: first half with double frequency sinusoid.
	for( i = 0; i < 512; i++ ) {
		waveforms[4][i] = sineTable[i * 2];
		waveforms[4][512 + i] = 0;
	}
	// 6th: first half with double frequency positive sinusoid.
	for( i = 0; i < 256; i++ ) {
		waveforms[5][i] = waveforms[5][256 + i] = sineTable[i * 2];
		waveforms[5][512 + i] = waveforms[5][768 + i] = 0;
	}
	// 7th: square wave
	for( i = 0; i < 512; i++ ) {
		waveforms[6][i] = 1;
		waveforms[6][512 + i] = -1;
	}
	// 8th: exponential
	double x;
	double xIncrement = 1 * 16.0 / 256.0;
	for( i = 0, x = 0; i < 512; i++, x += xIncrement ) {
		waveforms[7][i] = std::pow( 2, -x );
		waveforms[7][1023 - i] = -std::pow( 2, -( x + 1 / 16.0 ) );
	}
	return 1;
}
void Operator::update_AM1_VIB1_EGT1_KSR1_MULT4()
{

	int am1_vib1_egt1_ksr1_mult4 = opl()->readReg(m_operatorBaseAddress + Operator::AM1_VIB1_EGT1_KSR1_MULT4_Offset);

	// Amplitude Modulation. This register is used int EnvelopeGenerator.getEnvelope();
	m_am  = ( am1_vib1_egt1_ksr1_mult4 & 0x80 ) >> 7;
	// Vibrato. This register is used in PhaseGenerator.getPhase();
	m_vib = ( am1_vib1_egt1_ksr1_mult4 & 0x40 ) >> 6;
	// Envelope Generator Type. This register is used in EnvelopeGenerator.getEnvelope();
	m_egt = ( am1_vib1_egt1_ksr1_mult4 & 0x20 ) >> 5;
	// Key Scale Rate. Sets the actual envelope rate together with rate and keyScaleNumber.
	// This register os used in EnvelopeGenerator.setActualAttackRate().
	m_ksr = ( am1_vib1_egt1_ksr1_mult4 & 0x10 ) >> 4;
	// Multiple. Multiplies the Channel.baseFrequency to get the Operator.operatorFrequency.
	// This register is used in PhaseGenerator.setFrequency().
	m_mult = am1_vib1_egt1_ksr1_mult4 & 0x0F;

	m_phaseGenerator.setFrequency( m_f_number, m_block, m_mult );
	m_envelopeGenerator.setActualAttackRate( m_ar, m_ksr, m_keyScaleNumber );
	m_envelopeGenerator.setActualDecayRate( m_dr, m_ksr, m_keyScaleNumber );
	m_envelopeGenerator.setActualReleaseRate( m_rr, m_ksr, m_keyScaleNumber );
}
void Operator::update_KSL2_TL6()
{

	int ksl2_tl6 = opl()->readReg(m_operatorBaseAddress + Operator::KSL2_TL6_Offset);

	// Key Scale Level. Sets the attenuation in accordance with the octave.
	m_ksl = ( ksl2_tl6 & 0xC0 ) >> 6;
	// Total Level. Sets the overall damping for the envelope.
	m_tl  =  ksl2_tl6 & 0x3F;

	m_envelopeGenerator.setAtennuation( m_f_number, m_block, m_ksl );
	m_envelopeGenerator.setTotalLevel( m_tl );
}
void Operator::update_AR4_DR4()
{

	int ar4_dr4 = opl()->readReg(m_operatorBaseAddress + Operator::AR4_DR4_Offset);

	// Attack Rate.
	m_ar = ( ar4_dr4 & 0xF0 ) >> 4;
	// Decay Rate.
	m_dr =  ar4_dr4 & 0x0F;

	m_envelopeGenerator.setActualAttackRate( m_ar, m_ksr, m_keyScaleNumber );
	m_envelopeGenerator.setActualDecayRate( m_dr, m_ksr, m_keyScaleNumber );
}
void Operator::update_SL4_RR4()
{

	int sl4_rr4 = opl()->readReg(m_operatorBaseAddress + Operator::SL4_RR4_Offset);

	// Sustain Level.
	m_sl = ( sl4_rr4 & 0xF0 ) >> 4;
	// Release Rate.
	m_rr =  sl4_rr4 & 0x0F;

	m_envelopeGenerator.setActualSustainLevel( m_sl );
	m_envelopeGenerator.setActualReleaseRate( m_rr, m_ksr, m_keyScaleNumber );
}
void Operator::update_5_WS3()
{
	int _5_ws3 = opl()->readReg(m_operatorBaseAddress + Operator::_5_WS3_Offset);
	m_ws =  _5_ws3 & 0x07;
}
double Operator::getOperatorOutput( double modulator )
{
	if( m_envelopeGenerator.isOff() ) return 0;

	double envelopeInDB = m_envelopeGenerator.getEnvelope( m_egt, m_am );
	m_envelope = std::pow( 10, envelopeInDB / 10.0 );

	// If it is in OPL2 mode, use first four waveforms only:
	m_ws &= ( ( opl()->isNew() << 2 ) + 3 );
	const double* waveform = Operator::waveforms[m_ws];

	m_phase = m_phaseGenerator.getPhase( m_vib );

	double operatorOutput = getOutput( modulator, m_phase, waveform );
	return operatorOutput;
}
double Operator::getOutput( double modulator, double outputPhase, const double* waveform )
{
	outputPhase = std::fmod( outputPhase + modulator, 1);
	if( outputPhase < 0 ) {
		outputPhase++;
		// If the double could not afford to be less than 1:
		outputPhase = std::fmod(outputPhase,1);
	}
	int sampleIndex = ( int )( outputPhase * Operator::waveLength );
	return waveform[sampleIndex] * m_envelope;
}
void Operator::keyOn()
{
	if( m_ar > 0 ) {
		m_envelopeGenerator.keyOn();
		m_phaseGenerator.keyOn();
	}
	else m_envelopeGenerator.setOff();
}
void Operator::keyOff()
{
	m_envelopeGenerator.keyOff();
}
void Operator::updateOperator( int ksn, int f_num, int blk )
{
	m_keyScaleNumber = ksn;
	m_f_number = f_num;
	m_block = blk;
	update_AM1_VIB1_EGT1_KSR1_MULT4();
	update_KSL2_TL6();
	update_AR4_DR4();
	update_SL4_RR4();
	update_5_WS3();
}
Operator::Operator( Opl3* opl, int baseAddress ) : m_opl(opl), m_operatorBaseAddress( baseAddress ), m_phaseGenerator(opl), m_envelopeGenerator(),
	m_envelope( 0 ), m_am( 0 ), m_vib( 0 ), m_ksr( 0 ), m_egt( 0 ), m_mult( 0 ), m_ksl( 0 ), m_tl( 0 ),
	m_ar( 0 ), m_dr( 0 ), m_sl( 0 ), m_rr( 0 ), m_ws( 0 ),
	m_keyScaleNumber( 0 ), m_f_number( 0 ), m_block( 0 )
{
	static const int waveform_init = loadWaveforms();
}
}

