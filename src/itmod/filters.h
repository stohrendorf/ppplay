#pragma once

#include "genmod/sample.h"

#include <array>
#include <cmath>

namespace ppp
{
// See https://wiki.multimedia.cx/index.php/Impulse_Tracker#Resonant_filters
class Filter
{
    std::array<float, 2> m_k{0, 0};

    float m_r = 0;
    float m_p = 0;

public:
    void update(uint32_t outputFrequency, float cutoff, float resonance)
    {
        BOOST_ASSERT(resonance >= 0 && resonance <= 127);
        BOOST_ASSERT(cutoff >= 0 && cutoff <= 255);

        static const auto FreqMultiplier = 2.0f * M_PI * 110.0f * std::pow(2.0f, 0.25f);

        cutoff /= 256;
        m_r = static_cast<float>(outputFrequency / FreqMultiplier * std::pow(2.0f, cutoff / 24.0f));
        m_p = std::pow(10.0f, (-resonance * 24.0f) / (128.0f * 20.0f));
    }

    void reset()
    {
        m_k.fill(0);
    }

    float filter(float value)
    {
        return value; // TODO: enable when macro parsing is implemented

        const auto e = m_r * m_r;

        const auto d = (2 * m_p) * (m_r + 1) - 1;
        const auto a = 1 / (1 + d + e);
        const auto b = (d + 2 * e) * a;
        const auto c = -e * a;
        // K[n] = a * value + b * K[n-1] + c * K[n-2];
        const auto result = a * value + b * m_k[0] + c * m_k[1];
        m_k[1] = std::exchange(m_k[0], result);
        return result;
    }
};
}
