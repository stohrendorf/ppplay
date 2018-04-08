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

AudioFrameBuffer Sample::readNonInterpolated(ppp::Stepper& stepper,
                                             size_t requestedLen,
                                             size_t limitMin,
                                             size_t limitMax,
                                             bool reverse) const
{
    AudioFrameBuffer result;
    while( requestedLen-- )
    {
        if( !reverse && stepper >= 0 && static_cast<size_t>(stepper) >= limitMax )
        {
            return result;
        }
        else if( reverse && (stepper < 0 || static_cast<size_t>(stepper) < limitMin) )
        {
            return result;
        }

        result.emplace_back(sampleAt(stepper));
        if( !reverse )
        {
            stepper.next();
        }
        else
        {
            stepper.prev();
        }
    }
    return result;
}

AudioFrameBuffer Sample::readLinearInterpolated(ppp::Stepper& stepper,
                                                size_t requestedLen,
                                                size_t limitMin,
                                                size_t limitMax,
                                                bool reverse) const
{
    AudioFrameBuffer result;
    while( requestedLen-- )
    {
        if( !reverse && stepper >= 0 && static_cast<size_t>(stepper) >= limitMax )
        {
            return result;
        }
        else if( reverse && (stepper < 0 || static_cast<size_t>(stepper) < limitMin) )
        {
            return result;
        }

        result.emplace_back(stepper.biased(sampleAt(stepper), sampleAt(1u + stepper)));
        if( !reverse )
        {
            stepper.next();
        }
        else
        {
            stepper.prev();
        }
    }
    return result;
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

AudioFrameBuffer Sample::readCubicInterpolated(ppp::Stepper& stepper,
                                               size_t requestedLen,
                                               size_t limitMin,
                                               size_t limitMax,
                                               bool reverse) const
{
    AudioFrameBuffer result;
    while( requestedLen-- )
    {
        if( !reverse && stepper >= 0 && static_cast<size_t>(stepper) >= limitMax )
        {
            return result;
        }
        else if( reverse && (stepper < 0 || static_cast<size_t>(stepper) < limitMin) )
        {
            return result;
        }

        BasicSampleFrame samples[4];
        for( int i = 0u; i < 4; i++ )
        {
            samples[i] = sampleAt(i + stepper - 1u);
        }

        const auto l = ppp::clip<int>(interpolateCubic(samples[0].left, samples[1].left, samples[2].left, samples[3].left, stepper.floatFraction()), -32768,
                                      32767);
        const auto r = ppp::clip<int>(interpolateCubic(samples[0].right, samples[1].right, samples[2].right, samples[3].right, stepper.floatFraction()), -32768,
                                      32767);
        result.emplace_back(l, r);
        if( !reverse )
        {
            stepper.next();
        }
        else
        {
            stepper.prev();
        }
    }
    return result;
}

AudioFrameBuffer Sample::readHermiteInterpolated(ppp::Stepper& stepper,
                                                 size_t requestedLen,
                                                 size_t limitMin,
                                                 size_t limitMax,
                                                 bool reverse) const
{
    AudioFrameBuffer result;
    while( requestedLen-- )
    {
        if( !reverse && stepper >= 0 && static_cast<size_t>(stepper) >= limitMax )
        {
            return result;
        }
        else if( reverse && (stepper < 0 || static_cast<size_t>(stepper) < limitMin) )
        {
            return result;
        }

        BasicSampleFrame samples[4];
        for( int i = 0u; i < 4; i++ )
        {
            samples[i] = sampleAt(i + stepper - 1u);
        }

        const auto l = ppp::clip<int>(interpolateHermite4pt3oX(samples[0].left, samples[1].left, samples[2].left, samples[3].left, stepper.floatFraction()),
                                      -32768, 32767);
        const auto r = ppp::clip<int>(interpolateHermite4pt3oX(samples[0].right, samples[1].right, samples[2].right, samples[3].right, stepper.floatFraction()),
                                      -32768, 32767);
        result.emplace_back(l, r);
        if( !reverse )
        {
            stepper.next();
        }
        else
        {
            stepper.prev();
        }
    }
    return result;
}

AudioFrameBuffer Sample::read(ppp::Sample::Interpolation inter,
                              ppp::Stepper& stepper,
                              size_t requestedLen,
                              size_t limitMin,
                              size_t limitMax,
                              bool reverse) const
{
    switch( inter )
    {
        case Interpolation::None:
            return readNonInterpolated(stepper, requestedLen, limitMin, limitMax, reverse);
        case Interpolation::Linear:
            return readLinearInterpolated(stepper, requestedLen, limitMin, limitMax, reverse);
        case Interpolation::Cubic:
            return readCubicInterpolated(stepper, requestedLen, limitMin, limitMax, reverse);
        case Interpolation::Hermite:
            return readHermiteInterpolated(stepper, requestedLen, limitMin, limitMax, reverse);
        default:
            return {};
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
AudioFrameBuffer read(const Sample& smp,
                      Sample::LoopType loopType,
                      Sample::Interpolation inter,
                      Stepper& stepper,
                      size_t requestedLen,
                      bool& reverse,
                      size_t loopStart,
                      size_t loopEnd,
                      bool preprocess)
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

    AudioFrameBuffer result;
    result.reserve(requestedLen);

    while( result.size() < requestedLen )
    {
        if( !preprocess )
        {
            auto tmp = smp.read(inter, stepper, requestedLen - result.size(), loopStart, loopEnd, reverse);
            BOOST_ASSERT(tmp.size() <= requestedLen);
            std::copy(tmp.begin(), tmp.end(), std::back_inserter(result));
        }
        else
        {
            while( result.size() < requestedLen )
            {
                if( !reverse && stepper >= 0 && static_cast<size_t>(stepper) >= loopEnd )
                {
                    break;
                }
                else if( reverse && (stepper < 0 || static_cast<size_t>(stepper) < loopStart) )
                {
                    break;
                }

                result.emplace_back();

                if( !reverse )
                {
                    stepper.next();
                }
                else
                {
                    stepper.prev();
                }
            }
        }

        switch( loopType )
        {
            case Sample::LoopType::None:
                if( stepper >= 0 && static_cast<size_t>(stepper) >= loopEnd )
                {
                    BOOST_ASSERT(result.size() <= requestedLen);
                    return result;
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

    BOOST_ASSERT(result.size() <= requestedLen);

    return result;
}
}