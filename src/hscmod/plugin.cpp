#include <stuff/inputplugin.h>

#include "hscmodule.h"

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
        return "hsc";
    };

    std::string description() const override
    {
        return "HSC Module Loader";
    };

    AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat, Sample::Interpolation inter) const override
    {
        return hsc::Module::factory(stream, frequency, maxRepeat, inter);
    };
};
}

EXPOSE_PLUGIN(ppp::Plugin)