#include <stuff/inputplugin.h>

#include "modmodule.h"

namespace ppp
{
class Plugin : public InputPlugin
{
public:
    virtual int version() const
    {
        return Version;
    }
    virtual std::string name() const
    {
        return "mod";
    };
    virtual std::string description() const
    {
        return "Protracker Module Loader";
    };
    virtual AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat, Sample::Interpolation inter) const
    {
        return mod::ModModule::factory(stream, frequency, maxRepeat, inter);
    };
};
}

EXPOSE_PLUGIN(ppp::Plugin)