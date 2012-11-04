#ifndef PPP_OPL_SNAREDRUMOPERATOR_H
#define PPP_OPL_SNAREDRUMOPERATOR_H
#include "operator.h"

namespace opl
{

class Opl3;
class SnareDrumOperator : public Operator
{
public:
	static constexpr int snareDrumOperatorBaseAddress = 0x14;

	SnareDrumOperator(Opl3* opl) : Operator( opl, snareDrumOperatorBaseAddress ) {
	}

	double getOperatorOutput( int modulator ) ;
};
}

#endif
