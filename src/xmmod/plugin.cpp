#include "stuff/inputplugin.h"

#include "xmmodule.h"

namespace ppp
{
class Plugin : public InputPlugin
{
public:
	virtual int version() const {
		return 1;
	}
	virtual std::string name() const {
		return "xm";
	};
	virtual std::string description() const {
		return "FastTracker 2 Module Loader";
	};
	virtual ModuleRegistry::LoaderFunction loader() const {
		return &xm::XmModule::factory;
	};
};
}

ppp::Plugin plugin;
