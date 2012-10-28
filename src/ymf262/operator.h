#ifndef PPP_OPL_OPERATOR_H
#define PPP_OPL_OPERATOR_H
#include <cmath>
#include "phasegenerator.h"
#include "envelopegenerator.h"

namespace opl
{

class Opl3;
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
	static int loadWaveforms() ;

	Opl3* m_opl;
	
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

public:
	static constexpr double noModulator = 0;
	
	void setAr(int val) { m_ar=val; }
	const EnvelopeGenerator* envelopeGenerator() const { return &m_envelopeGenerator; }
	const PhaseGenerator* phaseGenerator() const { return &m_phaseGenerator; }
	PhaseGenerator* phaseGenerator() { return &m_phaseGenerator; }
	Opl3* opl() const { return m_opl; }
	double envelope() const { return m_envelope; }
	void setEnvelope(double e) { m_envelope = e; }
	int mult() const { return m_mult; }
	double phase() const { return m_phase; }
	void setPhase(double p) { m_phase = p; }
	int egt() const { return m_egt; }
	int am() const { return m_am; }
	int ws() const { return m_ws; }
	int vib() const { return m_vib; }
	
	Operator( Opl3* opl, int baseAddress ) ;
	virtual ~Operator() {}

	void update_AM1_VIB1_EGT1_KSR1_MULT4() ;

	void update_KSL2_TL6() ;

	void update_AR4_DR4() ;

	void update_SL4_RR4() ;

	void update_5_WS3() ;

	double getOperatorOutput( double modulator ) ;

	double getOutput( double modulator, double outputPhase, const double* waveform ) ;

	void keyOn() ;

	void keyOff() ;

	void updateOperator( int ksn, int f_num, int blk ) ;

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
