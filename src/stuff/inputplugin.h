#ifndef PPPLAY_INPUTPLUGIN_H
#define PPPLAY_INPUTPLUGIN_H

#include <string>
#include <genmod/sample.h>

#if defined(_MSC_VER)
#define PPPLAY_PLUGIN_EXPORT __declspec(dllexport)
#elif defined(__GNUG__)
#define PPPLAY_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#error Unsupported compiler
#endif

class Stream;

namespace ppp
{
class AbstractModule;

class InputPlugin
{
public:
	enum { Version = 3 };
	
	virtual ~InputPlugin() { }
	virtual int version() const = 0;
	virtual std::string name() const = 0;
	virtual std::string description() const = 0;
	virtual AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat, ppp::Sample::Interpolation inter ) const = 0;
};
}

#endif
