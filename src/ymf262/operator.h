#ifndef PPP_OPL_OPERATOR_H
#define PPP_OPL_OPERATOR_H
#include <cmath>

namespace opl
{
class Operator
{
public:
	static constexpr int
	AM1_VIB1_EGT1_KSR1_MULT4_Offset = 0x20,
	KSL2_TL6_Offset = 0x40,
	AR4_DR4_Offset = 0x60,
	SL4_RR4_Offset = 0x80,
	_5_WS3_Offset = 0xE0;

	enum class Type
	{
		NO_MODULATION, CARRIER, FEEDBACK
	};

	static constexpr int waveLength = 1024;

	static constexpr double multTable[] = {0.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15};

	static constexpr double ksl3dBtable[16][8] = {
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, -3, -6, -9},
		{0, 0, 0, 0, -3, -6, -9, -12},
		{0, 0, 0, -1.875, -4.875, -7.875, -10.875, -13.875},

		{0, 0, 0, -3, -6, -9, -12, -15},
		{0, 0, -1.125, -4.125, -7.125, -10.125, -13.125, -16.125},
		{0, 0, -1.875, -4.875, -7.875, -10.875, -13.875, -16.875},
		{0, 0, -2.625, -5.625, -8.625, -11.625, -14.625, -17.625},

		{0, 0, -3, -6, -9, -12, -15, -18},
		{0, -0.750, -3.750, -6.750, -9.750, -12.750, -15.750, -18.750},
		{0, -1.125, -4.125, -7.125, -10.125, -13.125, -16.125, -19.125},
		{0, -1.500, -4.500, -7.500, -10.500, -13.500, -16.500, -19.500},

		{0, -1.875, -4.875, -7.875, -10.875, -13.875, -16.875, -19.875},
		{0, -2.250, -5.250, -8.250, -11.250, -14.250, -17.250, -20.250},
		{0, -2.625, -5.625, -8.625, -11.625, -14.625, -17.625, -20.625},
		{0, -3, -6, -9, -12, -15, -18, -21}
	};

	static double waveforms[8][waveLength];

private:
	static int _waveform_init = loadWaveforms();
	static int loadWaveforms() {
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
		double xIncrement = 1 * 16d / 256d;
		for( i = 0, x = 0; i < 512; i++, x += xIncrement ) {
			waveforms[7][i] = std::pow( 2, -x );
			waveforms[7][1023 - i] = -std::pow( 2, -( x + 1 / 16d ) );
		}
		return 1;
	}

	static double log2( double x ) {
		return Math.log( x ) / Math.log( 2 );
	}
private:
	PhaseGenerator m_phaseGenerator;
	EnvelopeGenerator m_envelopeGenerator;

	double m_envelope;
	double m_phase;

	int m_operatorBaseAddress;
	int m_am;
	int m_vib;
	int m_ksr;
	int m_egt;
	int m_mult;
	int m_ksl;
	int m_tl;
	int m_ar;
	int m_dr;
	int m_sl;
	int m_rr;
	int m_ws;
	int m_keyScaleNumber;
	int m_f_number;
	int m_block;

	static constexpr double noModulator = 0;

public:
	Operator( int baseAddress )
		: m_operatorBaseAddress( baseAddress ), m_phaseGenerator(), m_envelopeGenerator(),
		  m_envelope( 0 ), m_am( 0 ), m_vib( 0 ), m_ksr( 0 ), m_egt( 0 ), m_mult( 0 ), m_ksl( 0 ), m_tl( 0 ),
		  m_ar( 0 ), m_dr( 0 ), m_sl( 0 ), m_rr( 0 ), m_ws( 0 ),
		  m_keyScaleNumber( 0 ), m_f_number( 0 ), m_block( 0 ) {
	}

