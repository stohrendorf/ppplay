/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ppgexcept.h"
#include <iostream>
#include <sstream>

namespace ppg {

	static std::string makePos( int lineno, const char function[] ) {
		std::ostringstream out;
		out << std::dec << lineno << ":" << function;
		return out.str();
	}

	Exception::Exception( const std::string& msg ) throw() : exception(), m_msg( msg ) {
	}

	Exception::Exception( const std::string& msg, int lineno, const char function[] ) throw(): exception(), m_msg( msg ) {
		std::ostringstream buf;
		buf << "(ppg::Exception) Backtrace, most recent call first:" << std::endl << "\tfrom " << makePos( lineno, function ) << " - " << msg;
		m_msg.assign( buf.str() );
	}

	Exception::Exception( const Exception& previous, int lineno, const char function[] ) throw(): exception(), m_msg( previous.what() ) {
		std::ostringstream buf;
		buf << std::endl << "\tfrom " << makePos( lineno, function );
		m_msg.append( buf.str() );
	}

	Exception::~Exception() throw() {
	}

	const char* Exception::what() const throw() {
		return m_msg.c_str();
	}

} // namespace ppg
