#include <stuff/inputplugin.h>

#include "itmodule.h"

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
        return "it";
    };

    std::string description() const override
    {
        return "Impulse Tracker Module Loader";
    };

    AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat, Sample::Interpolation inter) const override
    {
        return it::ItModule::factory(stream, frequency, maxRepeat, inter);
    };
};
}

EXPOSE_PLUGIN(ppp::Plugin)
