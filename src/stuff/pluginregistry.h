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

#ifndef PPPLAY_PLUGINREGISTRY_H
#define PPPLAY_PLUGINREGISTRY_H

#include <genmod/abstractmodule.h>
#include "ppplay_core_export.h"

#include <list>

namespace ppp
{

class PPPLAY_CORE_EXPORT PluginRegistry
{
	DISABLE_COPY( PluginRegistry )
private:
	/**
	 * @brief Default constructor
	 */
	PluginRegistry();
	/**
	 * @brief Frees the plugin handles
	 */
	~PluginRegistry();
	/**
	 * @brief The plugins found by findPlugins()
	 */
	std::list<void*> m_handles;
	/**
	 * @brief Looks for plugins in "../lib/ppplay" that start with "libppplay_input_"
	 */
	static void findPlugins();
public:
	/**
	 * @brief Singleton design pattern
	 * @return Single registry instance
	 */
	static PluginRegistry& instance();
	/**
	 * @brief Try to load a module file
	 * @param[in] filename Filename of the module file to load
	 * @param[in] frq Desired rendering frequency
	 * @param[in] maxRpt Maximum repeat count (for length calculation)
	 * @param[in] inter Sample interpolation type
	 * @return The loaded file or a NULL pointer if an error occured
	 */
	static AbstractModule::Ptr tryLoad( const std::string& filename, uint32_t frq, int maxRpt, Sample::Interpolation inter );
};

}

#endif