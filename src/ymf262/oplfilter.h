#ifndef PPPLAY_OPL_FILTER_H
#define PPPLAY_OPL_FILTER_H

#include <cmath>
#include <array>
#include <algorithm>
#include <numeric>

namespace opl
{

namespace detail
{
template<typename T, int N>
inline std::array<T,N> createZeroArray()
{
    std::array<T,N> result;
    result.fill(0);
    return result;
}
}

template<int InputRate, int Cutoff, int NTaps>
class LowPassFilter
{
    static_assert( NTaps>0 && NTaps<=100, "Taps out of range" );
    static_assert( InputRate>0, "Invalid input sample rate" );
    static_assert( Cutoff>0 && Cutoff < InputRate/2, "Invalid cutoff frequency" );
private:
    std::array<float_t,NTaps> m_taps;
    std::array<float_t,NTaps> m_sr;

public:
    LowPassFilter() : m_taps(detail::createZeroArray<float_t,NTaps>()), m_sr(detail::createZeroArray<float_t,NTaps>()) {
        const float_t lambda = 2 * M_PI * Cutoff / InputRate;

        for(int n = 0; n < NTaps; n++){
            float_t mm = n - (NTaps - 1.0) / 2.0;
            if( mm == 0 )
                m_taps[n] = lambda / M_PI;
            else
                m_taps[n] = std::sin( mm * lambda ) / (mm * M_PI);
        }
    }

    float_t filter(float_t sample)
    {
        // push the sample to the front
        for(int i = NTaps-1; i>=1; --i) {
            m_sr[i] = m_sr[i-1];
        }
        m_sr[0] = sample;

        return std::inner_product(m_sr.begin(), m_sr.end(), m_taps.begin(), 0.0);
    }
};

template<int InputRate, int Cutoff, int NTaps>
class HighPassFilter
{
    static_assert( NTaps>0 && NTaps<=100, "Taps out of range" );
    static_assert( InputRate>0, "Invalid input sample rate" );
    static_assert( Cutoff>0 && Cutoff < InputRate/2, "Invalid cutoff frequency" );
private:
    std::array<float_t,NTaps> m_taps;
    std::array<float_t,NTaps> m_sr;

    static const constexpr float Pi = 3.14159265358979323846f;

public:
    HighPassFilter() : m_taps(detail::createZeroArray<float_t,NTaps>()), m_sr(detail::createZeroArray<float_t,NTaps>()) {
        const float_t lambda = 2 * Pi * Cutoff / InputRate;

        for(int n = 0; n < NTaps; n++) {
            float_t mm = n - (NTaps - 1.0) / 2.0;
            if( mm == 0.0 )
                m_taps[n] = 1 - lambda / Pi;
            else
                m_taps[n] = -std::sin( mm * lambda ) / (mm * Pi);
        }
    }

    float_t filter(float_t sample) {
        // push the sample to the front
        for(int i = NTaps-1; i>=1; --i) {
            m_sr[i] = m_sr[i-1];
        }
        m_sr[0] = sample;

        return std::inner_product(m_sr.begin(), m_sr.end(), m_taps.begin(), 0.0f);
    }
};

template<int InputRate, int CutoffLow, int CutoffHigh, int NTaps>
class BandPassFilter
{
    static_assert( NTaps>0 && NTaps<=100, "Taps out of range" );
    static_assert( InputRate>0, "Invalid input sample rate" );
    static_assert( CutoffLow>0 && CutoffLow < InputRate/2, "Invalid lowpass cutoff frequency" );
    static_assert( CutoffHigh>0 && CutoffHigh < InputRate/2, "Invalid highpass cutoff frequency" );
private:
    std::array<float_t,NTaps> m_taps;
    std::array<float_t,NTaps> m_sr;

public:
    BandPassFilter() : m_taps(detail::createZeroArray<float_t,NTaps>()), m_sr(detail::createZeroArray<float_t,NTaps>()) {
        const float_t lambda = 2 * M_PI * CutoffLow / InputRate;
        const float_t phi = 2 * M_PI * CutoffHigh / InputRate;

        for(int n = 0; n < NTaps; n++){
            float_t mm = n - (NTaps - 1.0) / 2.0;
            if( mm == 0.0 )
                m_taps[n] = (phi - lambda) / M_PI;
            else
                m_taps[n] = (std::sin( mm * phi ) - std::sin( mm * lambda )) / (mm * M_PI);
        }
    }

    float_t filter(float_t sample)
    {
        // push the sample to the front
        for(int i = NTaps-1; i>=1; --i) {
            m_sr[i] = m_sr[i-1];
        }
        m_sr[0] = sample;

        return std::inner_product(m_sr.begin(), m_sr.end(), m_taps.begin(), 0.0);
    }
};

template<int N, class Filter>
class MultiplexFilter
{
private:
    std::array<Filter,N> m_filters;
public:
    MultiplexFilter() : m_filters(){}

    template<class T>
    void filter(std::array<T,N>& samples) {
        for(int i=0; i<N; ++i)
            samples[i] = static_cast<T>(m_filters[i].filter(samples[i]));
    }
};

}

#endif
