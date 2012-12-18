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

#include <dlfcn.h>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

namespace ppp
{

PluginRegistry::PluginRegistry() : m_handles()
{
}

PluginRegistry::~PluginRegistry()
{
	for(void* handle : m_handles) {
		dlclose(handle);
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
			for(void* handle : instance().m_handles) {
				InputPlugin* plugin = static_cast<InputPlugin*>(dlsym(handle, "plugin"));
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
			for(void* handle : instance().m_handles) {
				InputPlugin* plugin = static_cast<InputPlugin*>(dlsym(handle, "plugin"));
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
	// get current exe path
	void* exe = dlopen(nullptr, RTLD_NOW);
	if(!exe) {
		light4cxx::Logger::root()->fatal(L4CXX_LOCATION, "Failed to get own executable handle: '%s'", dlerror());
		return;
	}
	Dl_info info;
	if(!dladdr("main", &info)) {
		light4cxx::Logger::root()->fatal(L4CXX_LOCATION, "Failed to get own executable info: '%s'", dlerror());
		dlclose(exe);
		return;
	}
	// go to "../lib/ppplay"
	boost::filesystem3::path pluginPath(info.dli_fname);
	dlclose(exe);
	pluginPath.remove_filename();
	pluginPath = pluginPath.parent_path();
	pluginPath /= "lib";
	pluginPath /= "ppplay";
	light4cxx::Logger::root()->debug(L4CXX_LOCATION, "Looking for plugins in %s", pluginPath.native());
	
	std::list<boost::filesystem3::path> paths;
	std::copy(boost::filesystem3::directory_iterator( pluginPath ), boost::filesystem3::directory_iterator(), std::back_inserter(paths));
	for(const auto& entry : paths) {
		// find every file in "../lib/ppplay" that begins with "libppplay_input_"
		light4cxx::Logger::root()->info(L4CXX_LOCATION, "Checking: %s", entry.filename().native());
		if(entry.filename().native().find("libppplay_input_")!=0) {
			continue;
		}
		// try to load the file and look for the exported "plugin" symbol
		light4cxx::Logger::root()->info(L4CXX_LOCATION, "Trying to load input plugin: %s", entry.native());
		void* handle = dlopen(entry.c_str(), RTLD_LAZY);
		if(!handle) {
			light4cxx::Logger::root()->error(L4CXX_LOCATION, "Failed to load plugin '%s': %s", entry.native(), dlerror());
			continue;
		}
		InputPlugin* plugin = static_cast<InputPlugin*>(dlsym(handle, "plugin"));
		if(!plugin) {
			light4cxx::Logger::root()->error(L4CXX_LOCATION, "Failed to load plugin '%s': %s", entry.native(), dlerror());
			dlclose(handle);
			continue;
		}
		// verify that the API version matches
		int version = plugin->version();
		if(version != InputPlugin::Version) {
			light4cxx::Logger::root()->error(L4CXX_LOCATION, "Failed to load plugin '%s': API version mismatch. Expected %d, found %d.", entry.native(), InputPlugin::Version, version);
			dlclose(handle);
			continue;
		}
		// the plugin is valid, so add it to the list
		light4cxx::Logger::root()->debug(L4CXX_LOCATION, "Found input plugin '%s', '%s': %s", entry.native(), plugin->name(), plugin->description());
		instance().m_handles.push_back(handle);
	}
}

}
