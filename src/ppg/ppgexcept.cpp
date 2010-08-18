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
#include <stdio.h>

PpgException::PpgException(const std::string &msg) throw() : exception(), m_msg(msg) {
}

PpgException::PpgException(const std::string &msg, const char file[], const int lineno, const char function[]) throw(): exception(), m_msg(msg) {
	char cMsg[256];
	sprintf(cMsg, "(PpgException) %s:%u:%s - %s", file, lineno, function, msg.c_str());
	m_msg = cMsg;
}

PpgException::PpgException(const PpgException &previous, const char file[], const int lineno, const char function[]) throw(): exception(), m_msg(previous.what()) {
	char cMsg[256];
	sprintf(cMsg, "%s\n\tfrom %s:%u:%s", previous.what(), file, lineno, function);
	m_msg = cMsg;
}

PpgException::~PpgException() throw() {
}

const char *PpgException::what() const throw() {
	return m_msg.c_str();
}

