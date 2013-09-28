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

#include "pluginregistry.h"
#include "inputplugin.h"
#include "stream/archivefilestream.h"
#include "stream/filestream.h"

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <boost/filesystem.hpp>

namespace ppp
{

PluginRegistry::PluginRegistry() : m_handles()
{
}

PluginRegistry::~PluginRegistry()
{
    for(auto handle : m_handles) {
#ifdef WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
    }
}

PluginRegistry& PluginRegistry::instance()
{
	static PluginRegistry registry;
	return registry;
}

AbstractModule::Ptr PluginRegistry::tryLoad( const std::string& filename, uint32_t frq, int maxRpt, Sample::Interpolation inter )
{
	findPlugins();
	{
		FileStream file(filename);
		if( file.isOpen() ) {
            for(auto handle : instance().m_handles) {
#ifdef WIN32
                InputPlugin* plugin = reinterpret_cast<InputPlugin*>(GetProcAddress(handle, "plugin"));
#else
                InputPlugin* plugin = static_cast<InputPlugin*>(dlsym(handle, "plugin"));
#endif
				file.clear();
				file.seek(0);
				if( AbstractModule* result = plugin->load( &file, frq, maxRpt, inter ) ) {
					return AbstractModule::Ptr(result);
				}
			}
		}
	}
	{
		ArchiveFileStream file(filename);
		if( file.isOpen() ) {
            for(auto handle : instance().m_handles) {
#ifdef WIN32
                InputPlugin* plugin = reinterpret_cast<InputPlugin*>(GetProcAddress(handle, "plugin"));
#else
                InputPlugin* plugin = static_cast<InputPlugin*>(dlsym(handle, "plugin"));
#endif
				file.clear();
				file.seek(0);
				if( AbstractModule* result = plugin->load( &file, frq, maxRpt, inter ) ) {
					return AbstractModule::Ptr(result);
				}
			}
		}
	}
	return AbstractModule::Ptr();
}

void PluginRegistry::findPlugins()
{
	if(!instance().m_handles.empty()) {
		return;
	}
    boost::filesystem::path pluginPath(LIBEXECDIR);
    light4cxx::Logger::root()->debug(L4CXX_LOCATION, "Looking for plugins in %s", LIBEXECDIR);
	
    std::list<boost::filesystem::path> paths;
    std::copy(boost::filesystem::directory_iterator( pluginPath ), boost::filesystem::directory_iterator(), std::back_inserter(paths));
	for(const auto& entry : paths) {
#ifdef WIN32
        char nativeStr[260];
        wcstombs(nativeStr, entry.native().c_str(), 259);
        char nativeFnStr[260];
        wcstombs(nativeFnStr, entry.filename().native().c_str(), 259);
#endif
		// find every file in "../lib/ppplay" that begins with "libppplay_input_"
#ifdef WIN32
        light4cxx::Logger::root()->info(L4CXX_LOCATION, "Checking: %s", nativeFnStr);
        if(entry.filename().native().find(L"libppplay_input_")!=0)
            continue;
#else
        light4cxx::Logger::root()->info(L4CXX_LOCATION, "Checking: %s", entry.filename().native());
        if(entry.filename().native().find("libppplay_input_")!=0)
            continue;
#endif
		// try to load the file and look for the exported "plugin" symbol
#ifdef WIN32
        light4cxx::Logger::root()->info(L4CXX_LOCATION, "Trying to load input plugin: %s", nativeStr);
        HMODULE handle = LoadLibraryW(entry.c_str());
#else
        light4cxx::Logger::root()->info(L4CXX_LOCATION, "Trying to load input plugin: %s", entry.native());
        void* handle = dlopen(entry.c_str(), RTLD_LAZY);
#endif
		if(!handle) {
#ifdef WIN32
            light4cxx::Logger::root()->error(L4CXX_LOCATION, "Failed to load plugin '%s'", nativeStr);
#else
			light4cxx::Logger::root()->error(L4CXX_LOCATION, "Failed to load plugin '%s': %s", entry.native(), dlerror());
#endif
			continue;
		}
#ifdef WIN32
        InputPlugin* plugin = reinterpret_cast<InputPlugin*>(GetProcAddress(handle, "plugin"));
#else
		InputPlugin* plugin = static_cast<InputPlugin*>(dlsym(handle, "plugin"));
#endif
		if(!plugin) {
#ifdef WIN32
            light4cxx::Logger::root()->error(L4CXX_LOCATION, "Failed to load plugin '%s'", nativeStr);
            FreeLibrary(handle);
#else
			light4cxx::Logger::root()->error(L4CXX_LOCATION, "Failed to load plugin '%s': %s", entry.native(), dlerror());
            dlclose(handle);
#endif
			continue;
		}
		// verify that the API version matches
		int version = plugin->version();
		if(version != InputPlugin::Version) {
#ifdef WIN32
            light4cxx::Logger::root()->error(L4CXX_LOCATION, "Failed to load plugin '%s': API version mismatch. Expected %d, found %d.", nativeStr, InputPlugin::Version, version);
            FreeLibrary(handle);
#else
            light4cxx::Logger::root()->error(L4CXX_LOCATION, "Failed to load plugin '%s': API version mismatch. Expected %d, found %d.", entry.native(), InputPlugin::Version, version);
            dlclose(handle);
#endif
			continue;
		}
		// the plugin is valid, so add it to the list
#ifdef WIN32
        light4cxx::Logger::root()->debug(L4CXX_LOCATION, "Found input plugin '%s', '%s': %s", nativeStr, plugin->name(), plugin->description());
#else
        light4cxx::Logger::root()->debug(L4CXX_LOCATION, "Found input plugin '%s', '%s': %s", entry.native(), plugin->name(), plugin->description());
#endif
		instance().m_handles.push_back(handle);
	}
}

}
