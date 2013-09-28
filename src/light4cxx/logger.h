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

#ifndef LIGHT4CXX_LOGGER_H
#define LIGHT4CXX_LOGGER_H

#include "level.h"
#include "location.h"

#include <stuff/utils.h>
#include <stuff/stringutils.h>

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
class PPPLAY_LIGHT4CXX_EXPORT Logger
{
	DISABLE_COPY( Logger )
	Logger() = delete;
public:
	/**
	 * @brief Create a new or get an existing logger
	 * @param[in] name The name of the new logger
	 * @return Shared pointer to the new logger
	 */
	static Logger* get( const std::string& name );
	/**
	 * @brief Get the root logger
	 * @return Shared pointer to the root logger
	 * @note The root logger will always be named "root"
	 */
	static Logger* root();
	static void setOutput(std::ostream* stream)
	{
		s_output = stream;
	}
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
		if( Level::Trace < s_level || s_level == Level::Off ) {
			return;
		}
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
		if( Level::Debug < s_level || s_level == Level::Off ) {
			return;
		}
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
		if( Level::Info < s_level || s_level == Level::Off ) {
			return;
		}
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
		if( Level::Warn < s_level || s_level == Level::Off ) {
			return;
		}
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
		if( Level::Error < s_level || s_level == Level::Off ) {
			return;
		}
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
		if( Level::Fatal < s_level || s_level == Level::Off ) {
			return;
		}
		log(Level::Fatal, loc, stringFmt(fmt, args...));
	}
	/**
	 * @brief Get the current log level filter
	 * @return The log level filter, including Level::Off and Level::All
	 */
	static Level level()
	{
		return s_level;
	}
	/**
	 * @brief Sets the log level filter
	 * @param[in] l The log level filter, including Level::Off and Level::All
	 */
	static void setLevel( Level l )
	{
		s_level = l;
	}
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
	
	//! @brief The current logging level filter
	static Level s_level;
	//! @brief Output stream (defaults to stdout)
	static std::ostream* s_output;
};

/**
 * @}
 */

}

#endif
