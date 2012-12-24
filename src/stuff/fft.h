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

#ifndef FFT_H
#define FFT_H

#include "output/audiotypes.h"

#include <cstdint>
#include <vector>

#include "ppplay_core_export.h"

namespace ppp {
	namespace FFT {
		static constexpr uint8_t  InputBits    = 11;
		static constexpr uint16_t InputLength  = 1 << InputBits;

		static constexpr size_t FftSampleCount = InputLength;
		void PPPLAY_CORE_EXPORT doFFT( AudioFrameBuffer& samples, std::vector<uint16_t>* L, std::vector<uint16_t>* R );
	}
}


#endif // fftH
