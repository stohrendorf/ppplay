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
#include "inputplugin.h"

#include <dlfcn.h>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

namespace ppp
{

ModuleRegistry::ModuleRegistry() : m_handles()
{
}

ModuleRegistry::~ModuleRegistry()
{
	for(void* handle : m_handles) {
		dlclose(handle);
	}
}

ModuleRegistry& ModuleRegistry::instance()
{
	static ModuleRegistry registry;
	return registry;
}

GenModule::Ptr ModuleRegistry::tryLoad( const std::string& filename, uint32_t frq, int maxRpt )
{
	findModules();
	GenModule::Ptr result;
	for(void* handle : instance().m_handles) {
		InputPlugin* plugin = static_cast<InputPlugin*>(dlsym(handle, "plugin"));
		LoaderFunction loader = plugin->loader();
		if( ( result = loader( filename, frq, maxRpt ) ) ) {
			break;
		}
	}
	return result;
}

void ModuleRegistry::findModules()
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
	boost::filesystem3::path p(info.dli_fname);
	dlclose(exe);
	p.remove_filename();
	p = p.parent_path();
	p /= "lib";
	p /= "ppplay";
	light4cxx::Logger::root()->debug(L4CXX_LOCATION, "Looking for modules in %s", p.native());
	
	std::list<boost::filesystem3::path> paths;
	std::copy(boost::filesystem3::directory_iterator(p), boost::filesystem3::directory_iterator(), std::back_inserter(paths));
	for(const auto& entry : paths) {
		light4cxx::Logger::root()->info(L4CXX_LOCATION, "Checking: %s", entry.filename().native());
		if(entry.filename().native().find("libppplay_input_")!=0) {
			continue;
		}
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
		int version = plugin->version();
		if(version != InputPlugin::Version) {
			light4cxx::Logger::root()->error(L4CXX_LOCATION, "Failed to load plugin '%s': API version mismatch. Expected %d, found %d.", entry.native(), InputPlugin::Version, version);
			dlclose(handle);
			continue;
		}
		light4cxx::Logger::root()->debug(L4CXX_LOCATION, "Found input plugin '%s', '%s': %s", entry.native(), plugin->name(), plugin->description());
		instance().m_handles.push_back(handle);
	}
}

}
