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

#ifndef PPPLAY_SAMPLE_H
#define PPPLAY_SAMPLE_H

#include <genmod/ppplay_module_base_export.h>

#include <output/audiotypes.h>
#include <light4cxx/logger.h>
#include "breseninter.h"

namespace ppp
{

class BresenInterpolation;
/**
 * @ingroup GenMod
 * @{
 */

/**
 * @class GenSample
 * @brief An abstract sample class
 */
class PPPLAY_MODULE_BASE_EXPORT Sample
{
public:
    DISABLE_COPY(Sample)

    //! @brief Loop type definitions
    enum class LoopType
    {
        None, //!< @brief not looped
        Forward, //!< @brief Forward looped
        Pingpong //!< @brief Ping pong looped
    };

    enum class Interpolation
    {
        None,
        Linear,
        Cubic
    };
private:
    //! @brief Loop start sample
    uint_fast32_t m_loopStart = 0;
    //! @brief Loop end sample (points to 1 frame @e after the loop end)
    uint_fast32_t m_loopEnd = 0;
    //! @brief Default volume of the sample
    uint8_t m_volume = 0;
    //! @brief Base frequency of the sample
    uint16_t m_frequency = 0;
    //! @brief Sample data
    BasicSampleFrame::Vector m_data{};
    //! @brief Sample filename
    std::string m_filename{};
    //! @brief Sample title
    std::string m_title{};
    //! @brief Loop type
    LoopType m_looptype = LoopType::None;

    /**
     * @brief Wraps a virtual position of ping-pong looped samples to the real position
     * @param[in] pos Virtual position
     * @return Real position
     * @note Time-critical
     */
    inline uint_fast32_t makeRealPos(uint_fast32_t pos) const noexcept;

    /**
     * @brief Adjust the playback position so it doesn't fall out of the sample data. Returns EndOfSample if it does
     * @param[in,out] pos Reference to the variable that should be adjusted
     * @return Adjusted position
     * @note Time-critical
     */
    inline uint_fast32_t adjustPosition(uint_fast32_t pos) const noexcept;

    /**
     * @brief Get a sample
     * @param[in,out] pos Position of the requested sample
     * @return Sample value, 0 if invalid value for @a pos
     */
    inline BasicSampleFrame sampleAt(uint_fast32_t pos) const noexcept;

    bool mixNonInterpolated(BresenInterpolation& bresen, MixerFrameBuffer& buffer, int factorLeft, int factorRight, int rightShift) const;

    bool mixLinearInterpolated(BresenInterpolation& bresen, MixerFrameBuffer& buffer, int factorLeft, int factorRight, int rightShift) const;

    bool mixCubicInterpolated(BresenInterpolation& bresen, MixerFrameBuffer& buffer, int factorLeft, int factorRight, int rightShift) const;

public:
    /**
     * @brief Constructor
     */
    Sample() = default;

    /**
     * @brief Destructor
     */
    virtual ~Sample() noexcept = default;

    /**
     * @brief Get the sample's Base Frequency
     * @return Base frequency
     */
    uint16_t frequency() const noexcept
    {
        return m_frequency;
    }

    /**
     * @brief Get the sample's default volume
     * @return Default volume
     */
    uint8_t volume() const noexcept
    {
        return m_volume;
    }

    /**
     * @brief Get the sample's name
     * @return Sample's name
     */
    std::string title() const
    {
        return m_title;
    }

    /**
     * @brief Is the sample looped?
     * @return @c true if the sample is looped
     */
    bool isLooped() const noexcept
    {
        return m_looptype != LoopType::None;
    }

    /**
     * @brief Get the sample's length
     * @return The sample's length
     */
    size_t length() const noexcept
    {
        return m_data.size();
    }

    /**
     * @brief Get the loop type
     * @return The loop type
     */
    LoopType loopType() const noexcept
    {
        return m_looptype;
    }

    bool mix(Interpolation inter, BresenInterpolation& bresen, MixerFrameBuffer& buffer, int factorLeft, int factorRight, int rightShift) const;

protected:
    typedef BasicSampleFrame::Vector::iterator Iterator;
    typedef BasicSampleFrame::Vector::const_iterator ConstIterator;

    /**
     * @brief Set m_frequency
     * @param[in] f The new frequency value
     */
    void setFrequency(uint16_t f) noexcept
    {
        m_frequency = f;
    }

    /**
     * @brief Set m_looptype
     * @param[in] l The new loop type value
     */
    void setLoopType(LoopType l) noexcept
    {
        m_looptype = l;
    }

    /**
     * @brief Get data start iterator
     * @return Data start iterator
     */
    inline Iterator beginIterator() noexcept
    {
        return m_data.begin();
    }

    /**
     * @brief Get data end iterator
     * @return Data end iterator
     */
    inline Iterator endIterator() noexcept
    {
        return m_data.end();
    }

    /**
     * @brief Set the sample's name
     * @param[in] t The new name
     */
    void setTitle(const std::string& t)
    {
        m_title = t;
    }

    /**
     * @brief Set the sample's filename
     * @param[in] f The new filename
     */
    void setFilename(const std::string& f)
    {
        m_filename = f;
    }

    /**
     * @brief Set the sample's loop start
     * @param[in] s The new loop start
     */
    void setLoopStart(uint_fast32_t s) noexcept
    {
        m_loopStart = s;
    }

    /**
     * @brief Set the sample's loop end
     * @param[in] e The new loop end
     */
    void setLoopEnd(uint_fast32_t e) noexcept
    {
        m_loopEnd = e;
    }

    /**
     * @brief Set the sample's default volume
     * @param[in] v The new volume
     */
    void setVolume(uint8_t v) noexcept
    {
        m_volume = v;
    }

    /**
     * @brief Resize the data
     * @param[in] size New size
     */
    inline void resizeData(size_t size)
    {
        m_data.resize(size);
    }

    /**
     * @brief Get the logger
     * @return Logger with name "sample"
     */
    static light4cxx::Logger* logger();
};

inline uint_fast32_t Sample::adjustPosition(uint_fast32_t pos) const noexcept
{
    if( pos == BresenInterpolation::InvalidPosition )
    {
        return BresenInterpolation::InvalidPosition;
    }
    if( m_looptype == LoopType::None )
    {
        if( pos >= length() )
        {
            return BresenInterpolation::InvalidPosition;
        }
        return pos;
    }
    auto vLoopLen = m_loopEnd - m_loopStart;
    auto vLoopEnd = m_loopEnd;
    if( m_looptype == LoopType::Pingpong )
    {
        vLoopLen *= 2;
        vLoopEnd = m_loopStart + vLoopLen;
    }
    while( pos >= vLoopEnd )
    {
        pos -= vLoopLen;
    }
    return pos;
}

/**
 * @}
 */

} // namespace ppp

#endif
