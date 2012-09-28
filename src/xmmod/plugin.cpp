#include <stuff/inputplugin.h>

#include "xmmodule.h"

namespace ppp
{
class Plugin : public InputPlugin
{
public:
	virtual int version() const {
		return Version;
	}
	virtual std::string name() const {
		return "xm";
	};
	virtual std::string description() const {
		return "FastTracker 2 Module Loader";
	};
	virtual AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat, Sample::Interpolation inter) const {
		return xm::XmModule::factory(stream,frequency,maxRepeat,inter);
	};
};
}

PPPLAY_PLUGIN_EXPORT ppp::Plugin plugin;
