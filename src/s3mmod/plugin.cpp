#include "stuff/inputplugin.h"

#include "s3mmodule.h"

namespace ppp
{
class Plugin : public InputPlugin
{
public:
	virtual int version() const {
		return Version;
	}
	virtual std::string name() const {
		return "s3m";
	};
	virtual std::string description() const {
		return "ScreamTracker 3 Module Loader";
	};
	virtual AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat) const {
		return s3m::S3mModule::factory(stream, frequency, maxRepeat);
	};
};
}

ppp::Plugin plugin;
