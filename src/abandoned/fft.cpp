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

#include "logger/logger.h"
#include "utils.h"

// #include <cmath>
#include <complex>
//#include <cstdint>

// #include <iostream>

using namespace ppp::FFT;

constexpr uint8_t  inputBits    = 11;
constexpr uint16_t inputLength  = 1 << inputBits;
constexpr uint16_t inputLength2 = inputLength / 2;

namespace ppp {
	namespace FFT {
		const size_t fftSampleCount = inputLength;
	}
}
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

std::complex<float> cArr[inputLength];

void DFFT() {
	uint16_t j = 0;
	for( uint16_t i = 0; i < inputLength - 1; i++ ) {
		if( i < j )
			std::swap( cArr[i], cArr[j] );
		uint16_t k = inputLength2;
		while( k <= j ) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
	std::complex<float> c( -1, 0 );
	for( uint8_t l = 0; l < inputBits; l++ ) {
		std::complex<float> u( 1, 0 );
		for( j = 0; j < ( 1 << l ); j++ ) {
			for( uint16_t i = j; i < inputLength; i += 2 << l ) {
				uint16_t i1 = i + ( 1 << l );
				std::complex<float> t1 = u * cArr[i1];
				cArr[i1] = cArr[i] - t1;
				cArr[i] += t1;
			}
			u *= c;
		}
		c.imag( sqrt( ( 1.0 - c.real() ) / 2.0 ) );
		if( !reverseFFT )
			c.imag( -c.imag() );
		c.real( sqrt( ( 1.0 + c.real() ) / 2.0 ) );
	}
	/*	if (!reverseFFT)
		{
			for (unsigned short i = 0; i < inputLength; i++)
				cArr[i] /= inputLength;
		}   */
}

void prepare( BasicSample* smpPtr ) {
	for( uint16_t i = 0; i < inputLength; i++ )
		cArr[i] = std::complex<float>( *( smpPtr += 2 ) / 32768.0f, 0 );
}

void post( AmpsData& amps ) {
	amps.reset( new std::vector<uint16_t>( inputLength2 ) );
	uint16_t* ampsPtr = &amps->front();
	for( uint16_t i = 0; i < inputLength2; i++ ) {
		*( ampsPtr++ ) = abs( cArr[i] ) * sqrt( i );
	}
}

} // anonymous namespace

namespace ppp {
	namespace FFT {
		void doFFT( AudioFrameBuffer& samples, AmpsData& L, AmpsData& R ) {
			prepare( &samples->front().left );
			DFFT();
			post( L );
			prepare( &samples->front().right );
			DFFT();
			post( R );
		}
	}
}
