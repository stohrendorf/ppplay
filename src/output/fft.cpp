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

#include "fft.h"

#include <boost/assert.hpp>

#include <array>
#include <cmath>

/*
copied and modified from: http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/dft/fft_ms.c

Modification of Paul Bourkes FFT code by Peter Cusack
to utilise the Microsoft complex type.

This computes an in-place complex-to-complex FFT
x and y are the real and imaginary arrays of 2^m points.
dir =  1 gives forward transform
dir = -1 gives reverse transform
*/

namespace
{
constexpr bool reverseFFT = false;

template<typename T>
struct SimpleComplex
{
  typedef T Type;
  Type real;
  Type imag;

  explicit constexpr SimpleComplex(Type r = 0, Type i = 0) noexcept
    : real( r ), imag( i )
  {
  }

  template<typename U>
  constexpr SimpleComplex<Type> operator*(const SimpleComplex<U>& rhs) const noexcept
  {
    return SimpleComplex<Type>( real * rhs.real - imag * rhs.imag, real * rhs.imag + imag * rhs.real );
  }

  template<typename U>
  const SimpleComplex<Type>& operator*=(const SimpleComplex<U>& rhs) noexcept
  {
    Type r = real * rhs.real - imag * rhs.imag;
    imag = real * rhs.imag + imag * rhs.real;
    real = r;
    return *this;
  }

  template<typename U>
  constexpr SimpleComplex<Type> operator+(const SimpleComplex<U>& rhs) const noexcept
  {
    return SimpleComplex<Type>( real + rhs.real, imag + rhs.imag );
  }

  template<typename U>
  const SimpleComplex<Type>& operator+=(const SimpleComplex<U>& rhs) noexcept
  {
    real += rhs.real;
    imag += rhs.imag;
    return *this;
  }

  template<typename U>
  constexpr SimpleComplex<Type> operator-(const SimpleComplex<U>& rhs) const noexcept
  {
    return SimpleComplex<Type>( real - rhs.real, imag - rhs.imag );
  }

  template<typename U>
  const SimpleComplex<Type>& operator-=(const SimpleComplex<U>& rhs) noexcept
  {
    real -= rhs.real;
    imag -= rhs.imag;
    return *this;
  }

  constexpr Type length() const noexcept
  {
    return std::sqrt( real * real + imag * imag );
  }
};

void DFFT(std::array<SimpleComplex<float>, ppp::FFT::InputLength>& data)
{
  // re-sort values by inverting the index bits
  for( uint_fast16_t i = 0, j = 0; i < ppp::FFT::InputLength - 1; i++ )
  {
    if( i < j )
    {
      std::swap( data[i], data[j] );
    }
    uint_fast16_t k = ppp::FFT::InputLength / 2;
    while( k <= j )
    {
      j -= k;
      k >>= 1;
    }
    j += k;
  }
  SimpleComplex<float> expFacMul( -1, 0 );
  // recursive level loop
  for( uint_fast8_t level = 0; level < ppp::FFT::InputBits; level++ )
  {
    SimpleComplex<float> expFac( 1, 0 );
    for( uint_fast16_t section = 0; section < uint16_t( 1 << level ); section++ )
    {
      for( uint_fast16_t left = section; left < ppp::FFT::InputLength; left += 2 << level )
      {
        const uint_fast16_t right = left + (1 << level);
        SimpleComplex<float> deltaRight = expFac * data[right];
        data[right] = data[left] - deltaRight;
        data[left] += deltaRight;
      }
      expFac *= expFacMul;
    }
    expFacMul.imag = std::sqrt( (1.0f - expFacMul.real) / 2.0f );
    if( !reverseFFT )
    {
      expFacMul.imag = -expFacMul.imag;
    }
    expFacMul.real = std::sqrt( (1.0f + expFacMul.real) / 2.0f );
  }
  /*	if (!reverseFFT)
      {
              for (unsigned short i = 0; i < inputLength; i++)
                      data[i] /= inputLength;
      }   */
}

void samplesToComplex(const BasicSample* smpPtr, std::array<SimpleComplex<float>, ppp::FFT::InputLength>* dest)
{
  BOOST_ASSERT( smpPtr != nullptr );
  BOOST_ASSERT( dest != nullptr );
  for( size_t i = 0; i < ppp::FFT::InputLength; i++ )
  {
    (*dest)[i] = SimpleComplex<float>( *smpPtr / 32768.0f, 0 );
    smpPtr += 2;
  }
}

void fftToAmp(const std::array<SimpleComplex<float>, ppp::FFT::InputLength>& fft, std::vector<uint16_t>* dest)
{
  dest->resize( ppp::FFT::InputLength / 2 );
  uint16_t* ampsPtr = &dest->front();
  for( size_t i = 0; i < ppp::FFT::InputLength / 2; i++ )
  {
    *(ampsPtr++) = static_cast<uint16_t>(fft[i].length() * std::sqrt( i ));
  }
}
} // anonymous namespace

namespace ppp
{
namespace FFT
{
void doFFT(const AudioFrameBufferPtr& samples, std::vector<uint16_t>* L, std::vector<uint16_t>* R)
{
  BOOST_ASSERT( L != nullptr );
  BOOST_ASSERT( R != nullptr );
  BOOST_ASSERT( samples );

  std::array<SimpleComplex<float>, ppp::FFT::InputLength> cArr;

  samplesToComplex( &samples->front().left, &cArr );
  DFFT( cArr );
  fftToAmp( cArr, L );

  samplesToComplex( &samples->front().right, &cArr );
  DFFT( cArr );
  fftToAmp( cArr, R );
}
}
}
