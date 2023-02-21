/*
    PPPlay - an old-fashioned module player
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

#include "location.h"

#include "level.h"
#include "logger.h"

#include <stdexcept>
#include <sstream>

#include <chrono>

namespace light4cxx
{
namespace
{
//! @brief The current runtime
std::chrono::high_resolution_clock::time_point s_bootTime = std::chrono::high_resolution_clock::now();

/**
 * @brief Get the current run-time in ms
 * @return Current run-time in ms
 */
long long timeSinceBootMs()
{
  std::chrono::high_resolution_clock::duration delta = std::chrono::high_resolution_clock::now() - s_bootTime;
  return std::chrono::duration_cast<std::chrono::milliseconds>( delta ).count();
}

/**
 * @brief The current message format
 */
std::string s_format = "[%T %<5t %r] %L (in %F:%l): %m";

#ifndef L4CXX_NO_ANSI_COLORS
#define COLOR_RESET  "\033[0m"
#define COLOR_RED    "\033[1;31m"
#define COLOR_GREEN  "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN   "\033[1;36m"
#else
#define COLOR_RESET
#define COLOR_RED
#define COLOR_GREEN
#define COLOR_YELLOW
#define COLOR_CYAN
#endif

/**
 * @brief Converts a Level to a string
 * @param[in] l The level to convert
 * @return String representation of @a l
 */
const char* levelString(Level l)
{
  switch( l )
  {
  case Level::Off:
    return "";
  case Level::Trace:
    return COLOR_CYAN "TRACE" COLOR_RESET;
  case Level::Debug:
    return COLOR_GREEN "DEBUG" COLOR_RESET;
  case Level::Info:
    return "INFO";
  case Level::Warn:
    return COLOR_YELLOW "WARN " COLOR_RESET;
  case Level::Error:
    return COLOR_RED "ERROR" COLOR_RESET;
  case Level::Fatal:
    return COLOR_RED "FATAL" COLOR_RESET;
  case Level::All:
    BOOST_THROW_EXCEPTION( std::runtime_error( "Logging level invalid: Level::All should not be passed to levelString()" ) );
  default:
    BOOST_THROW_EXCEPTION( std::runtime_error( "Logging level invalid: Unknown Level passed to levelString()" ) );
  }
}
} // anonymous namespace

void Location::setFormat(const std::string& fmt)
{
  s_format = fmt;
}

std::string Location::toString(light4cxx::Level l, const light4cxx::Logger& logger, const std::string& msg) const
{
  std::ostringstream oss;
  // 	const std::ios_base::fmtflags clearFlags = oss.flags();
  int state = 0;
  for( size_t i = 0; i < s_format.length(); i++ )
  {
    char c = s_format[i];
    switch( state )
    {
    case 0:
      // scan for %
      if( c == '%' )
      {
        state = 1;
        break;
      }
      oss << c;
      break;
    case 1:
      // flags
      switch( c )
      {
      case '#':
        oss << std::showbase;
        break;
      case '+':
        oss << std::showpos;
        break;
      case ',':
        oss << std::showpoint;
        break;
      case '0':
        oss.fill( '0' );
        break;
      case ' ':
        oss.fill( ' ' );
        break;
      case '<':
        oss << std::left;
        break;
      case '|':
        oss << std::internal;
        break;
      case '>':
        oss << std::right;
        break;
      case '=':
        oss << std::fixed;
        break;
      case '~':
        oss << std::scientific;
        break;
      default:
        state = 2;
        i--;
        break;
      }
      break;
    case 2:
      // field width
      if( isdigit( c ) )
      {
        oss.width( c - '0' );
      }
      else
      {
        i--;
      }
      state = 3;
      break;
    case 3:
      // precision dot
      if( c == '.' )
      {
        state = 4;
      }
      else
      {
        // no precision
        state = 5;
        i--;
      }
      break;
    case 4:
      // precision value
      if( isdigit( c ) )
      {
        oss.precision( c - '0' );
      }
      else
      {
        oss.precision( 0 );
        i--;
      }
      state = 5;
      break;
    case 5:
      // format specifier
      switch( c )
      {
      case '%':
        oss << '%';
        break;
      case 'f':
        oss << m_function;
        break;
      case 'F':
        oss << m_file;
        break;
      case 'l':
        oss << m_line;
        break;
      case 'L':
        oss << logger.name();
        break;
      case 'm':
        oss << msg;
        break;
      case 'r':
        oss << timeSinceBootMs() / 1000.0f;
        break;
      case 't':
        oss << levelString( l );
        break;
      case 'T':
        oss << std::hex << m_threadId;
        break;
      default:
        BOOST_THROW_EXCEPTION( std::runtime_error(
          std::string( "Unknown format specifier at: " ) + s_format.substr( i ) ) );
      }
      state = 0;
      oss.copyfmt( std::ostringstream() );
      /*				oss.flags( clearFlags );
                                      oss.fill(' ');
                                      oss.width(0);*/
      break;
    default:
      BOOST_THROW_EXCEPTION( std::runtime_error( "Invalid format parsing state" ) );
    }
  }
  return oss.str();
}
}