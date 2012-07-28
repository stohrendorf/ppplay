#ifndef PPPLAY_INPUTPLUGIN_H
#define PPPLAY_INPUTPLUGIN_H

#include <string>

class Stream;

namespace ppp
{
class AbstractModule;

class InputPlugin
{
public:
	enum { Version = 2 };
	
	virtual ~InputPlugin() { }
	virtual int version() const = 0;
	virtual std::string name() const = 0;
	virtual std::string description() const = 0;
	virtual AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat) const = 0;
};
}

#endif
