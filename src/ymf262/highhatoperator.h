#ifndef PPP_OPL_HIGHHATOPERATOR_H
#define PPP_OPL_HIGHHATOPERATOR_H

#include <cstdlib>

#include "operator.h"

namespace opl
{

class Opl3;
class HighHatOperator : public Operator
{
public:
	static constexpr int highHatOperatorBaseAddress = 0x11;

	HighHatOperator( Opl3* opl ) : Operator( opl, highHatOperatorBaseAddress ) {
	}

	int16_t nextSample( int16_t modulator );
};
}

#endif
