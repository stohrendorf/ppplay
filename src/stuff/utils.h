/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UTILS_H
#define UTILS_H

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
inline const T& clip(const T& v, const T& a, const T& b) throw() {
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
std::string stringf(const char fmt[], ...) __attribute__((format(printf, 1, 2), nonnull(1)));

/**
 * @brief Helper function like strncpy, but returns a std::string
 * @ingroup Common
 * @param[in] src Source string
 * @param[in] maxlen Maximum length of the string to copy
 * @return Copied string
 * @note Stops at the NUL character
 */
std::string stringncpy(const char src[], std::size_t maxlen) __attribute__((nonnull(1)));

/**
 * @brief Get low nibble of a byte
 * @ingroup Common
 * @param[in] x Value
 * @return Low nibble of @a x
 * @note Time-critical
 */
inline uint8_t lowNibble(uint8_t x) throw() {
	return x & 0x0f;
}

/**
 * @brief Get high nibble of a byte
 * @ingroup Common
 * @param[in] x Value
 * @return High nibble of @a x
 * @note Time-critical
 */
inline uint8_t highNibble(uint8_t x) throw() {
	return x >> 4;
}

void swapEndian(char data[], std::size_t size);

template<class T>
void swapEndian(T* data) {
	swapEndian(reinterpret_cast<char*>(data), sizeof(T));
}

inline void reuseIfZero(uint8_t& oldFx, uint8_t& newFx) {
	if(newFx == 0)
		newFx = oldFx;
	else
		oldFx = newFx;
	newFx = oldFx;
}
inline void reuseIfZeroEx(uint8_t& oldFx, uint8_t newFx) {
	if(newFx != 0)
		oldFx = newFx;
}
inline void reuseNibblesIfZero(uint8_t& oldFx, uint8_t& newFx) {
	if(newFx == 0)
		newFx = oldFx;
	else if(highNibble(newFx) == 0)
		oldFx = (newFx & 0x0f) | (oldFx & 0xf0);
	else if(lowNibble(newFx) == 0)
		oldFx = (newFx & 0xf0) | (oldFx & 0x0f);
	else
		oldFx = newFx;
	newFx = oldFx;
}

std::string trimString(const std::string& str);
}

#define DISABLE_COPY(classname) \
	classname(const classname&) = delete; \
	classname& operator=(const classname&) = delete;

#endif
