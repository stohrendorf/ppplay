#ifndef PPP_OPL_ENVELOPEGENERATOR_H
#define PPP_OPL_ENVELOPEGENERATOR_H

#include <cmath>

namespace opl
{
class EnvelopeGenerator
{
public:
	static constexpr double* INFINITYX = nullptr;
	enum class Stage
	{
		ATTACK, DECAY, SUSTAIN, RELEASE, OFF
	};
private:
	Stage m_stage;
	int m_actualAttackRate;
	int m_actualDecayRate;
	int m_actualReleaseRate;
	double m_xAttackIncrement;
	double m_xMinimumInAttack;
	double m_dBdecayIncrement;
	double m_dBreleaseIncrement;
	double m_attenuation;
	double m_totalLevel;
	double m_sustainLevel;
	double m_x;
	double m_envelope;

public:
	bool isOff() const { return m_stage == Stage::OFF; }
	void setOff() { m_stage=Stage::OFF; }
	
	EnvelopeGenerator()
	: m_stage(Stage::OFF), m_actualAttackRate(0), m_actualDecayRate(0), m_actualReleaseRate(0),
	  m_xAttackIncrement(0), m_xMinimumInAttack(0),
	  m_dBdecayIncrement(0), m_dBreleaseIncrement(0),
	  m_attenuation(0), m_totalLevel(0), m_sustainLevel(0),
	  m_x(dBtoX(-96)), m_envelope(-96)
	{
	}

	void setActualSustainLevel( int sl ) ;

	void setTotalLevel( int tl ) ;

	void setAtennuation( int f_number, int block, int ksl ) ;

	void setActualAttackRate( int attackRate, int ksr, int keyScaleNumber ) ;


	void setActualDecayRate( int decayRate, int ksr, int keyScaleNumber ) ;

	void setActualReleaseRate( int releaseRate, int ksr, int keyScaleNumber ) ;

private:
	int calculateActualRate( int rate, int ksr, int keyScaleNumber ) ;
public:
	double getEnvelope( int egt, int am ) const;

	void keyOn() ;

	void keyOff() ;

private:
	static double dBtoX( double dB ) ;

	static double percentageToDB( double percentage ) {
		return std::log10( percentage ) * 10;
	}

	static double percentageToX( double percentage ) {
		return dBtoX( percentageToDB( percentage ) );
	}

//     @Override
//     public String toString() {
//         StringBuffer str = new StringBuffer();
//         str.append("Envelope Generator: \n");
//         double attackPeriodInSeconds = EnvelopeGeneratorData.attackTimeValuesTable[actualAttackRate][0]/1000d;
//         str.append(String.format("\tATTACK  %f s, rate %d. \n", attackPeriodInSeconds, actualAttackRate));
//         double decayPeriodInSeconds = EnvelopeGeneratorData.decayAndReleaseTimeValuesTable[actualDecayRate][0]/1000d;
//         str.append(String.format("\tDECAY   %f s, rate %d. \n",decayPeriodInSeconds, actualDecayRate));
//         str.append(String.format("\tSL      %f dB. \n", sustainLevel));
//         double releasePeriodInSeconds = EnvelopeGeneratorData.decayAndReleaseTimeValuesTable[actualReleaseRate][0]/1000d;
//         str.append(String.format("\tRELEASE %f s, rate %d. \n", releasePeriodInSeconds,actualReleaseRate));
//         str.append("\n");
//
//         return str.toString();
//     }
};
}

#endif
