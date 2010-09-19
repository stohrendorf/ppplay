/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef utilsH
#define utilsH

#include <string>
#include <cstdint>
#include <algorithm>

namespace ppp {
	/**
	 * @brief Clip a value @a v in the range from @a a to @a b
	 * @ingroup Common
	 * @tparam T Value's type
	 * @param[in] v Value to clip
	 * @param[in] a Lower border
	 * @param[in] b Upper border
	 * @return Clipped value @a v between @a a and @a b
	 * @note Time-critical
	 */
	template<typename T>
	inline const T &clip(const T &v, const T &a, const T &b) throw() {
		return std::min(b, std::max(v, a));
	}
	
	/**
	 * @brief Check if @a v is between @a a and @a b
	 * @ingroup Common
	 * @tparam T Integral type
	 * @param[in] v Value
	 * @param[in] a Lower border
	 * @param[in] b Upper border
	 * @return True if @a v is between @a a and @a b
	 * @note Time-critical
	 */
	template<typename T>
	inline bool inRange(const T v, const T a, const T b) throw() {
		return (v >= a) && (v <= b);
	}

	/**
	 * @brief Helper function like printf, but returns a std::string
	 * @ingroup Common
	 * @param[in] fmt Format string
	 * @return Formatted string
	 * @note Time-critical
	 */
	std::string stringf(const char *fmt, ...) __attribute__(( format(printf,1,2), nonnull(1) ));
	
	/**
	 * @brief Helper function like strncpy, but returns a std::string
	 * @ingroup Common
	 * @param[in] src Source string
	 * @param[in] maxlen Maximum length of the string to copy
	 * @return Copied string
	 * @note Stops at the NUL character
	 */
	std::string stringncpy(const char src[], const std::size_t maxlen) __attribute__(( nonnull(1) ));
	
	/**
	 * @brief Get low nibble of a byte
	 * @ingroup Common
	 * @param[in] x Value
	 * @return Low nibble of @a x
	 * @note Time-critical
	 */
	inline uint8_t lowNibble(const uint8_t x) throw() {
		return x & 0x0f;
	}
	
	/**
	 * @brief Get high nibble of a byte
	 * @ingroup Common
	 * @param[in] x Value
	 * @return High nibble of @a x
	 * @note Time-critical
	 */
	inline uint8_t highNibble(const uint8_t x) throw() {
		return x >> 4;
	}
}

#ifndef WITHIN_DOXYGEN
#define TSPEC(tn) \
extern template const tn &std::min<tn>(const tn&, const tn&);\
extern template const tn &std::max<tn>(const tn&, const tn&);\
extern template const tn &ppp::clip<tn>(const tn&, const tn&, const tn&);\
extern template bool ppp::inRange<tn>(const tn, const tn, const tn);
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
#endif

#define SHARED_PTR_DECL(tparam) \
extern template class std::shared_ptr< tparam >;
#define SHARED_PTR_IMPL(tparam) \
template class std::shared_ptr< tparam >;

#define VECTOR_DECL(tparam) \
extern template class std::vector< tparam >; \
extern template class std::allocator< tparam >;
#define VECTOR_IMPL(tparam) \
template class std::vector< tparam >; \
template class std::allocator< tparam >;

#endif
