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
 * @defgroup Logger Logging facility
 * @{
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>

/**
 * @enum LogLevel
 * @brief Log level
 */
enum LogLevel {
	llMessage = 0, //!< @brief Log all messages
	llWarning, //!< @brief Log only warnings and errors
	llError, //!< @brief Log only errors
	llNone, //!< @brief Don't log anything
	llDebug = 0xff //!< @brief Debug message (ignore log level)
};

/**
 * @brief Log a message
 * @param[in] where Where the message occured
 * @param[in] msg Message to logger
 * @param[in] ll Log level of the message
 */
void logger( const std::string& where, const std::string& msg, LogLevel ll = llMessage );

/**
 * @brief Set the log level
 * @param[in] ll Log level
 */
void setLogLevel( LogLevel ll );

/**
 * @brief Get the current log level
 * @return The current log level
 */
LogLevel getLogLevel();

/**
 * @def LOG(msg, ...)
 * @brief Log a message
 * @param[in] ll Message's log level
 * @param[in] ... Format string and arguments
 */
#define LOG(ll, ...) logger(__PRETTY_FUNCTION__, ppp::stringf(__VA_ARGS__), ll)

/**
 * @def LOG_MESSAGE(...)
 * @brief Log a message with level LogLevel::llMessage
 * @param[in] ... Format string and arguments
 */
#define LOG_MESSAGE(...) LOG(llMessage, __VA_ARGS__)

/**
 * @def LOG_TEST_MESSAGE(condition)
 * @brief Log a message with level LogLevel::llMessage if @a condition is true
 * @param[in] condition Condition to test
 */
#define LOG_TEST_MESSAGE(condition) if(condition) LOG_MESSAGE("[LOG_TEST_MESSAGE] %s", #condition)

/**
 * @def LOG_WARNING(...)
 * @brief Log a message with level LogLevel::llWarning
 * @param[in] ... Format string and arguments
 */
#define LOG_WARNING(...) LOG(llWarning, __VA_ARGS__)

/**
 * @def LOG_TEST_WARN(condition)
 * @brief Log a message with level LogLevel::llWarning if @a condition is true
 * @param[in] condition Condition to test
 */
#define LOG_TEST_WARN(condition) if(condition) LOG_WARNING("[LOG_TEST_WARN] %s", #condition)

/**
 * @def LOG_ERROR(...)
 * @brief Log a message with level LogLevel::llError
 * @param[in] ... Format string and arguments
 */
#define LOG_ERROR(...) LOG(llError, __VA_ARGS__)

/**
 * @def LOG_TEST_ERROR(condition)
 * @brief Log a message with level LogLevel::llError if @a condition is true
 * @param[in] condition Condition to test
 */
#define LOG_TEST_ERROR(condition) if(condition) LOG_ERROR("[LOG_TEST_ERROR] %s", #condition)

#ifndef NDEBUG
#	define LOG_DEBUG(...) LOG(llDebug, __VA_ARGS__)
#else
#	define LOG_DEBUG(...)
#endif

/**
 * @def LOG_DEBUG(...)
 * @brief Log a debug message
 * @param[in] ... Format string and arguments
 * @note This resolves to a no-op when @c NDEBUG is defined
 */

/**
 * @}
 */

#endif // loggerH
