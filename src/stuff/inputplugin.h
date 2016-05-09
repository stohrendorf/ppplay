#ifndef PPPLAY_INPUTPLUGIN_H
#define PPPLAY_INPUTPLUGIN_H

#include <string>
#include <genmod/sample.h>

#if defined(_MSC_VER)
#define PPPLAY_PLUGIN_EXPORT __declspec(dllexport)
#elif defined(__GNUG__)
#define PPPLAY_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#error Unsupported compiler
#endif

#define EXPOSE_PLUGIN(class) \
    extern "C" { \
        PPPLAY_PLUGIN_EXPORT class plugin; \
    }

class Stream;

namespace ppp
{
class AbstractModule;

/**
 * @class InputPlugin
 * @brief Base class for input plugin interfaces
 */
class InputPlugin
{
public:
    //! @brief API version
    enum { Version = 4 };
    /**
     * @brief Virtual empty base destructor
     */
    virtual ~InputPlugin() { }
    /**
     * @brief API version
     * @return API version of the plugin
     * @note This should match @c Version, else the plugin is ABI incompatible
     */
    virtual int version() const = 0;
    /**
     * @brief Input plugin name (short)
     * @return E.g., "s3m" or "xm"
     */
    virtual std::string name() const = 0;
    /**
     * @brief Input plugin description
     * @return E.g., "ScreamTracker 3 Module Loader"
     */
    virtual std::string description() const = 0;
    /**
     * @brief Try to load a module file
     * @param[in] stream Data stream to load the module from
     * @param[in] frequency Rendering frequency
     * @param[in] maxRepeat Maximum repeat count
     * @param[in] inter Sample interpolation type
     * @retval NULL if the module could not be loaded by this plugin
     * @retval else a pointer to the loaded module
     */
    virtual AbstractModule* load( Stream* stream, uint32_t frequency, int maxRepeat, ppp::Sample::Interpolation inter ) const = 0;
};
}

#endif
