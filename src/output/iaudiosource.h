/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2010  Steffen Ohrendorf <email>

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


#ifndef IAUDIOSOURCE_H
#define IAUDIOSOURCE_H

#include "stuff/utils.h"
#include "audiotypes.h"

namespace ppp {
	class IAudioSource {
		public:
			/**
			 * @brief Get audio data from the source
			 * @param[out] buffer The buffer containing the data
			 * @param[in] requestedFrames Number of requested frames
			 * @returns The number of frames actually returned - should be equal to @code buffer->size() @endcode
			 */
			virtual std::size_t getAudioData(AudioFrameBuffer& buffer, std::size_t requestedFrames) = 0;
			virtual ~IAudioSource();
	};
	IAudioSource::~IAudioSource() = default;
} // namespace ppp

#endif // IAUDIOSOURCE_H
