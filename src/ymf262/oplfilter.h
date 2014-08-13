#ifndef PPPLAY_OPL_FILTER_H
#define PPPLAY_OPL_FILTER_H

#include <cmath>
#include <boost/assert.hpp>

namespace opl
{
template<typename T, int N, int SampleFrq, int LowPass=22050, int HiPass=150>
class OplFilter
{
private:
    static constexpr float SamplingTimeConstant = SampleFrq / (2*M_PI);
    // 10Hz cutoff
    static constexpr float HighPassAlpha = SamplingTimeConstant / (HiPass+SamplingTimeConstant);
    static_assert(0<=HighPassAlpha && HighPassAlpha<=1, "Error: High Pass constant invalid");
    // 8kHz cutoff
    static constexpr float LowPassAlpha = SamplingTimeConstant / (LowPass+SamplingTimeConstant);
    static_assert(0<=LowPassAlpha && LowPassAlpha<=1, "Error: Low Pass constant invalid");
    std::array<float,N> m_lastOutputHi;
    std::array<float,N> m_lastOutputLo;
    std::array<float,N> m_lastInputHi;
public:
    OplFilter() {
        m_lastOutputHi.fill(0);
        m_lastOutputLo.fill(0);
        m_lastInputHi.fill(0);
    }

    void filter(std::array<T,N>& smp) noexcept {
        for(int i=0; i<N; ++i) {
            m_lastOutputHi[i] = HighPassAlpha * (m_lastOutputHi[i] + smp[i]-m_lastInputHi[i]);
            m_lastOutputLo[i] += LowPassAlpha * (m_lastOutputHi[i] - m_lastOutputLo[i]);
            m_lastInputHi[i] = smp[i];
            smp[i] = m_lastOutputLo[i];
        }
    }
};
}

#endif
