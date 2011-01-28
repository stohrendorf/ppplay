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
	BasicSample left; //!< @brief Left channel sample
	BasicSample right; //!< @brief Right channel sample
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
	MixerSample left; //!< @brief Left channel sample
	MixerSample right; //!< @brief Right channel sample
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