#ifndef PPPLAY_OPL_FILTER_H
#define PPPLAY_OPL_FILTER_H

#include <cmath>
#include <boost/assert.hpp>

namespace ppp
{
class OplFilter
{
private:
    float m_highPassAlpha = 0;
    float m_lowPassAlpha = 0;
    float m_lastOutputHi = 0;
    float m_lastOutputLo = 0;
    float m_lastInputHi = 0;
    
    static constexpr float HighPassCutoff = 10;
    static constexpr float LowPassCutoff = 18000;
public:
    explicit OplFilter(uint16_t samplerate) {
        const float samplingTimeConstant = samplerate / (2*M_PI);
        // 10Hz cutoff
        m_highPassAlpha = samplingTimeConstant / (HighPassCutoff+samplingTimeConstant);
        BOOST_ASSERT_MSG(0<=m_highPassAlpha && m_highPassAlpha<=1, "Error: High Pass constant invalid");
        // 8kHz cutoff
        m_lowPassAlpha = samplingTimeConstant / (LowPassCutoff+samplingTimeConstant);
        BOOST_ASSERT_MSG(0<=m_lowPassAlpha && m_lowPassAlpha<=1, "Error: Low Pass constant invalid");
    }
    
    int16_t filter(int16_t smp) noexcept {
        m_lastOutputHi = m_highPassAlpha * (m_lastOutputHi + smp-m_lastInputHi);
        m_lastOutputLo += m_lowPassAlpha * (m_lastOutputHi - m_lastOutputLo);
        m_lastInputHi = smp;
        return m_lastOutputLo;
    }
};
}

#endif