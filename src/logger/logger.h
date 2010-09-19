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
 * @defgroup Logger Logging facility
 * @brief This module contains the logger definitions
 */

/**
 * @file
 * @ingroup Logger
 * @brief Logger definitions (header)
 */

#ifndef loggerH
#define loggerH

#include <string>
#include <boost/scope_exit.hpp>

#include "utils.h"

/**
 * @ingroup Logger
 * @brief Disables function call tracing
 */
#define LOG_NOTRACE

/**
 * @enum LogLevel
 * @ingroup Logger
 * @brief Log level
 */
typedef enum {
	llMessage=0, //!< @brief Log all messages
	llWarning, //!< @brief Log only warnings and errors
	llError, //!< @brief Log only errors
	llNone, //!< @brief Don't log anything
	llDebug = 0xff
} LogLevel;

/**
 * @ingroup Logger
 * @brief Log a message
 * @param[in] msg Message to logger
 * @param[in] ll Log level of the message
 */
void logger(const std::string &msg, LogLevel ll = llMessage);

/**
 * @ingroup Logger
 * @brief Set the log level
 * @param[in] ll Log level
 */
void setLogLevel(LogLevel ll);

/**
 * @ingroup Logger
 * @brief Get the current log level
 * @return The current log level
 */
LogLevel getLogLevel();

/**
 * @def LOG(msg,ll)
 * @ingroup Logger
 * @brief Log a message
 * @param[in] msg Message
 * @param[in] ll Message's log level
 */
#ifdef LOG_NOTRACE
#	define LOG(ll,fmt,...) logger(ppp::stringf("%s // " fmt, __PRETTY_FUNCTION__, ##__VA_ARGS__), ll)
#else
#	define LOG(ll,fmt,...) logger(ppp::stringf(fmt, ##__VA_ARGS__), ll)
#endif

/**
 * @ingroup Logger
 * @brief Log a message with log level ::llMessage
 * @param[in] msg Message
 */
#define LOG_MESSAGE(fmt, ...) LOG(llMessage, fmt, ##__VA_ARGS__)
#define LOG_TEST_MESSAGE(condition) if(condition) LOG_MESSAGE("[LOG_TEST_MESSAGE] %s", #condition)

/**
* @ingroup Logger
* @brief Log a message with log level ::llWarning
* @param[in] msg Message
*/
#define LOG_WARNING(fmt, ...) LOG(llWarning, fmt, ##__VA_ARGS__)
#define LOG_TEST_WARN(condition) if(condition) LOG_WARNING("[LOG_TEST_WARN] %s", #condition)

/**
* @ingroup Logger
* @brief Log a message with log level ::llError
* @param[in] msg Message
*/
#define LOG_ERROR(fmt, ...) LOG(llError, fmt, ##__VA_ARGS__)
#define LOG_TEST_ERROR(condition) if(condition) LOG_ERROR("[LOG_TEST_ERROR] %s", #condition)

#ifndef LOG_NOTRACE
/**
 * @ingroup Logger
 * @brief Increase log depth
 * @param[in] func Function name for tracing
 * @return The new log depth
 */
int incLogLevel(string func);

/**
 * @ingroup Logger
 * @brief Decrease log depth
 * @return The new log depth
 */
int decLogLevel();

#	define LOG_BEGIN(atReturn) \
		incLogLevel(__PRETTY_FUNCTION__); \
		BOOST_SCOPE_EXIT( ) { atReturn; decLogLevel(); } BOOST_SCOPE_EXIT_END;
#else
#	define LOG_BEGIN(atReturn)
#endif

/**
 * @def LOG_BEGIN(atReturn)
 * @ingroup Logger
 * @brief Begin a function trace
 * @details Uses BOOST_SCOPE_EXIT for automatically decreasing the log depth at scope exit
 * @param[in] atReturn Commands to execute before decreasing the log depth
 */

#ifndef NDEBUG
#	define LOG_DEBUG(fmt, ...) LOG(llDebug, fmt, ##__VA_ARGS__)
#else
#	define LOG_DEBUG(fmt, ...)
#endif

/**
 * @def LOG_DEBUG(msg)
 * @ingroup Logger
 * @brief Prints @a msg to stdout if @c NDEBUG is not defined, independent of the log level
 * @param[in] msg The message to print
 */

#endif // loggerH
