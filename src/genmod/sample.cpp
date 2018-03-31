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
        if(!reverse && stepper >= m_data.size())
            return mixed;
        else if(reverse && stepper < 0)
            return mixed;

        const auto& sampleVal = sampleAt(stepper);
        buffer[offset].add(sampleVal, factorLeft, factorRight, rightShift);
        if(!reverse)
            stepper.next();
        else
            stepper.prev();
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
        if(!reverse && stepper >= m_data.size())
            return mixed;
        else if(reverse && stepper < 0)
            return mixed;

        auto sampleVal = stepper.biased(sampleAt(stepper), sampleAt(1u + stepper));
        buffer[offset].add(sampleVal, factorLeft, factorRight, rightShift);
        if(!reverse)
            stepper.next();
        else
            stepper.prev();
    }
    return mixed;
}

namespace
{
constexpr inline int interpolateCubicLevel0(int p0, int p1, int p2, int p3, int bias)
{
    return (bias * (3 * (p1 - p2) + p3 - p0)) / 256;
}

constexpr inline int interpolateCubicLevel1(int p0, int p1, int p2, int p3, int bias)
{
    return (bias * ((p0 * 2) - 5 * p1 + (p2 * 4) - p3 + interpolateCubicLevel0(p0, p1, p2, p3, bias))) / 256;
}

constexpr inline MixerSample interpolateCubic(int p0, int p1, int p2, int p3, int bias)
{
    return p1 + ((bias * (p2 - p0 + interpolateCubicLevel1(p0, p1, p2, p3, bias))) / 512);
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
        if(!reverse && stepper >= m_data.size())
            return mixed;
        else if(reverse && stepper < 0)
            return mixed;

        BasicSampleFrame samples[4];
        for( int i = 0u; i < 4; i++ )
        {
            samples[i] = sampleAt(i + stepper - 1u);
        }

        buffer[offset].left += (factorLeft * interpolateCubic(samples[0].left, samples[1].left, samples[2].left, samples[3].left, stepper.stepSize())) >> rightShift;
        buffer[offset].right += (factorRight * interpolateCubic(samples[0].right, samples[1].right, samples[2].right, samples[3].right, stepper.stepSize())) >> rightShift;
        if(!reverse)
            stepper.next();
        else
            stepper.prev();
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
}