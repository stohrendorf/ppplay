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

	Fractional9 nextSample( Fractional9 );
};
}

#endif
