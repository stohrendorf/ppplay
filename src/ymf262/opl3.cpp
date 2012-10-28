#include "opl3.h"

namespace opl
{
Opl3::Opl3() : m_registers(), m_nts( 0 ), dam( 0 ), m_dvb( 0 ), ryt( 0 ), bd( 0 ), sd( 0 ), tom( 0 ), tc( 0 ), hh( 0 ), _new( 0 ), connectionsel( 0 ),
	m_vibratoIndex( 0 ), tremoloIndex( 0 ), channels()
{
	static const int loadTablesVar = loadTables();

	initOperators();
	initChannels2op();
	initChannels4op();
	initRhythmChannels();
	initChannels();
}
}

