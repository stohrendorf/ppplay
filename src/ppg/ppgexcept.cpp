/***************************************************************************
 *   Copyright (C) 2009 by Syron                                         *
 *   mr.syron@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ppgexcept.h"
#include <iostream>
#include <sstream>

namespace ppg {

	static std::string makePos( const unsigned int lineno, const char function[] ) {
		std::ostringstream out;
		out << std::dec << lineno << ":" << function;
		return out.str();
	}

	Exception::Exception( const std::string& msg ) throw() : exception(), m_msg( msg ) {
	}

	Exception::Exception( const std::string& msg, const int lineno, const char function[] ) throw(): exception(), m_msg( msg ) {
		std::ostringstream buf;
		buf << "(ppg::Exception) Backtrace, most recent call first:" << std::endl << "\tfrom " << makePos( lineno, function ) << " - " << msg;
		m_msg.assign( buf.str() );
	}

	Exception::Exception( const Exception& previous, const int lineno, const char function[] ) throw(): exception(), m_msg( previous.what() ) {
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
