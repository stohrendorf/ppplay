#ifndef PPP_OPL_TOPCYMBALOPERATOR_H
#define PPP_OPL_TOPCYMBALOPERATOR_H
#include "operator.h"

namespace opl
{

class Opl3;
class TopCymbalOperator : public Operator
{
	static constexpr int topCymbalOperatorBaseAddress = 0x15;

public:

	TopCymbalOperator( Opl3* opl, int baseAddress = topCymbalOperatorBaseAddress ) : Operator( opl, baseAddress ) {
	}

	double getOperatorOutput( int modulator ) ;

protected:
	// This method is used here with the HighHatOperator phase
	// as the externalPhase.
	// Conversely, this method is also used through inheritance by the HighHatOperator,
	// now with the TopCymbalOperator phase as the externalPhase.
	double getOperatorOutput( int modulator, int externalPhase ) ;
};
}

#endif
