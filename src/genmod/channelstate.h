#pragma once

#include <string>
#include <cstdint>

#include <stream/iserializable.h>

namespace ppp
{
/**
 * @struct ChannelState
 * @ingroup GenMod
 * @brief Contains data to display to the user
 */
struct ChannelState
  : public ISerializable
{
  //! @brief Indicates that the channel is active
  bool active = false;
  //! @brief Is @c true to indicate that a note was triggered
  bool noteTriggered = false;
  //! @brief Current instrument number
  uint8_t instrument = 0;
  //! @brief Current instrument title
  std::string instrumentName{};
  /**
   * @brief Currently played note index
   * @see KeyOff NoteCut TooLow TooHigh NoNote MaxNote
   */
  uint8_t note = NoNote;
  //! @brief Current effect display character
  char fx = 0;
  /**
   * @brief The current effect in the format @c xxxx[xS]S, where @c x is a short description and @c S is a symbol
   * @see ppp::fxdesc
   */
  std::string fxDesc{};
  //! @brief Panning position (-100..100), or @c Surround
  int8_t panning = 0;
  //! @brief Volume, 0..100
  uint8_t volume = 0;
  //! @brief Current pattern cell string
  std::string cell{};

  /**
   * @brief Special note values
   * @{
   */
  //! @brief Key Off note
  static constexpr uint8_t KeyOff = 255;
  //! @brief Note Cut note
  static constexpr uint8_t NoteCut = 254;
  //! @brief Note is lower than C-0
  static constexpr uint8_t TooLow = 253;
  //! @brief Note is higher than B-9
  static constexpr uint8_t TooHigh = 252;
  static constexpr uint8_t NoNote = 251;
  //! @brief Highest possible note (B-9)
  static constexpr uint8_t MaxNote = 9 * 12 + 11;
  /**
   * @}
   */

  //! @brief Panning is virtual surround
  static constexpr int8_t Surround = -128;

  ChannelState() = default;

  AbstractArchive& serialize(AbstractArchive* archive) override;
};
}
