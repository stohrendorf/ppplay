
#include "utils.h"
#include <cstdarg>

namespace ppp {
	std::string stringf(const char *fmt, ...) {
		va_list args;
		va_start(args, fmt);
		char tmp[1024];
		vsnprintf(tmp, 1024, fmt, args);
		va_end(args);
		return tmp;
	}
	
	std::string stringncpy(const char src[], const std::size_t maxlen) {
		std::string res;
		for (std::size_t i = 0; i < maxlen; i++) {
			if (src[i] == 0x00)
				break;
			res += src[i];
		}
		return res;
	}
}

/**
 * @def TSPEC
 * @ingroup Common
 * @brief Helper macro for template specialization
 * @param[in] tn Typename
 */
#define TSPEC(tn) \
namespace std { \
	template const tn &min<tn>(const tn&, const tn&);\
	template const tn &max<tn>(const tn&, const tn&);\
} \
namespace ppp { \
	template const tn &clip<tn>(const tn&, const tn&, const tn&);\
	template bool inRange<tn>(const tn, const tn, const tn); \
}
TSPEC(int8_t)
TSPEC(int16_t)
TSPEC(int32_t)
TSPEC(int64_t)
TSPEC(uint8_t)
TSPEC(uint16_t)
TSPEC(uint32_t)
TSPEC(uint64_t)
TSPEC(float)
TSPEC(double)
#undef TSPEC
