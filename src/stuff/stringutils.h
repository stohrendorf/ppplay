/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef PPPLAY_STRINGUTILS_H
#define PPPLAY_STRINGUTILS_H

#include <boost/format.hpp>

/**
 * @ingroup Common
 * @{
 */

/**
 * @brief Helper function like strncpy, but returns a std::string
 * @param[in] src Source string
 * @param[in] maxlen Maximum length of the string to copy
 * @return Copied string
 * @note Stops at the NUL character
 */
inline std::string stringncpy(const char* src, size_t maxlen)
{
  std::string res;
  for( size_t i = 0; i < maxlen; i++ )
  {
    if( src[i] == 0x00 )
    {
      break;
    }
    res += src[i];
  }
  return res;
}

inline std::string stringFmt(boost::format& fmt)
{
  return fmt.str();
}

inline std::string stringFmt(std::string&& fmt)
{
  return fmt;
}

template<class T, class ...Args>
inline std::string stringFmt(boost::format& fmt, T&& val, Args&& ...args)
{
  return stringFmt( fmt % std::forward<T>( val ), std::forward<Args>( args )... );
}

template<class T, class ...Args>
inline std::string stringFmt(std::string&& fmt, T&& val, Args&& ...args)
{
  return stringFmt( boost::format( std::forward<std::string>( fmt ) ) % std::forward<T>( val ),
                    std::forward<Args>( args )... );
}

/**
 * @}
 */

#endif
