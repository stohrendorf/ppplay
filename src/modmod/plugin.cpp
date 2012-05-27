#include "stuff/inputplugin.h"

#include "modmodule.h"

namespace ppp
{
class Plugin : public InputPlugin
{
public:
	virtual int version() const {
		return 1;
	}
	virtual std::string name() const {
		return "mod";
	};
	virtual std::string description() const {
		return "Protracker Module Loader";
	};
	virtual ModuleRegistry::LoaderFunction loader() const {
		return &mod::ModModule::factory;
	};
};
}

ppp::Plugin plugin;
