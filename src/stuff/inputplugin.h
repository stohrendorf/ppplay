#ifndef INPUTPLUGIN_H
#define INPUTPLUGIN_H

#include "moduleregistry.h"

#include <string>

namespace ppp
{
class InputPlugin
{
public:
	enum { Version = 1 };
	
	virtual ~InputPlugin() { }
	virtual int version() const = 0;
	virtual std::string name() const = 0;
	virtual std::string description() const = 0;
	virtual ModuleRegistry::LoaderFunction loader() const = 0;
};
}

#endif
