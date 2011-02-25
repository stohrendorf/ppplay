/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#include <vector>
#include <deque>
#include <memory>

/**
 * @file
 * @ingroup Output
 * @brief Basic types for the audio system
 */

/**
 * @ingroup Output
 * @brief Basic sample
 * @details
 * Used for both internal sample storage and for output devices.
 */
typedef int16_t BasicSample;
/**
 * @ingroup Output
 * @brief Basic sample frame
 * @details
 * Combination of two BasicSample's for stereo storage
 */
#pragma pack(push, 1)
struct BasicSampleFrame {
	BasicSample left;
	BasicSample right;
};
#pragma pack(pop)
/**
 * @ingroup Output
 * @brief Mixer sample
 * @details
 * Used for mixing BasicSample's so that there is no loss of sample data.
 * Operation should be to sum all BasicSample's into one MixerSample and @e then apply division.
 */
typedef int32_t MixerSample;
/**
 * @ingroup Output
 * @brief Mixer sample frame
 * @details
 * Combination of two MixerSample's for stereo storage
 */
#pragma pack(push,1)
struct MixerSampleFrame {
	MixerSample left;
	MixerSample right;
};
#pragma pack(pop)
/**
 * @ingroup Output
 * @brief Shared pointer to a vector of BasicSampleFrame's
 * @details
 * Due to the fact that a std::vector stores its data internally as an array,
 * this allows for easy iteration over the frames by grabbing a pointer to
 * the front element.
 */
typedef std::shared_ptr< std::vector<BasicSampleFrame> > AudioFrameBuffer;
/**
 * @ingroup Output
 * @brief Shared pointer to a vector of MixerSampleFrame's
 * @details
 * Due to the fact that a std::vector stores its data internally as an array,
 * this allows for easy iteration over the frames by grabbing a pointer to
 * the front element.
 */
typedef std::shared_ptr< std::vector<MixerSampleFrame> > MixerFrameBuffer;
/**
 * @ingroup Output
 * @brief Double-ended queue of AudioFrameBuffer's
 * @details
 * Used to create FIFO buffers.
 */
typedef std::deque< AudioFrameBuffer > AudioFrameBufferQueue;

#endif // AUDIOTYPES_H
