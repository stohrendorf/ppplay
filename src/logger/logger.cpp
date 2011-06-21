/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

/**
* @file
* @ingroup Logger
* @brief Logger definitions (implementation)
*/

#include "logger.h"

#include <sstream>
#include <iostream>
#include <stack>

/**
 * @ingroup Logger
 * @brief The current log level
 */
static LogLevel currentLogLevel = llError;

using std::cout;
using std::flush;
using std::endl;
void logger( const std::string& where, const std::string& msg, LogLevel ll ) {
	if( ll < currentLogLevel && ll != -1 )
		return;
	switch( ll ) {
		case llDebug:
			cout << "-- ";
			break;
		case llMessage:
			cout << "** ";
			break;
		case llWarning:
			cout << "WW ";
			break;
		case llError:
			cout << "!! ";
			break;
		default:
			cout << "?? ";
	}
	cout << where << " // " << msg << endl << flush;
}

void setLogLevel( LogLevel ll ) {
	currentLogLevel = ll;
}

LogLevel getLogLevel() {
	return currentLogLevel;
}
