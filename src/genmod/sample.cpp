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

#include "sample.h"

namespace ppp
{
/**
 * @ingroup GenMod
 * @{
 */
light4cxx::Logger* Sample::logger()
{
    return light4cxx::Logger::get("sample");
}

size_t Sample::mixNonInterpolated(ppp::Stepper& stepper,
                                  MixerFrameBuffer& buffer,
                                  size_t offset,
                                  size_t len,
                                  bool reverse,
                                  int factorLeft,
                                  int factorRight,
                                  int rightShift) const
{
    BOOST_ASSERT(rightShift >= 0);

    size_t mixed = 0;
    for( ; offset < buffer.size() && mixed < len; ++offset, ++mixed )
    {
        if( !reverse && stepper >= 0 && static_cast<size_t>(stepper) >= m_data.size() )
        {
            return mixed;
        }
        else if( reverse && stepper < 0 )
        {
            return mixed;
        }

        const auto& sampleVal = sampleAt(stepper);
        buffer[offset].add(sampleVal, factorLeft, factorRight, rightShift);
        if( !reverse )
        {
            stepper.next();
        }
        else
        {
            stepper.prev();
        }
    }
    return mixed;
}

size_t Sample::mixLinearInterpolated(ppp::Stepper& stepper,
                                     MixerFrameBuffer& buffer,
                                     size_t offset,
                                     size_t len,
                                     bool reverse,
                                     int factorLeft,
                                     int factorRight,
                                     int rightShift) const
{
    BOOST_ASSERT(rightShift >= 0);

    size_t mixed = 0;
    for( ; offset < buffer.size() && mixed < len; ++offset, ++mixed )
    {
        if( !reverse && stepper >= 0 && static_cast<size_t>(stepper) >= m_data.size() )
        {
            return mixed;
        }
        else if( reverse && stepper < 0 )
        {
            return mixed;
        }

        auto sampleVal = stepper.biased(sampleAt(stepper), sampleAt(1u + stepper));
        buffer[offset].add(sampleVal, factorLeft, factorRight, rightShift);
        if( !reverse )
        {
            stepper.next();
        }
        else
        {
            stepper.prev();
        }
    }
    return mixed;
}

namespace
{
constexpr inline float interpolateCubic(float x0, float x1, float x2, float x3, float t)
{
    const auto a0 = x3 - x2 - x0 + x1;
    const auto a1 = x0 - x1 - a0;
    const auto a2 = x2 - x0;
    const auto a3 = x1;
    return a0 * t * t * t + a1 * t * t + a2 * t + a3;
}

constexpr inline float interpolateHermite4pt3oX(float x0, float x1, float x2, float x3, float t)
{
    float c0 = x1;
    float c1 = (x2 - x0) / 2;
    float c2 = x0 - 2.5f * x1 + 2 * x2 - x3 / 2;
    float c3 = (x3 - x0) / 2 + 1.5f * (x1 - x2);
    return ((c3 * t + c2) * t + c1) * t + c0;
}
}

size_t Sample::mixCubicInterpolated(ppp::Stepper& stepper,
                                    MixerFrameBuffer& buffer,
                                    size_t offset,
                                    size_t len,
                                    bool reverse,
                                    int factorLeft,
                                    int factorRight,
                                    int rightShift) const
{
    BOOST_ASSERT(rightShift >= 0);

    size_t mixed = 0;
    for( ; offset < buffer.size() && mixed < len; ++offset, ++mixed )
    {
        if( !reverse && stepper >= 0 && static_cast<size_t>(stepper) >= m_data.size() )
        {
            return mixed;
        }
        else if( reverse && stepper < 0 )
        {
            return mixed;
        }

        BasicSampleFrame samples[4];
        for( int i = 0u; i < 4; i++ )
        {
            samples[i] = sampleAt(i + stepper - 1u);
        }

        buffer[offset].left +=
            factorLeft * interpolateCubic(samples[0].left, samples[1].left, samples[2].left, samples[3].left, stepper.floatStepSize()) / (1 << rightShift);
        buffer[offset].right +=
            factorRight * interpolateCubic(samples[0].right, samples[1].right, samples[2].right, samples[3].right, stepper.floatStepSize()) / (1 << rightShift);
        if( !reverse )
        {
            stepper.next();
        }
        else
        {
            stepper.prev();
        }
    }
    return mixed;
}

size_t Sample::mixHermiteInterpolated(ppp::Stepper& stepper,
                                      MixerFrameBuffer& buffer,
                                      size_t offset,
                                      size_t len,
                                      bool reverse,
                                      int factorLeft,
                                      int factorRight,
                                      int rightShift) const
{
    BOOST_ASSERT(rightShift >= 0);

    size_t mixed = 0;
    for( ; offset < buffer.size() && mixed < len; ++offset, ++mixed )
    {
        if( !reverse && stepper >= 0 && static_cast<size_t>(stepper) >= m_data.size() )
        {
            return mixed;
        }
        else if( reverse && stepper < 0 )
        {
            return mixed;
        }

        BasicSampleFrame samples[4];
        for( int i = 0u; i < 4; i++ )
        {
            samples[i] = sampleAt(i + stepper - 1u);
        }

        buffer[offset].left +=
            factorLeft * interpolateHermite4pt3oX(samples[0].left, samples[1].left, samples[2].left, samples[3].left, stepper.floatStepSize()) /
            (1 << rightShift);
        buffer[offset].right +=
            factorRight * interpolateHermite4pt3oX(samples[0].right, samples[1].right, samples[2].right, samples[3].right, stepper.floatStepSize()) /
            (1 << rightShift);
        if( !reverse )
        {
            stepper.next();
        }
        else
        {
            stepper.prev();
        }
    }
    return mixed;
}

size_t Sample::mix(ppp::Sample::Interpolation inter,
                   ppp::Stepper& stepper,
                   MixerFrameBuffer& buffer,
                   size_t offset,
                   size_t len,
                   bool reverse,
                   int factorLeft,
                   int factorRight,
                   int rightShift) const
{
    switch( inter )
    {
        case Interpolation::None:
            return mixNonInterpolated(stepper, buffer, offset, len, reverse, factorLeft, factorRight, rightShift);
        case Interpolation::Linear:
            return mixLinearInterpolated(stepper, buffer, offset, len, reverse, factorLeft, factorRight, rightShift);
        case Interpolation::Cubic:
            return mixCubicInterpolated(stepper, buffer, offset, len, reverse, factorLeft, factorRight, rightShift);
        case Interpolation::Hermite:
            return mixHermiteInterpolated(stepper, buffer, offset, len, reverse, factorLeft, factorRight, rightShift);
        default:
            return 0;
    }
}

inline BasicSampleFrame Sample::sampleAt(size_t pos) const noexcept
{
    if( pos >= m_data.size() )
    {
        return {0, 0};
    }
    return m_data[pos];
}

/**
 * @}
 */
bool mix(const Sample& smp, Sample::LoopType loopType, Sample::Interpolation inter, Stepper& stepper, MixerFrameBuffer& buffer, bool& reverse, size_t loopStart,
         size_t loopEnd, int factorLeft, int factorRight, int rightShift)
{
    BOOST_ASSERT(stepper >= 0);

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
        long canRead;
        if( stepper < loopStart )
        {
            // special case: loop not yet entered
            BOOST_ASSERT(!reverse);
            canRead = loopEnd - stepper;
        }
        else
        {
            if( !reverse )
            {
                canRead = loopEnd - stepper;
            }
            else
            {
                canRead = stepper - loopStart;
            }
        }

        if( canRead < 0 )
        {
            canRead = 0;
        }

        const size_t canWrite = buffer.size() - offset;
        BOOST_ASSERT(canWrite > 0);
        auto mustRead = static_cast<long>(canWrite * stepper.floatStepSize());
        BOOST_ASSERT(mustRead >= 0);
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
                if( stepper >= 0 && static_cast<size_t>(stepper) >= loopEnd )
                {
                    return false;
                }
                break;
            case Sample::LoopType::Forward:
                if( stepper >= 0 && static_cast<size_t>(stepper) >= loopEnd )
                {
                    stepper = loopStart + (stepper - loopEnd);
                    BOOST_ASSERT(stepper >= 0 && static_cast<size_t>(stepper) >= loopStart && static_cast<size_t>(stepper) < loopEnd);
                }
                break;
            case Sample::LoopType::Pingpong:
                if( reverse && (stepper < 0 || static_cast<size_t>(stepper) < loopStart) )
                {
                    stepper = loopStart + (loopStart - stepper);
                    reverse = false;
                    BOOST_ASSERT(stepper >= 0 && static_cast<size_t>(stepper) >= loopStart && static_cast<size_t>(stepper) < loopEnd);
                }
                else if( !reverse && (stepper > 0 && static_cast<size_t>(stepper) >= loopEnd) )
                {
                    stepper = loopEnd - (stepper - loopEnd) - 1;
                    reverse = true;
                    BOOST_ASSERT(stepper >= 0 && static_cast<size_t>(stepper) >= loopStart && static_cast<size_t>(stepper) < loopEnd);
                }

                break;
        }
    }

    return true;
}
}