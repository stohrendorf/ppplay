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

#include "stuff/numberutils.h"

#include <queue>
#include <cstdint>
#include <memory>
#include <vector>

/**
 * @ingroup Output
 * @{
 */

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
	//! @brief Vector of BasicSampleFrame's
	typedef std::vector<BasicSampleFrame> Vector;
	//! @brief Left sample value
	BasicSample left;
	//! @brief Right sample value
	BasicSample right;
	/**
	 * @brief Multiply and shift right
	 * @param[in] mul Factor
	 * @param[in] shift Amount to shift right
	 */
	inline void mulRShift( int mul, int shift ) {
		mulRShift( mul, mul, shift );
	}
	/**
	 * @overload
	 * @brief Multiply and shift right
	 * @param[in] mulLeft Factor for left data
	 * @param[in] mulRight Factor for right data
	 * @param[in] shift Amount to shift right
	 */
	inline void mulRShift( int mulLeft, int mulRight, int shift ) {
		left = ( left * mulLeft ) >> shift;
		right = ( right * mulRight ) >> shift;
	}

	constexpr BasicSampleFrame() : left( 0 ), right( 0 ) {
	}
	constexpr BasicSampleFrame(int16_t l, int16_t r) : left( l ), right( r ) {
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
	/**
	 * @brief Add a BasicSampleFrame
	 * @param[in] rhs BasicSampleFrame to add
	 * @return Copy of *this
	 */
	inline MixerSampleFrame operator+=( const BasicSampleFrame& rhs ) {
		left += rhs.left;
		right += rhs.right;
		return *this;
	}
	/**
	 * @brief Shift data right and clip
	 * @param[in] shift Amount to shift right
	 * @return Clipped BasicSampleFrame
	 */
	inline BasicSampleFrame rightShiftClip( uint8_t shift ) const {
		BasicSampleFrame result;
		result.left = ppp::clip( left >> shift, -32768, 32767 );
		result.right = ppp::clip( right >> shift, -32768, 32767 );
		return result;
	}

	constexpr MixerSampleFrame() : left( 0 ), right( 0 ) {
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
extern template class std::vector<BasicSampleFrame>;
extern template class std::shared_ptr< std::vector<BasicSampleFrame> >;

/**
 * @brief Shared pointer to a vector of MixerSampleFrame's
 * @details
 * Due to the fact that a std::vector stores its data internally as an array,
 * this allows for easy iteration over the frames by grabbing a pointer to
 * the front element.
 */
typedef std::shared_ptr< std::vector<MixerSampleFrame> > MixerFrameBuffer;
extern template class std::vector<MixerSampleFrame>;
extern template class std::shared_ptr< std::vector<MixerSampleFrame> >;

/**
 * @brief Double-ended queue of AudioFrameBuffer's
 * @details
 * Used to create FIFO buffers.
 */
typedef std::queue< AudioFrameBuffer > AudioFrameBufferQueue;
extern template class std::queue<AudioFrameBuffer>;

/**
 * @}
 */

#endif // AUDIOTYPES_H
