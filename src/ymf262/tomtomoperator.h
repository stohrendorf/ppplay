#ifndef PPP_OPL_TOMTOMOPERATOR_H
#define PPP_OPL_TOMTOMOPERATOR_H
#include "operator.h"

namespace opl
{

class Opl3;
class TomTomOperator : public Operator
{
public:
	static constexpr int tomTomOperatorBaseAddress = 0x12;
	TomTomOperator(Opl3* opl) : Operator( opl, tomTomOperatorBaseAddress ) {
	}
};
}

#endif
