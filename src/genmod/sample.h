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
#include "stepper.h"

namespace ppp
{

class Stepper;
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
        Cubic,
        Hermite
    };
private:
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

    /**
     * @brief Get a sample
     * @param[in,out] pos Position of the requested sample
     * @return Sample value, 0 if invalid value for @a pos
     */
    inline BasicSampleFrame sampleAt(size_t pos) const noexcept;

    size_t mixNonInterpolated(Stepper& stepper, MixerFrameBuffer& buffer, size_t offset, size_t len, bool reverse, int factorLeft, int factorRight,
                              int rightShift) const;

    size_t mixLinearInterpolated(Stepper& stepper, MixerFrameBuffer& buffer, size_t offset, size_t len, bool reverse, int factorLeft,
                                 int factorRight, int rightShift) const;

    size_t mixCubicInterpolated(Stepper& stepper, MixerFrameBuffer& buffer, size_t offset, size_t len, bool reverse, int factorLeft, int factorRight,
                                int rightShift) const;

    size_t mixHermiteInterpolated(Stepper& stepper, MixerFrameBuffer& buffer, size_t offset, size_t len, bool reverse, int factorLeft, int factorRight,
                                  int rightShift) const;

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
     * @brief Get the sample's length
     * @return The sample's length
     */
    size_t length() const noexcept
    {
        return m_data.size();
    }

    size_t mix(Interpolation inter, Stepper& stepper, MixerFrameBuffer& buffer, size_t offset, size_t len, bool reverse, int factorLeft,
               int factorRight, int rightShift) const;

protected:
    typedef BasicSampleFrame::Vector::iterator Iterator;

    /**
     * @brief Set m_frequency
     * @param[in] f The new frequency value
     */
    void setFrequency(uint16_t f) noexcept
    {
        m_frequency = f;
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

inline bool mix(
    const Sample& smp,
    Sample::LoopType loopType,
    Sample::Interpolation inter,
    Stepper& stepper,
    MixerFrameBuffer& buffer,
    bool& reverse,
    size_t loopStart,
    size_t loopEnd,
    int factorLeft,
    int factorRight,
    int rightShift)
{
    // sanitize
    if( loopType == Sample::LoopType::None )
    {
        loopStart = 0;
        loopEnd = smp.length();
    }
    else
    {
        if( loopStart > smp.length() )
        {
            loopStart = smp.length();
        }
        if( loopEnd > smp.length() )
        {
            loopEnd = smp.length();
        }
    }

    size_t offset = 0;
    while( offset < buffer.size() )
    {
        size_t canRead;
        if( !reverse )
        {
            canRead = loopEnd - stepper;
        }
        else
        {
            canRead = stepper - loopStart;
        }

        const size_t canWrite = buffer.size() - offset;
        BOOST_ASSERT(canWrite > 0);
        auto mustRead = static_cast<size_t>(canWrite * stepper.floatStepSize());
        if( mustRead > canRead )
        {
            mustRead = canRead;
        }
        auto mustMix = static_cast<size_t>(mustRead / stepper.floatStepSize());
        if( mustMix <= 0 )
        {
            mustMix = 1;
        }

        offset += smp.mix(inter, stepper, buffer, offset, mustMix, reverse, factorLeft, factorRight, rightShift);

        switch( loopType )
        {
            case Sample::LoopType::None:
                if( stepper >= loopEnd )
                {
                    return false;
                }
                break;
            case Sample::LoopType::Forward:
                if( stepper >= loopEnd )
                {
                    stepper.setPosition(loopStart + (stepper - loopEnd));
                }
                break;
            case Sample::LoopType::Pingpong:
                if( reverse && stepper <= loopStart )
                {
                    stepper.setPosition(loopStart + (loopStart - stepper));
                    reverse = false;
                }
                else if( !reverse && stepper >= loopEnd )
                {
                    stepper.setPosition(loopEnd - (stepper - loopEnd));
                    reverse = true;
                }

                break;
        }
    }

    return true;
}

inline bool mix(
    const Sample& smp,
    Sample::LoopType loopType,
    Sample::Interpolation inter,
    Stepper& stepper,
    MixerFrameBuffer& buffer,
    size_t loopStart,
    size_t loopEnd,
    int factorLeft,
    int factorRight,
    int rightShift)
{
    BOOST_ASSERT(loopType != Sample::LoopType::Pingpong);

    bool tmp = false;

    return mix(
        smp,
        loopType,
        inter,
        stepper,
        buffer,
        tmp,
        loopStart,
        loopEnd,
        factorLeft,
        factorRight,
        rightShift);
}

#if 0
inline bool mix(
    const Sample& smp,
    Sample::Interpolation inter,
    Stepper& stepper,
    MixerFrameBuffer& buffer,
    bool& reverse,
    int factorLeft,
    int factorRight,
    int rightShift)
{
    size_t loopStart = smp.loopType() == Sample::LoopType::None ? 0 : smp.loopStart();
    size_t loopEnd = smp.loopType() == Sample::LoopType::None ? smp.length() : smp.loopEnd();

    return mix(
        smp,
        smp.loopType(),
        inter,
        stepper,
        buffer,
        reverse,
        loopStart,
        loopEnd,
        factorLeft,
        factorRight,
        rightShift);
}

inline bool mix(
    const Sample& smp,
    Sample::Interpolation inter,
    Stepper& stepper,
    MixerFrameBuffer& buffer,
    int factorLeft,
    int factorRight,
    int rightShift)
{
    bool tmp = false;

    size_t loopStart = smp.loopType() == Sample::LoopType::None ? 0 : smp.loopStart();
    size_t loopEnd = smp.loopType() == Sample::LoopType::None ? smp.length() : smp.loopEnd();

    return mix(
        smp,
        smp.loopType(),
        inter,
        stepper,
        buffer,
        tmp,
        loopStart,
        loopEnd,
        factorLeft,
        factorRight,
        rightShift);
}
#endif

/**
 * @}
 */

} // namespace ppp

#endif
