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

#ifndef AUDIOTYPES_H
#define AUDIOTYPES_H

/**
 * @ingroup Output
 * @{
 */

#include <queue>
#include <cstdint>
#include <memory>
#include <vector>

/**
 * @brief Basic sample
 * @details
 * Used for both internal sample storage and for output devices.
 */
typedef int16_t BasicSample;

/**
 * @brief Basic sample frame
 * @details
 * Combination of two BasicSample's for stereo storage
 */
#pragma pack(push, 1)
struct BasicSampleFrame {
	typedef std::vector<BasicSampleFrame> Vector;
	//! @brief Left sample value
	BasicSample left;
	//! @brief Right sample value
	BasicSample right;
	inline void mulRShift( uint8_t mul, uint8_t shift ) {
		mulRShift( mul, mul, shift );
	}
	inline void mulRShift( uint8_t mulLeft, uint8_t mulRight, uint8_t shift ) {
		left = ( left * mulLeft ) >> shift;
		right = ( right * mulRight ) >> shift;
	}
};
#pragma pack(pop)

/**
 * @brief Mixer sample
 * @details
 * Used for mixing BasicSample's so that there is no loss of sample data.
 * Operation should be to sum all BasicSample's into one MixerSample and @e then apply division.
 */
typedef int32_t MixerSample;

/**
 * @brief Mixer sample frame
 * @details
 * Combination of two MixerSample's for stereo storage
 */
#pragma pack(push,1)
struct MixerSampleFrame {
	//! @brief Left sample value
	MixerSample left;
	//! @brief Right sample value
	MixerSample right;
	inline MixerSampleFrame operator+=( const BasicSampleFrame& rhs ) {
		left += rhs.left;
		right += rhs.right;
		return *this;
	}
};
#pragma pack(pop)

/**
 * @brief Shared pointer to a vector of BasicSampleFrame's
 * @details
 * Due to the fact that a std::vector stores its data internally as an array,
 * this allows for easy iteration over the frames by grabbing a pointer to
 * the front element.
 */
typedef std::shared_ptr< std::vector<BasicSampleFrame> > AudioFrameBuffer;

/**
 * @brief Shared pointer to a vector of MixerSampleFrame's
 * @details
 * Due to the fact that a std::vector stores its data internally as an array,
 * this allows for easy iteration over the frames by grabbing a pointer to
 * the front element.
 */
typedef std::shared_ptr< std::vector<MixerSampleFrame> > MixerFrameBuffer;

/**
 * @brief Double-ended queue of AudioFrameBuffer's
 * @details
 * Used to create FIFO buffers.
 */
typedef std::queue< AudioFrameBuffer > AudioFrameBufferQueue;

/**
 * @}
 */

#endif // AUDIOTYPES_H
