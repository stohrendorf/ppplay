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

#ifndef LOGGER_H
#define LOGGER_H

#include "level.h"
#include "location.h"

#include "stuff/utils.h"

#include <boost/format.hpp>

#include <memory>

namespace light4cxx {

class Logger {
	DISABLE_COPY(Logger)
	Logger() = delete;
public:
	typedef std::shared_ptr<Logger> Ptr;
	static Ptr get(const std::string& name);
	static Ptr root();
	inline std::string name() const {
		return m_name;
	}
	void log(Level l, const Location& loc, const std::string& str) const;
	void log(Level l, const Location& loc, const boost::format& fmt) const;
	inline void trace(const Location& loc, const std::string& str) const { log(Level::Trace, loc, str); }
	inline void trace(const Location& loc, const boost::format& str) const { log(Level::Trace, loc, str); }
	inline void debug(const Location& loc, const std::string& str) const { log(Level::Debug, loc, str); }
	inline void debug(const Location& loc, const boost::format& str) const { log(Level::Debug, loc, str); }
	inline void info(const Location& loc, const std::string& str) const { log(Level::Info, loc, str); }
	inline void info(const Location& loc, const boost::format& str) const { log(Level::Info, loc, str); }
	inline void warn(const Location& loc, const std::string& str) const { log(Level::Warn, loc, str); }
	inline void warn(const Location& loc, const boost::format& str) const { log(Level::Warn, loc, str); }
	inline void error(const Location& loc, const std::string& str) const { log(Level::Error, loc, str); }
	inline void error(const Location& loc, const boost::format& str) const { log(Level::Error, loc, str); }
	inline void fatal(const Location& loc, const std::string& str) const { log(Level::Fatal, loc, str); }
	inline void fatal(const Location& loc, const boost::format& str) const { log(Level::Fatal, loc, str); }
	static Level level();
	static void setLevel(Level l);
private:
	Logger(const std::string& name);
	std::string m_name;
};

}

#endif