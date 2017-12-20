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

#ifndef PPPLAY_PLUGINREGISTRY_H
#define PPPLAY_PLUGINREGISTRY_H

#include <genmod/abstractmodule.h>
#include "ppplay_core_export.h"

#include <list>

#ifdef WIN32
#include <windows.h>
#endif

#include <boost/filesystem.hpp>

namespace ppp
{
class PPPLAY_CORE_EXPORT PluginRegistry
{
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
#ifdef WIN32
    std::list<HMODULE> m_handles;
#else
    std::list<void*> m_handles;
#endif
    boost::filesystem::path m_searchPath;

    /**
     * @brief Looks for plugins in @c m_searchPath that start with "libppplay_input_"
     */
    static void findPlugins();

public:
    DISABLE_COPY(PluginRegistry)

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
     * @return The loaded file or a nullptr if an error occured
     */
    static AbstractModule::Ptr tryLoad(const std::string& filename, uint32_t frq, int maxRpt, Sample::Interpolation inter);

    void setSearchPath(const boost::filesystem::path& path);
};
}

#endif
