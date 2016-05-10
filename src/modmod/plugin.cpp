#include <stuff/inputplugin.h>

#include "modmodule.h"

namespace ppp
{
class Plugin : public InputPlugin
{
public:
    int version() const override
    {
        return Version;
    }

    std::string name() const override
    {
        return "mod";
    };

    std::string description() const override
    {
        return "Protracker Module Loader";
    };

    AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat, Sample::Interpolation inter) const override
    {
        return mod::ModModule::factory(stream, frequency, maxRepeat, inter);
    };
};
}

EXPOSE_PLUGIN(ppp::Plugin)