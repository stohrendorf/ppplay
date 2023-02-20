/*
    PPPlay - an old-fashioned module player
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

#ifndef PPPLAY_AUDIOTYPES_H
#define PPPLAY_AUDIOTYPES_H

#include <stuff/numberutils.h>

#include <cstdint>
#include <memory>
#include <queue>
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

struct BasicSampleFrame
{
  //! @brief Vector of BasicSampleFrame's
  typedef std::vector<BasicSampleFrame> Vector;
  //! @brief Left sample value
  BasicSample left = 0;
  //! @brief Right sample value
  BasicSample right = 0;

  constexpr BasicSampleFrame() noexcept = default;

  constexpr BasicSampleFrame(int16_t l, int16_t r) noexcept
    : left( l ), right( r )
  {}

  constexpr BasicSampleFrame operator-(const BasicSampleFrame& rhs) const
  noexcept
  {
    return BasicSampleFrame( left - rhs.left, right - rhs.right );
  }

  constexpr BasicSampleFrame operator*(int value) const noexcept
  {
    return BasicSampleFrame( left * value, right * value );
  }

  constexpr BasicSampleFrame operator>>(int shift) const noexcept
  {
    return BasicSampleFrame( left >> shift, right >> shift );
  }

  inline BasicSampleFrame& operator+=(const BasicSampleFrame& rhs) noexcept
  {
    left += rhs.left;
    right += rhs.right;
    return *this;
  }
};

#pragma pack(pop)

/**
 * @brief Mixer sample
 * @details
 * Used for mixing BasicSample's so that there is no loss of sample data.
 * Operation should be to sum all BasicSample's into one MixerSample and @e then
 * apply division.
 */
typedef int32_t MixerSample;

/**
 * @brief Mixer sample frame
 * @details
 * Combination of two MixerSample's for stereo storage
 */
#pragma pack(push, 1)

struct MixerSampleFrame
{
  //! @brief Left sample value
  MixerSample left = 0;
  //! @brief Right sample value
  MixerSample right = 0;

  constexpr explicit MixerSampleFrame(const BasicSampleFrame& f) noexcept
    : MixerSampleFrame{ f.left, f.right }
  {}

  constexpr explicit MixerSampleFrame(MixerSample left,
                                      MixerSample right) noexcept
    : left{ left }, right{ right }
  {}

  /**
   * @brief Add a BasicSampleFrame
   * @param[in] rhs BasicSampleFrame to add
   * @return Copy of *this
   */
  inline MixerSampleFrame operator+=(const BasicSampleFrame& rhs) noexcept
  {
    left += rhs.left;
    right += rhs.right;
    return *this;
  }

  /**
   * @brief Shift data right and clip
   * @param[in] shift Amount to shift right
   * @return Clipped BasicSampleFrame
   */
  inline BasicSampleFrame rightShiftClip(uint8_t shift) const noexcept
  {
    BasicSampleFrame result;
    result.left = ppp::clip( left >> shift, -32768, 32767 );
    result.right = ppp::clip( right >> shift, -32768, 32767 );
    return result;
  }

  constexpr MixerSampleFrame() noexcept = default;

  void add(const BasicSampleFrame& frame, int mulL, int mulR, int shift)
  {
    left += (static_cast<MixerSample>(frame.left) * mulL) >> shift;
    right += (static_cast<MixerSample>(frame.right) * mulR) >> shift;
  }

  constexpr MixerSampleFrame operator-(const MixerSampleFrame& rhs) noexcept
  {
    return MixerSampleFrame{ left - rhs.left, right - rhs.right };
  }

  constexpr MixerSampleFrame operator+(const MixerSampleFrame& rhs) noexcept
  {
    return MixerSampleFrame{ left + rhs.left, right + rhs.right };
  }

  constexpr MixerSampleFrame operator*(const MixerSampleFrame& rhs) noexcept
  {
    return MixerSampleFrame{ left * rhs.left, right * rhs.right };
  }

  constexpr MixerSampleFrame operator*(const BasicSampleFrame& rhs) noexcept
  {
    return MixerSampleFrame{ left * rhs.left, right * rhs.right };
  }

  template<typename T>
  constexpr MixerSampleFrame operator*(const T& rhs) noexcept
  {
    return MixerSampleFrame{ left * rhs, right * rhs };
  }

  template<typename T>
  constexpr MixerSampleFrame operator/(const T& rhs) noexcept
  {
    return MixerSampleFrame{ left / rhs, right / rhs };
  }

  template<typename T> MixerSampleFrame clamped() const noexcept
  {
    return MixerSampleFrame{ ppp::clip<T>( left ), ppp::clip<T>( right ) };
  }

  BasicSampleFrame toBasicSampleFrame() const noexcept
  {
    return { ppp::clip<int16_t>( left ), ppp::clip<int16_t>( right ) };
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
using AudioFrameBuffer = std::vector<BasicSampleFrame>;
using AudioFrameBufferPtr = std::shared_ptr<AudioFrameBuffer>;

/**
 * @brief Shared pointer to a vector of MixerSampleFrame's
 * @details
 * Due to the fact that a std::vector stores its data internally as an array,
 * this allows for easy iteration over the frames by grabbing a pointer to
 * the front element.
 */
using MixerFrameBuffer = std::vector<MixerSampleFrame>;
using MixerFrameBufferPtr = std::shared_ptr<MixerFrameBuffer>;

/**
 * @}
 */

#endif // AUDIOTYPES_H
