#ifndef PPP_OPL_TOMTOMOPERATOR_H
#define PPP_OPL_TOMTOMOPERATOR_H
#include "operator.h"

namespace opl
{
class TomTomOperator : public Operator
{
	static constexpr int tomTomOperatorBaseAddress = 0x12;
	TomTomOperator() : Operator( tomTomOperatorBaseAddress ) {
	}
};
}

#endif
