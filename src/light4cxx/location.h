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

/**
 * @ingroup light4cxx
 * @{
 */

namespace light4cxx {

class Logger;

/**
 * @class Location
 * @brief A class containing location information
 */
class Location {
private:
	const int m_line; //!< @brief The line withing m_file
	const std::string m_file; //!< @brief The file
	const std::string m_function; //!< @brief The function within m_file
	const __gthread_t m_threadId; //!< @brief The thread ID
	Location() = delete;
public:
	/**
	 * @brief Inline constructor
	 * @param[in] line Line of the location within @a file
	 * @param[in] file File name of the location
	 * @param[in] function Function name of the location
	 * @param[in] id Thread ID of the location
	 */
	inline Location(int line, const std::string& file, const std::string& function, const __gthread_t& id)
		: m_line(line), m_file(file), m_function(function), m_threadId(id)
	{
	}
	/**
	 * @brief Get m_line
	 * @return m_line
	 */
	inline int line() const {
		return m_line;
	}
	/**
	 * @brief Get m_function
	 * @return m_function
	 */
	inline std::string function() const {
		return m_function;
	}
	/**
	 * @brief Get m_file
	 * @return m_file
	 */
	inline std::string file() const {
		return m_file;
	}
	/**
	 * @brief Get m_threadId
	 * @return m_threadId
	 */
	inline __gthread_t threadId() const {
		return m_threadId;
	}
	/**
	 * @brief Convert a logging message to a string
	 * @param[in] l The logging level of the message
	 * @param[in] logger The logger where the message comes from
	 * @param[in] msg The actual message
	 * @return The formatted string ready for output
	 * @see toString()
	 */
	std::string toString(Level l, const Logger& logger, const std::string& msg) const;
	/**
	 * @brief Sets the format of the output of toString()
	 * @param[in] fmt The format string
	 * 
	 * @details
	 * The format string is loosely based on printf. Its syntax is:
	 * @code
	 * %<flags><fieldwidth>[.<precision>]<dataSelector>
	 * @endcode
	 * where
	 * <ul>
	 *   <li>
	 *     @b flags is zero, one or more of
	 *     <ul>
	 *       <li>@b # - Applies std::showbase</li>
	 *       <li>@b + - Applies std::showpos</li>
	 *       <li>@b , (comma) - applies std::showpoint</li>
	 *       <li>@b 0 - applies ostream.fill('0')</li>
	 *       <li>' ' (space) - applies ostream.fill(' ')</li>
	 *       <li>@b &lt; - applies std::left</li>
	 *       <li>@b | (pipe) - applies std::internal</li>
	 *       <li>@b &gt; - applies std::right</li>
	 *       <li>@b = - applies std::fixed</li>
	 *       <li>@b ~ - applies std::scientific</li>
	 *     </ul>
	 *     If multiple flags of one group (e.g., fixed or scientific notation) are given,
	 *     only the last one applies.
	 *   </li>
	 *   <li>
	 *     @b fieldwidth is an optional digit (yes, a digit, not a number) for specifying
	 *     the field width. It is passed to ostream.width().
	 *   </li>
	 *   <li>
	 *     @b . (dot) and @b precision - The dot is used to separate @e fieldwidth and @e precision. If it is
	 *     given, it marks the @e precision field to be present. The value of the @e precision field is a
	 *     digit that will be passed to ostream.precision().
	 *   </li>
	 *   <li>
	 *     @b dataSelector may be one of:
	 *     <ul>
	 *       <li>@b % - A simple percent sign.</li>
	 *       <li>@b f - The function name,</li>
	 *       <li>@b F - The file name.</li>
	 *       <li>@b l - The line number.</li>
	 *       <li>@b L - The logger's name.</li>
	 *       <li>@b m - The message.</li>
	 *       <li>@b n - A newline (std::endl exactly).</li>
	 *       <li>@b p - Current process runtime in seconds (double precision).</li>
	 *       <li>@b P - Current process runtime in seconds (integer precision).</li>
	 *       <li>@b r - Current processor time in seconds (double precision).</li>
	 *       <li>@b R - Current processor time in seconds (integer precision).</li>
	 *       <li>@b t - The log level string (one of TRACE, DEBUG, INFO, WARN, ERROR, FATAL).</li>
	 *       <li>@b T - The thread ID in hexadecimal, but without the "0x" prefix. Depending on the architecture, it may be either 8 or 16 chars wide.</li>
	 *     </ul>
	 *   </li>
	 * </ul>
	 * The default format string is
	 * @code
	 * "[%T %<5t %p] %L (in %F:%l): %m%n"
	 * @endcode
	 * which will output something like
	 * @code
	 * [7fffa234 WARN  1.234] root (in dev/foo.cpp:123): A warning message
	 * @endcode
	 * 
	 * @note The format string is pretty strict, so make sure to check it twice,
	 * otherwise you will encounter a std::runtime_error telling you that toString()
	 * cannot parse the string.
	 */
	static void setFormat(const std::string& fmt);
};

/**
 * @brief Creates a Location instance with the location set to the current
 * file, line, function and thread.
 */
#define L4CXX_LOCATION ::light4cxx::Location(__LINE__, __FILE__, __PRETTY_FUNCTION__, __gthread_self())

}

/**
 * @}
 */

#endif
