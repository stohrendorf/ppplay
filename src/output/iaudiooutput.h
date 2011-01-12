/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef IAUDIOOUTPUT_H
#define IAUDIOOUTPUT_H

#include "iaudiosource.h"
#include <memory>

/**
 * @defgroup Output Output routines
 * @brief Output routines
 */

/**
 * @class OutputGen
 * @ingroup Output
 * @brief Abstract base class for sound output
 */
class IAudioOutput {
		DISABLE_COPY(IAudioOutput)
		IAudioOutput() = delete;
	private:
		IAudioSource* m_source; //!< @brief The audio source
	public:
		typedef std::shared_ptr<IAudioOutput> Ptr;
		explicit IAudioOutput(IAudioSource* src) : m_source(src) {}
		virtual ~IAudioOutput();
		/**
		 * @brief Initialize output
		 * @param[in] desiredFrq Desired output frequency
		 * @return Either the real output frequency, or 0 if the call failed
		 */
		virtual int init(int desiredFrq) = 0;
		virtual bool playing() volatile = 0;
		virtual bool paused() volatile = 0;
		virtual bool stopped() volatile = 0;
		virtual void play() = 0;
		virtual void pause() = 0;
		virtual void stop() = 0;
		IAudioSource* source() const { return m_source; }
		virtual uint16_t volumeLeft() const = 0;
		virtual uint16_t volumeRight() const = 0;
};

#endif
