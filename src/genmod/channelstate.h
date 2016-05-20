#pragma once

#include <string>
#include <cstdint>

#include <stream/iserializable.h>

#include <genmod/ppplay_module_base_export.h>

namespace ppp
{
/**
 * @struct ChannelState
 * @ingroup GenMod
 * @brief Contains data to display to the user
 */
struct PPPLAY_MODULE_BASE_EXPORT ChannelState : public ISerializable {
    //! @brief Indicates that the channel is active
    bool active;
    //! @brief Is @c true to indicate that a note was triggered
    bool noteTriggered;
    //! @brief Current instrument number
    uint8_t instrument;
    //! @brief Current instrument title
    std::string instrumentName;
    /**
     * @brief Currently played note index
     * @see KeyOff NoteCut TooLow TooHigh NoNote MaxNote
     */
    uint8_t note;
    //! @brief Current effect display character
    char fx;
    //! @brief Current effect parameter
    uint8_t fxParam;
    /**
     * @brief The current effect in the format @c xxxx[xS]S, where @c x is a short description and @c S is a symbol
     * @see ppp::fxdesc
     */
    std::string fxDesc;
    //! @brief Panning position (-100..100), or @c Surround
    int8_t panning;
    //! @brief Volume, 0..100
    uint8_t volume;
    //! @brief Current pattern cell string
    std::string cell;

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

    ChannelState() : active( false ), noteTriggered( false ), instrument( 0 )
        , instrumentName(), note( NoNote ), fx( 0 ), fxParam( 0 ), fxDesc()
        , panning( 0 ), volume( 0 ), cell() {
    }

    AbstractArchive& serialize( AbstractArchive* archive ) override;
};
}
