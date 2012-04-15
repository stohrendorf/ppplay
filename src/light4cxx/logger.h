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
#include "stuff/stringutils.h"

#include <boost/format.hpp>

#include <memory>

namespace light4cxx
{

/**
 * @ingroup light4cxx
 * @{
 */

/**
 * @class Logger
 * @brief The logger class
 */
class Logger
{
	DISABLE_COPY( Logger )
	Logger() = delete;
public:
	/**
	 * @brief Class pointer
	 */
	typedef std::shared_ptr<Logger> Ptr;
	/**
	 * @brief Create a new or get an existing logger
	 * @param[in] name The name of the new logger
	 * @return Shared pointer to the new logger
	 */
	static Ptr get( const std::string& name );
	/**
	 * @brief Get the root logger
	 * @return Shared pointer to the root logger
	 * @note The root logger will always be named "root"
	 */
	static Ptr root();
	/**
	 * @brief Get the logger's name
	 * @return m_name
	 */
	inline std::string name() const {
		return m_name;
	}
	/**
	 * @brief Log a message
	 * @param[in] l The level of the message, except Level::Off or Level::All
	 * @param[in] loc The location of the message
	 * @param[in] str The message itself
	 * @see L4CXX_LOCATION
	 */
	void log( Level l, const Location& loc, const std::string& str ) const;
	/**
	 * @brief Log a message with Level::Trace
	 * @param[in] loc The location
	 * @param[in] fmt The message format string
	 */
	template<class ...Args>
	void trace(const Location& loc, const std::string& fmt, const Args& ...args)
	{
		log(Level::Trace, loc, stringFmt(fmt, args...));
	}
	/**
	 * @brief Log a message with Level::Debug
	 * @param[in] loc The location
	 * @param[in] str The message itself
	 */
	template<class ...Args>
	void debug(const Location& loc, const std::string& fmt, const Args& ...args)
	{
		log(Level::Debug, loc, stringFmt(fmt, args...));
	}
	/**
	 * @brief Log a message with Level::Info
	 * @param[in] loc The location
	 * @param[in] str The message itself
	 */
	template<class ...Args>
	void info(const Location& loc, const std::string& fmt, const Args& ...args)
	{
		log(Level::Info, loc, stringFmt(fmt, args...));
	}
	/**
	 * @brief Log a message with Level::Warn
	 * @param[in] loc The location
	 * @param[in] str The message itself
	 */
	template<class ...Args>
	void warn(const Location& loc, const std::string& fmt, const Args& ...args)
	{
		log(Level::Warn, loc, stringFmt(fmt, args...));
	}
	/**
	 * @brief Log a message with Level::Error
	 * @param[in] loc The location
	 * @param[in] str The message itself
	 */
	template<class ...Args>
	void error(const Location& loc, const std::string& fmt, const Args& ...args)
	{
		log(Level::Error, loc, stringFmt(fmt, args...));
	}
	/**
	 * @brief Log a message with Level::Fatal
	 * @param[in] loc The location
	 * @param[in] str The message itself
	 */
	template<class ...Args>
	void fatal(const Location& loc, const std::string& fmt, const Args& ...args)
	{
		log(Level::Fatal, loc, stringFmt(fmt, args...));
	}
	/**
	 * @brief Get the current log level filter
	 * @return The log level filter, including Level::Off and Level::All
	 */
	static Level level();
	/**
	 * @brief Sets the log level filter
	 * @param[in] l The log level filter, including Level::Off and Level::All
	 */
	static void setLevel( Level l );
private:
	/**
	 * @brief Private constructor
	 * @param[in] name The logger's name
	 */
	Logger( const std::string& name );
	/**
	 * @brief The logger's name
	 */
	std::string m_name;
};

/**
 * @}
 */

}

#endif
