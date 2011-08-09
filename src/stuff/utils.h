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

/**
 * @ingroup Common
 * @{
 */

#include <string>
#include <cstdint>
#include <algorithm>

namespace ppp {
/**
 * @brief Clip a value @a v in the range from @a a to @a b
 * @tparam T Value's type
 * @param[in] v Value to clip
 * @param[in] a Lower border
 * @param[in] b Upper border
 * @return Clipped value @a v between @a a and @a b
 * @note Time-critical
 */
template<typename T>
inline const T& clip(const T& v, const T& a, const T& b) {
	return std::min(b, std::max(v, a));
}

/**
 * @brief Check if @a v is between @a a and @a b
 * @tparam T Value's type
 * @param[in] v Value
 * @param[in] a Lower border
 * @param[in] b Upper border
 * @return True if @a v is between @a a and @a b
 * @note Time-critical
 */
template<typename T>
inline bool inRange(const T v, const T a, const T b) {
	return (v >= a) && (v <= b);
}

/**
 * @brief Helper function like printf, but returns a std::string
 * @param[in] fmt Format string
 * @param[in] ... Arguments
 * @return Formatted string
 * @note Time-critical
 */
std::string stringf(const char fmt[], ...) __attribute__((format(printf, 1, 2)));

/**
 * @brief Helper function like strncpy, but returns a std::string
 * @param[in] src Source string
 * @param[in] maxlen Maximum length of the string to copy
 * @return Copied string
 * @note Stops at the NUL character
 */
std::string stringncpy(const char src[], size_t maxlen);

/**
 * @brief Get low nibble of a byte
 * @param[in] x Value
 * @return Low nibble of @a x
 * @note Time-critical
 */
inline uint8_t lowNibble(uint8_t x) {
	return x & 0x0f;
}

/**
 * @brief Get high nibble of a byte
 * @param[in] x Value
 * @return High nibble of @a x
 * @note Time-critical
 */
inline uint8_t highNibble(uint8_t x) {
	return x >> 4;
}

/**
 * @brief Swap the bytes of @a data
 * @param[in,out] data Data to swap
 * @param[in] size Size of @a data
 * @details
 * Reverts the bytes in @a data
 */
void swapEndian(char data[], size_t size);

/**
 * @overload
 * @brief Swap the bytes of @a data
 * @tparam T Type of @a data
 * @param[in,out] data Data to swap
 */
template<class T>
void swapEndian(T* data) {
	swapEndian(reinterpret_cast<char*>(data), sizeof(T));
}

/**
 * @brief If one of the arguments is zero, set both to the other one
 * @param[in,out] oldFx If this is 0, use @a newFx
 * @param[in,out] newFx If this is 0, use @a oldFx
 */
inline void reuseIfZero(uint8_t& oldFx, uint8_t& newFx) {
	if(newFx == 0) {
		newFx = oldFx;
	}
	else {
		oldFx = newFx;
	}
}

/**
 * @brief If @a newFx is not 0, assign it to @a oldFx
 * @param[in,out] oldFx If @a newFx is not 0, assign @a newFx to this
 * @param[in] newFx If this is not 0, assign it to @a oldFx
 */
inline void reuseIfZeroEx(uint8_t& oldFx, uint8_t newFx) {
	if(newFx != 0) {
		oldFx = newFx;
	}
}

/**
 * @brief Works like reuseIfZero(), but uses nibbles instead
 * @param[in,out] oldFx Old data
 * @param[in,out] newFx New data
 */
inline void reuseNibblesIfZero(uint8_t& oldFx, uint8_t& newFx) {
	if(newFx == 0) {
		newFx = oldFx;
	}
	else if(highNibble(newFx) == 0) {
		oldFx = (newFx & 0x0f) | (oldFx & 0xf0);
	}
	else if(lowNibble(newFx) == 0) {
		oldFx = (newFx & 0xf0) | (oldFx & 0x0f);
	}
	else {
		oldFx = newFx;
	}
	newFx = oldFx;
}

/**
 * @brief Removes whitespaces from front and end of a string
 * @param[in] str String to trim
 * @return Trimmed string
 */
std::string trimString(const std::string& str);

}

/**
 * @def DISABLE_COPY(classname)
 * @brief Disables copy operator and constructor of @a classname
 * @param[in] classname Class name to disable copy functions of
 */
#define DISABLE_COPY(classname) \
	classname(const classname&) = delete; \
	classname& operator=(const classname&) = delete;

/**
 * @}
 */

#endif
