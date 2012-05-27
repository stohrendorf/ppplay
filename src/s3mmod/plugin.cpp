#include "stuff/inputplugin.h"

#include "s3mmodule.h"

namespace ppp
{
class Plugin : public InputPlugin
{
public:
	virtual int version() const {
		return 1;
	}
	virtual std::string name() const {
		return "s3m";
	};
	virtual std::string description() const {
		return "ScreamTracker 3 Module Loader";
	};
	virtual ModuleRegistry::LoaderFunction loader() const {
		return &s3m::S3mModule::factory;
	};
};
}

ppp::Plugin plugin;
