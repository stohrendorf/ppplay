/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef LOCATION_H
#define LOCATION_H

#include "level.h"

#include <string>

namespace light4cxx {

class Logger;

class Location {
private:
	int m_line;
	std::string m_file;
	std::string m_function;
	__gthread_t m_threadId;
	Location() = delete;
public:
	inline Location(int line, const std::string& file, const std::string& function, const __gthread_t& id)
		: m_line(line), m_file(file), m_function(function), m_threadId(id)
	{
	}
	inline int line() const {
		return m_line;
	}
	inline std::string function() const {
		return m_function;
	}
	inline std::string file() const {
		return m_file;
	}
	inline __gthread_t threadId() const {
		return m_threadId;
	}
	std::string toString(Level l, const Logger& logger, const std::string& msg) const;
	static void setFormat(const std::string& fmt);
};

#define L4CXX_LOCATION ::light4cxx::Location(__LINE__, __FILE__, __PRETTY_FUNCTION__, __gthread_self())

}

#endif