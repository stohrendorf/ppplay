#include "system.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace ppp
{
std::string whereAmI()
{
    char tmp[1024];
    std::string path;
#ifdef WIN32
    auto selfPathLen = GetModuleFileNameA(nullptr, selfPath, sizeof(selfPath));
    if(selfPathLen == sizeof(selfPath)) {
        return std::string();
    }
#else
    // this is not NUL-terminated.
    auto selfPathLen = readlink("/proc/self/exe", tmp, sizeof(tmp)-1);
    if(selfPathLen == -1) {
        return std::string();
    }
    tmp[selfPathLen] = '\0';
#endif
    path = tmp;
    auto idx = path.find_last_of("\\/");
    return path.substr(0, idx);
}
}
