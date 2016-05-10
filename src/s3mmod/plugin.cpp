#include <stuff/inputplugin.h>

#include "s3mmodule.h"

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
        return "s3m";
    };

    std::string description() const override
    {
        return "ScreamTracker 3 Module Loader";
    };

    AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat, Sample::Interpolation inter) const override
    {
        return s3m::S3mModule::factory(stream, frequency, maxRepeat, inter);
    };
};
}

EXPOSE_PLUGIN(ppp::Plugin)