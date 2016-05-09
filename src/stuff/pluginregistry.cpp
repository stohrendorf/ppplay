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

#include "pluginregistry.h"
#include "inputplugin.h"
#include "stream/archivefilestream.h"
#include "stream/filestream.h"
#include "stuff/system.h"

#ifdef WIN32
#include <windows.h>
#include <cwchar>
#else
#include <dlfcn.h>
#endif

namespace ppp
{

namespace
{
#ifdef WIN32
std::string wideToMultibyte(const std::wstring& ws)
{
    char tmp[1024];
    mbstate_t state;
    mbrlen(nullptr, 0, &state);
    auto ptr = ws.c_str();
    auto len = wcsrtombs( tmp, &ptr, sizeof(tmp)-1, &state );
    tmp[sizeof(tmp)-1] = '\0';
    if(len == size_t(-1))
        return std::string();
    return tmp;
}
#endif

light4cxx::Logger* logger()
{
    static light4cxx::Logger* l = light4cxx::Logger::get("pluginregistry");
    return l;
}
}

PluginRegistry::PluginRegistry() : m_handles(), m_searchPath(LIBEXECDIR)
{
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
    AddDllDirectory(boost::filesystem::path(ppp::whereAmI()).parent_path().native().c_str());
}

PluginRegistry::~PluginRegistry()
{
    for( auto handle : m_handles ) {
#ifdef WIN32
        FreeLibrary( handle );
#else
        dlclose( handle );
#endif
    }
}

PluginRegistry& PluginRegistry::instance()
{
    static PluginRegistry registry;
    return registry;
}

void PluginRegistry::setSearchPath( const boost::filesystem::path& path )
{
#ifdef WIN32
    logger()->debug(L4CXX_LOCATION, "Setting plugin search path to: %s", wideToMultibyte(path.native()));
#else
    logger()->debug(L4CXX_LOCATION, "Setting plugin search path to: %s", path);
#endif
    m_searchPath = path;
}

AbstractModule::Ptr PluginRegistry::tryLoad( const std::string& filename, uint32_t frq, int maxRpt, Sample::Interpolation inter )
{
    findPlugins();
    {
        FileStream file( filename );
        if( file.isOpen() ) {
            for( auto handle : instance().m_handles ) {
#ifdef WIN32
                InputPlugin* plugin = reinterpret_cast<InputPlugin*>( GetProcAddress( handle, "plugin" ) );
#else
                InputPlugin* plugin = static_cast<InputPlugin*>( dlsym( handle, "plugin" ) );
#endif
                file.clear();
                file.seek( 0 );
                logger()->debug(L4CXX_LOCATION, "Trying to load using input plugin %s", plugin->name());
                if( AbstractModule* result = plugin->load( &file, frq, maxRpt, inter ) ) {
                    return AbstractModule::Ptr( result );
                }
            }
        }
    }
    {
        ArchiveFileStream file( filename );
        if( file.isOpen() ) {
            for( auto handle : instance().m_handles ) {
#ifdef WIN32
                InputPlugin* plugin = reinterpret_cast<InputPlugin*>( GetProcAddress( handle, "plugin" ) );
#else
                InputPlugin* plugin = static_cast<InputPlugin*>( dlsym( handle, "plugin" ) );
#endif
                file.clear();
                file.seek( 0 );
                if( AbstractModule* result = plugin->load( &file, frq, maxRpt, inter ) ) {
                    return AbstractModule::Ptr( result );
                }
            }
        }
    }
    return AbstractModule::Ptr();
}

void PluginRegistry::findPlugins()
{
    if( !instance().m_handles.empty() ) {
        return;
    }
#ifdef WIN32
    logger()->debug( L4CXX_LOCATION, "Looking for plugins in %s", wideToMultibyte(instance().m_searchPath.native()) );
#else
    logger()->debug( L4CXX_LOCATION, "Looking for plugins in %s", instance().m_searchPath );
#endif

    std::list<boost::filesystem::path> paths;
    std::copy( boost::filesystem::directory_iterator( instance().m_searchPath ), boost::filesystem::directory_iterator(), std::back_inserter( paths ) );
    for( const auto& entry : paths ) {
        // find every file in m_searchPath that begins with "libppplay_input_"
#ifdef WIN32
        logger()->info( L4CXX_LOCATION, "Checking: %s", wideToMultibyte(entry.filename().native()) );
        if( entry.filename().native().find( L"ppplay_input_" ) != 0 )
            continue;
#else
        logger()->info( L4CXX_LOCATION, "Checking: %s", entry.filename().native() );
        if( entry.filename().native().find( "libppplay_input_" ) != 0 )
            continue;
#endif
        // try to load the file and look for the exported "plugin" symbol
#ifdef WIN32
        logger()->info( L4CXX_LOCATION, "Trying to load input plugin: %s", wideToMultibyte(entry.native()) );
        HMODULE handle = LoadLibraryW( entry.c_str() );
#else
        logger()->info( L4CXX_LOCATION, "Trying to load input plugin: %s", entry.native() );
        void* handle = dlopen( entry.c_str(), RTLD_LAZY );
#endif
        if( !handle ) {
#ifdef WIN32
            logger()->error( L4CXX_LOCATION, "Failed to load plugin '%s' (error %d)", wideToMultibyte(entry.native()), GetLastError());
#else
            logger()->error( L4CXX_LOCATION, "Failed to load plugin '%s': %s", entry.native(), dlerror() );
#endif
            continue;
        }
#ifdef WIN32
        InputPlugin* plugin = reinterpret_cast<InputPlugin*>( GetProcAddress( handle, "plugin" ) );
#else
        InputPlugin* plugin = static_cast<InputPlugin*>( dlsym( handle, "plugin" ) );
#endif
        if( !plugin ) {
#ifdef WIN32
            logger()->error( L4CXX_LOCATION, "Failed to load plugin '%s' (missing 'plugin' structure, error %d)", wideToMultibyte(entry.native()), GetLastError());
            FreeLibrary( handle );
#else
            logger()->error( L4CXX_LOCATION, "Failed to load plugin '%s': %s", entry.native(), dlerror() );
            dlclose( handle );
#endif
            continue;
        }
        // verify that the API version matches
        int version = plugin->version();
        if( version != InputPlugin::Version ) {
#ifdef WIN32
            logger()->error( L4CXX_LOCATION, "Failed to load plugin '%s': API version mismatch. Expected %d, found %d.", wideToMultibyte(entry.native()), InputPlugin::Version, version );
            FreeLibrary( handle );
#else
            logger()->error( L4CXX_LOCATION, "Failed to load plugin '%s': API version mismatch. Expected %d, found %d.", entry.native(), InputPlugin::Version, version );
            dlclose( handle );
#endif
            continue;
        }
        // the plugin is valid, so add it to the list
#ifdef WIN32
        logger()->debug( L4CXX_LOCATION, "Found input plugin '%s', '%s': %s", wideToMultibyte(entry.native()), plugin->name(), plugin->description() );
#else
        logger()->debug( L4CXX_LOCATION, "Found input plugin '%s', '%s': %s", entry.native(), plugin->name(), plugin->description() );
#endif
        instance().m_handles.emplace_back( handle );
    }
}

}
