/*
    PeePeePlayer - an old-fashioned module player
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

#include <complex>
#include <boost/assert.hpp>

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

void DFFT( std::vector<std::complex<float>>& data )
{
    BOOST_ASSERT( data.size() == ppp::FFT::InputLength );
    // re-sort values by inverting the index bits
    for( uint_fast16_t i = 0, j = 0; i < ppp::FFT::InputLength - 1; i++ ) {
        if( i < j ) {
            std::swap( data[i], data[j] );
        }
        uint_fast16_t k = ppp::FFT::InputLength / 2;
        while( k <= j ) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
    std::complex<float> expFacMul( -1, 0 );
    // recursive level loop
    for( uint_fast8_t level = 0; level < ppp::FFT::InputBits; level++ ) {
        std::complex<float> expFac( 1, 0 );
        for( uint_fast16_t section = 0; section < uint16_t( 1 << level ); section++ ) {
            for( uint_fast16_t left = section; left < ppp::FFT::InputLength; left += 2 << level ) {
                uint_fast16_t right = left + ( 1 << level );
                std::complex<float> deltaRight = expFac * data[right];
                data[right] = data[left] - deltaRight;
                data[left] += deltaRight;
            }
            expFac *= expFacMul;
        }
        expFacMul.imag( sqrt( ( 1.0 - expFacMul.real() ) / 2.0 ) );
        if( !reverseFFT ) {
            expFacMul.imag( -expFacMul.imag() );
        }
        expFacMul.real( sqrt( ( 1.0 + expFacMul.real() ) / 2.0 ) );
    }
    /*	if (!reverseFFT)
    	{
    		for (unsigned short i = 0; i < inputLength; i++)
    			data[i] /= inputLength;
    	}   */
}

void samplesToComplex( const BasicSample* smpPtr, std::vector<std::complex<float>>* dest )
{
    BOOST_ASSERT( smpPtr != nullptr );
    BOOST_ASSERT( dest != nullptr );
    dest->resize( ppp::FFT::InputLength );
    for( size_t i = 0; i < ppp::FFT::InputLength; i++ ) {
        ( *dest )[i] = std::complex<float>( *smpPtr / 32768.0f, 0 );
        smpPtr += 2;
    }
}

void fftToAmp( const std::vector<std::complex<float>>& fft, std::vector<uint16_t>* dest )
{
    dest->resize( ppp::FFT::InputLength / 2 );
    uint16_t* ampsPtr = &dest->front();
    for( size_t i = 0; i < ppp::FFT::InputLength / 2; i++ ) {
        *( ampsPtr++ ) = fabs( fft[i] ) * sqrt( i );
    }
}

} // anonymous namespace

namespace ppp
{
namespace FFT
{
void doFFT( const AudioFrameBuffer& samples, std::vector< uint16_t >* L, std::vector< uint16_t >* R )
{
    BOOST_ASSERT( L != nullptr );
    BOOST_ASSERT( R != nullptr );
    BOOST_ASSERT( samples );

    std::vector<std::complex<float>> cArr;

    samplesToComplex( &samples->front().left, &cArr );
    DFFT( cArr );
    fftToAmp( cArr, L );

    samplesToComplex( &samples->front().right, &cArr );
    DFFT( cArr );
    fftToAmp( cArr, R );
}
}
}
