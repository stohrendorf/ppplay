#include <stuff/inputplugin.h>

#include "hscmodule.h"

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
        return "hsc";
    };
    virtual std::string description() const
    {
        return "HSC Module Loader";
    };
    virtual AbstractModule* load(Stream* stream, uint32_t frequency, int maxRepeat, Sample::Interpolation inter) const
    {
        return hsc::Module::factory(stream, frequency, maxRepeat, inter);
    };
};
}

EXPOSE_PLUGIN(ppp::Plugin)