	void update_AM1_VIB1_EGT1_KSR1_MULT4() {

		int am1_vib1_egt1_ksr1_mult4 = OPL3.registers[m_operatorBaseAddress + OperatorData.AM1_VIB1_EGT1_KSR1_MULT4_Offset];

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

	void update_KSL2_TL6() {

		int ksl2_tl6 = OPL3.registers[m_operatorBaseAddress + OperatorData.KSL2_TL6_Offset];

		// Key Scale Level. Sets the attenuation in accordance with the octave.
		m_ksl = ( ksl2_tl6 & 0xC0 ) >> 6;
		// Total Level. Sets the overall damping for the envelope.
		m_tl  =  ksl2_tl6 & 0x3F;

		m_envelopeGenerator.setAtennuation( m_f_number, m_block, m_ksl );
		m_envelopeGenerator.setTotalLevel( tl );
	}

	void update_AR4_DR4() {

		int ar4_dr4 = OPL3.registers[m_operatorBaseAddress + OperatorData.AR4_DR4_Offset];

		// Attack Rate.
		ar = ( ar4_dr4 & 0xF0 ) >> 4;
		// Decay Rate.
		dr =  ar4_dr4 & 0x0F;

		m_envelopeGenerator.setActualAttackRate( m_ar, m_ksr, m_keyScaleNumber );
		m_envelopeGenerator.setActualDecayRate( m_dr, m_ksr, m_keyScaleNumber );
	}

	void update_SL4_RR4() {

		int sl4_rr4 = OPL3.registers[m_operatorBaseAddress + OperatorData.SL4_RR4_Offset];

		// Sustain Level.
		m_sl = ( sl4_rr4 & 0xF0 ) >> 4;
		// Release Rate.
		m_rr =  sl4_rr4 & 0x0F;

		m_envelopeGenerator.setActualSustainLevel( m_sl );
		m_envelopeGenerator.setActualReleaseRate( m_rr, m_ksr, m_keyScaleNumber );
	}

	void update_5_WS3() {
		int _5_ws3 = OPL3.registers[m_operatorBaseAddress + OperatorData._5_WS3_Offset];
		m_ws =  _5_ws3 & 0x07;
	}

	double getOperatorOutput( double modulator ) {
		if( m_envelopeGenerator.stage == EnvelopeGenerator.Stage.OFF ) return 0;

		double envelopeInDB = m_envelopeGenerator.getEnvelope( m_egt, m_am );
		m_envelope = std::pow( 10, envelopeInDB / 10.0 );

		// If it is in OPL2 mode, use first four waveforms only:
		m_ws &= ( ( OPL3._new << 2 ) + 3 );
		const double* waveform = OperatorData.waveforms[m_ws];

		m_phase = m_phaseGenerator.getPhase( m_vib );

		double operatorOutput = getOutput( modulator, m_phase, waveform );
		return operatorOutput;
	}

protected:
	double getOutput( double modulator, double outputPhase, const double* waveform ) {
		outputPhase = ( outputPhase + modulator ) % 1;
		if( outputPhase < 0 ) {
			outputPhase++;
			// If the double could not afford to be less than 1:
			outputPhase %= 1;
		}
		int sampleIndex = ( int )( outputPhase * OperatorData.waveLength );
		return waveform[sampleIndex] * m_envelope;
	}

	void keyOn() {
		if( m_ar > 0 ) {
			m_envelopeGenerator.keyOn();
			m_phaseGenerator.keyOn();
		}
		else m_envelopeGenerator.stage = EnvelopeGenerator.Stage.OFF;
	}

	void keyOff() {
		m_envelopeGenerator.keyOff();
	}

	void updateOperator( int ksn, int f_num, int blk ) {
		m_keyScaleNumber = ksn;
		m_f_number = f_num;
		m_block = blk;
		update_AM1_VIB1_EGT1_KSR1_MULT4();
		update_KSL2_TL6();
		update_AR4_DR4();
		update_SL4_RR4();
		update_5_WS3();
	}

//     @Override
//     public String toString() {
//         StringBuffer str = new StringBuffer();
//
//         double operatorFrequency = f_number * Math.pow(2, block-1) * OPL3Data.sampleRate / Math.pow(2,19)*OperatorData.multTable[mult];
//
//         str.append(String.format("operatorBaseAddress: %d\n", operatorBaseAddress));
//         str.append(String.format("operatorFrequency: %f\n", operatorFrequency));
//         str.append(String.format("mult: %d, ar: %d, dr: %d, sl: %d, rr: %d, ws: %d\n", mult, ar, dr, sl, rr, ws));
//         str.append(String.format("am: %d, vib: %d, ksr: %d, egt: %d, ksl: %d, tl: %d\n", am, vib, ksr, egt, ksl, tl));
//
//         return str.toString();
//     }
};
}

#endif
