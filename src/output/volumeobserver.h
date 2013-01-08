/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2013  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef PPPLAY_VOLUMEOBSERVER_H
#define PPPLAY_VOLUMEOBSERVER_H

#include "ppplay_core_export.h"
#include "audiotypes.h"
#include <stuff/utils.h>

class AudioFifo;

/**
 * @ingroup Output
 * @{
 */

/**
 * @class VolumeObserver
 * @brief Observer for calculating the output peaks of an AudioFifo
 */
class PPPLAY_CORE_EXPORT VolumeObserver
{
	DISABLE_COPY(VolumeObserver)
private:
	//! @brief Sum of all left absolute sample values
	uint64_t m_volLeftSum;
	//! @brief Sum of all left absolute sample values (logarithmic)
	uint16_t m_volLeftLog;
	
	//! @brief Sum of all right absolute sample values
	uint64_t m_volRightSum;
	//! @brief Sum of all right absolute sample values (logarithmic)
	uint16_t m_volRightLog;
	
	//! @brief Observed FIFO
	AudioFifo* m_fifo;
public:
	explicit VolumeObserver(AudioFifo* fifo);
	~VolumeObserver();
	
	inline uint16_t leftVol() const
	{
		return m_volLeftLog;
	}
	inline uint16_t rightVol() const
	{
		return m_volRightLog;
	}
	
private:
	void dataPulled(const AudioFrameBuffer& buffer);
	void dataPushed(const AudioFrameBuffer& buffer);
};

/**
 * @}
 */

#endif
