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

#include "moduleregistry.h"

namespace ppp
{

ModuleRegistry::ModuleRegistry() : m_loaders()
{
}

ModuleRegistry& ModuleRegistry::instance()
{
	static ModuleRegistry registry;
	return registry;
}

void ModuleRegistry::registerLoader( LoadFunc func )
{
	ModuleRegistry& inst = instance();
	if( std::find( inst.m_loaders.begin(), inst.m_loaders.end(), func ) != inst.m_loaders.end() ) {
		std::cout << "Already...\n";
		return;
	}
	inst.m_loaders.push_back( func );
}

GenModule::Ptr ModuleRegistry::tryLoad( const std::string& filename, uint32_t frq, uint8_t maxRpt )
{
	GenModule::Ptr result;
	for( const LoadFunc & func : instance().m_loaders ) {
		if(( result = func( filename, frq, maxRpt ) )) {
			break;
		}
	}
	return result;
}

}