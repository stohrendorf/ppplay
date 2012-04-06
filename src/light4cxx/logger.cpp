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

#include "logger.h"

#include <iostream>
#include <unordered_map>
#include <boost/exception/all.hpp>
#include <boost/thread.hpp>

/**
 * @ingroup light4cxx
 * @{
 */

namespace light4cxx
{

namespace
{
/**
 * @brief The current logging level filter
 */
Level s_level = Level::Debug;
}

Level Logger::level()
{
	return s_level;
}

void Logger::setLevel( Level l )
{
	s_level = l;
}

Logger::Ptr Logger::root()
{
	return get( "root" );
}

Logger::Ptr Logger::get( const std::string& name )
{
	typedef std::unordered_map<std::string, Logger::Ptr> RepoMap; //!< @brief Maps logger names to their instances
	static RepoMap s_repository; //!< @brief The logger repository
	static boost::recursive_mutex lockMutex; //!< @brief Mutex for locking the repository
	boost::recursive_mutex::scoped_lock lockGuard( lockMutex );

	RepoMap::const_iterator elem = s_repository.find( name );
	if( elem != s_repository.end() ) {
		return elem->second;
	}
	Ptr res( new Logger( name ) );
	s_repository.insert( std::make_pair( name, res ) );
	return res;
}

Logger::Logger( const std::string& name ) : m_name( name )
{
}

void Logger::log( light4cxx::Level l, const light4cxx::Location& loc, const std::string& str ) const
{
	if( l < s_level || s_level == Level::Off ) {
		return;
	}
	static boost::recursive_mutex outMutex;
	boost::recursive_mutex::scoped_lock outLock( outMutex );
	std::cout << loc.toString( l, *this, str );
}

void Logger::log( Level l, const Location& loc, const boost::format& fmt ) const
{
	log( l, loc, fmt.str() );
}

}

/**
 * @}
 */
