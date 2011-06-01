/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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
 * @defgroup Logger Logging facility
 * @brief This module contains the logger definitions
 */

/**
 * @file
 * @ingroup Logger
 * @brief Logger definitions (header)
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#include "stuff/utils.h"

/**
 * @enum LogLevel
 * @ingroup Logger
 * @brief Log level
 */
enum LogLevel {
	llMessage = 0, //!< @brief Log all messages
	llWarning, //!< @brief Log only warnings and errors
	llError, //!< @brief Log only errors
	llNone, //!< @brief Don't log anything
	llDebug = 0xff
};

/**
 * @ingroup Logger
 * @brief Log a message
 * @param[in] msg Message to logger
 * @param[in] ll Log level of the message
 */
void logger( const std::string& where, const std::string& msg, LogLevel ll = llMessage );

/**
 * @ingroup Logger
 * @brief Set the log level
 * @param[in] ll Log level
 */
void setLogLevel( LogLevel ll );

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
 * @param[in] ll Message's log level
 * @param[in] fmt Message
 */
#define LOG(ll, ...) logger(__PRETTY_FUNCTION__, ppp::stringf(__VA_ARGS__), ll)

/**
 * @ingroup Logger
 * @brief Log a message with log level ::llMessage
 * @param[in] fmt Message
 */
#define LOG_MESSAGE(...) LOG(llMessage, __VA_ARGS__)
#define LOG_TEST_MESSAGE(condition) if(condition) LOG_MESSAGE("[LOG_TEST_MESSAGE] %s", #condition)

/**
 * @ingroup Logger
 * @brief Log a message with log level ::llWarning
 * @param[in] fmt Message
 */
#define LOG_WARNING(...) LOG(llWarning, __VA_ARGS__)
#define LOG_TEST_WARN(condition) if(condition) LOG_WARNING("[LOG_TEST_WARN] %s", #condition)

/**
 * @ingroup Logger
 * @brief Log a message with log level ::llError
 * @param[in] fmt Message
 */
#define LOG_ERROR(...) LOG(llError, __VA_ARGS__)
#define LOG_TEST_ERROR(condition) if(condition) LOG_ERROR("[LOG_TEST_ERROR] %s", #condition)

#ifndef NDEBUG
#	define LOG_DEBUG(...) LOG(llDebug, __VA_ARGS__)
#else
#	define LOG_DEBUG(...)
#endif

/**
 * @def LOG_DEBUG(msg)
 * @ingroup Logger
 * @brief Prints @a msg to stdout if @c NDEBUG is not defined, independent of the log level
 * @param[in] msg The message to print
 */

#endif // loggerH
