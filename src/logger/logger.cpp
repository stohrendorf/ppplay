/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
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

/**
* @file
* @ingroup Logger
* @brief Logger definitions (implementation)
*/

#include <sstream>
#include <iostream>
#include <stack>

#include "logger.h"

/**
 * @ingroup Logger
 * @brief The current log level
 */
static LogLevel currentLogLevel = llError;

#ifndef LOG_NOTRACE
/**
 * @ingroup Logger
 * @brief Function trace stack
 * @todo This should be a std::stack for performance reasons
 */
static std::stack<string> logStack;

/**
 * @ingroup Logger
 * @brief Indentation string repeated in front of any messages
 */
static const std::string logIndentString = "  ";
/**
 * @ingroup Logger
 * @brief String printed when a function trace begins
 */
static const std::string logCallIndentString = " #  [call] ";
/**
 * @ingroup Logger
 * @brief String printed when a function trace returns
 */
static const std::string logRetIndentString  = " #[return] ";
/**
 * @ingroup Logger
 * @brief String printed when a message is logged
 */
static const std::string logFuncIndentString = " : ";

static unsigned int lastLoggedDepth = 0;

/**
 * @ingroup Logger
 * @brief Print the indentation
 * @param[in] isFunc Set to @c true if this is a function call
 * @param[in] isReturn Set to @c true if this is a function return
 */
void printIndent(bool isFunc, bool isReturn) {
	for(unsigned int i=0; i<lastLoggedDepth; i++)
		std::cout << logIndentString;
	if(!isFunc) {
		if(!isReturn)
			std::cout << logCallIndentString;
		else
			std::cout << logRetIndentString;
	}
	else
		std::cout << logFuncIndentString;
}
#endif

using std::cout;
using std::flush;
using std::endl;
void logger(const std::string &msg, LogLevel ll) {
	if(ll<currentLogLevel && ll!=-1)
		return;
#	ifndef LOG_NOTRACE
	while(lastLoggedDepth<logStack.size()) {
		printIndent(false,false);
		cout << logStack[lastLoggedDepth] << endl << flush;
		lastLoggedDepth++;
	}
	printIndent(true,false);
#	endif
	switch(ll) {
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
	cout << msg << endl << flush;
}

void setLogLevel(LogLevel ll) {
	currentLogLevel = ll;
}

LogLevel getLogLevel() {
	return currentLogLevel;
}

#ifndef LOG_NOTRACE
int incLogLevel(string func) {
	logStack.push_back(func);
	return logStack.size();
}

int decLogLevel() {
	if(!logStack.empty()) {
		if(lastLoggedDepth >= logStack.size()) {
			lastLoggedDepth--;
			printIndent(false,true);
			cout << *(--logStack.end()) << endl << flush;
		}
		logStack.pop_back();
	}
	return logStack.size();
}
#endif
