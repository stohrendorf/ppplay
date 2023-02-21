#pragma once

#include <cmath>
#include <array>
#include <numeric>

namespace opl
{
namespace detail
{
template<typename T, int N>
inline std::array<T, N> createZeroArray()
{
  std::array<T, N> result;
  result.fill( 0 );
  return result;
}

const constexpr float Pi = 3.14159265358979323846f;
}

template<int InputRate, int Cutoff, int NTaps>
class LowPassFilter
{
  static_assert( NTaps > 0 && NTaps <= 100, "Taps out of range" );
  static_assert( InputRate > 0, "Invalid input sample rate" );
  static_assert( Cutoff > 0 && Cutoff < InputRate / 2, "Invalid cutoff frequency" );
private:
  std::array<float_t, NTaps> m_taps = detail::createZeroArray<float_t, NTaps>();
  std::array<float_t, NTaps> m_sr = detail::createZeroArray<float_t, NTaps>();

public:
  LowPassFilter()
  {
    static constexpr float_t lambda = 2 * detail::Pi * Cutoff / InputRate;

    for( int n = 0; n < NTaps; n++ )
    {
      float_t mm = n - (NTaps - 1) / float_t( 2 );
      if( mm == 0 )
        m_taps[n] = lambda / detail::Pi;
      else
        m_taps[n] = std::sin( mm * lambda ) / (mm * detail::Pi);
    }
  }

  float_t filter(float_t sample)
  {
    // push the sample to the front
    for( int i = NTaps - 1; i >= 1; --i )
    {
      m_sr[i] = m_sr[i - 1];
    }
    m_sr[0] = sample;

    return std::inner_product( m_sr.begin(), m_sr.end(), m_taps.begin(), 0.0 );
  }
};

template<int InputRate, int Cutoff, int NTaps>
class HighPassFilter
{
  static_assert( NTaps > 0 && NTaps <= 100, "Taps out of range" );
  static_assert( InputRate > 0, "Invalid input sample rate" );
  static_assert( Cutoff > 0 && Cutoff < InputRate / 2, "Invalid cutoff frequency" );
private:
  std::array<float_t, NTaps> m_taps = detail::createZeroArray<float_t, NTaps>();
  std::array<float_t, NTaps> m_sr = detail::createZeroArray<float_t, NTaps>();

public:
  HighPassFilter()
  {
    static constexpr float_t lambda = 2 * detail::Pi * Cutoff / InputRate;

    for( int n = 0; n < NTaps; n++ )
    {
      float_t mm = n - (NTaps - 1) / float_t( 2 );
      if( mm == 0.0 )
        m_taps[n] = 1 - lambda / detail::Pi;
      else
        m_taps[n] = -std::sin( mm * lambda ) / (mm * detail::Pi);
    }
  }

  float_t filter(float_t sample)
  {
    // push the sample to the front
    for( int i = NTaps - 1; i >= 1; --i )
    {
      m_sr[i] = m_sr[i - 1];
    }
    m_sr[0] = sample;

    return std::inner_product( m_sr.begin(), m_sr.end(), m_taps.begin(), 0.0f );
  }
};

template<int InputRate, int CutoffLow, int CutoffHigh, int NTaps>
class BandPassFilter
{
  static_assert( NTaps > 0 && NTaps <= 100, "Taps out of range" );
  static_assert( InputRate > 0, "Invalid input sample rate" );
  static_assert( CutoffLow > 0 && CutoffLow < InputRate / 2, "Invalid lowpass cutoff frequency" );
  static_assert( CutoffHigh > 0 && CutoffHigh < InputRate / 2, "Invalid highpass cutoff frequency" );
private:
  std::array<float_t, NTaps> m_taps = detail::createZeroArray<float_t, NTaps>();
  std::array<float_t, NTaps> m_sr = detail::createZeroArray<float_t, NTaps>();

public:
  BandPassFilter()
  {
    static constexpr float_t lambda = 2 * detail::Pi * CutoffLow / InputRate;
    static constexpr float_t phi = 2 * detail::Pi * CutoffHigh / InputRate;

    for( int n = 0; n < NTaps; n++ )
    {
      float_t mm = n - (NTaps - 1) / 2.0f;
      if( mm == 0.0 )
        m_taps[n] = (phi - lambda) / detail::Pi;
      else
        m_taps[n] = (std::sin( mm * phi ) - std::sin( mm * lambda )) / (mm * detail::Pi);
    }
  }

  float_t filter(float_t sample)
  {
    // push the sample to the front
    for( int i = NTaps - 1; i >= 1; --i )
    {
      m_sr[i] = m_sr[i - 1];
    }
    m_sr[0] = sample;

    return std::inner_product( m_sr.begin(), m_sr.end(), m_taps.begin(), 0.0f );
  }
};

template<int N, class Filter>
class MultiplexFilter
{
private:
  std::array<Filter, N> m_filters;
public:
  MultiplexFilter()
    : m_filters()
  {
  }

  template<class T>
  void filter(std::array<T, N>& samples)
  {
    for( int i = 0; i < N; ++i )
      samples[i] = static_cast<T>(m_filters[i].filter( samples[i] ));
  }
};
}
