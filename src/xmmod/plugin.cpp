#include <stuff/inputplugin.h>

#include "xmmodule.h"

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
        return "xm";
    };

    std::string description() const override
    {
        return "FastTracker 2 Module Loader";
    };

    AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat, Sample::Interpolation inter) const override
    {
        return xm::XmModule::factory(stream, frequency, maxRepeat, inter);
    };
};
}

EXPOSE_PLUGIN(ppp::Plugin)