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

#ifndef MODULEREGISTRY_H
#define MODULEREGISTRY_H

#include "utils.h"

#include "genmod/genmodule.h"
#include <list>

namespace ppp
{

class ModuleRegistry
{
	DISABLE_COPY( ModuleRegistry )
private:
	ModuleRegistry();
	typedef GenModule::Ptr( *LoadFunc )( const std::string& filename, uint32_t frequency, uint8_t maxRepeat );
	std::list<LoadFunc> m_loaders;
public:
	static ModuleRegistry& instance();
	static void registerLoader( ppp::ModuleRegistry::LoadFunc func );
	static GenModule::Ptr tryLoad( const std::string& filename, uint32_t frq, uint8_t maxRpt );
};

}

#endif
