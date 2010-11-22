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

#ifndef genbaseH
#define genbaseH

#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>

#include "utils.h"
#include "pppexcept.h"
#include "logger.h"
#include "binstream.h"
#include "audiofifo.h"

/**
 * @example ppp.cpp
 */

/**
 * @mainpage Hi folks!
 * @author Syron
 * @version 0.1.1
 * @date 2009
 * @section prereqs_sec Prerequisites
 * OK, you have found the docs... Well done, less work for me *g* @n
 * To be able to compile PeePeePlayer, make sure you have the SDL sources installed...
 *
 * @section notes Notes
 * PeePeePlayer is an OS-independent module playing framework.
 *
 * @section inspiration Inspired by...
 * PeePeePlayer was inspired by OpenCubicPlayer (http://www.cubic.org).
 * I tried @e several @e hours to understand that code... The messiest thing I ever saw. @n
 * So, I decided to start my own, platform-independent player.
 *
 * @section thanks Thanks fly out to...
 * The creators of DosBox! Without DosBox I wouldn't have found that many bugs and quirks.
 */

/**
 * @defgroup GenMod Common module definitions
 * @brief This module contains abstract base classes for the PeePeePlayer Module Interface
 * @details
 * If you want to create your own Module Class, be sure to use these classes to create it.
 *
 * @defgroup Common Common definitions
 * @brief This module contains common things for all things
 */

/**
 * @file
 * @ingroup GenMod
 * @brief Module base declarations
 * @details
 * This file contains several common definitions for all module classes.
 */

/**
 * @namespace ppp
 * @ingroup Common GenMod
 * @brief This namespace is just for grouping things together
 */
namespace ppp {
	/**
	 * @brief General note names
	 * @ingroup Common
	 */
	extern const char NoteNames[12][3];

	/**
	 * @brief Clip/convert a signed sample to a signed 16-bit PCM value
	 * @ingroup GenMod
	 * @param[in] smp Sample to be clipped
	 * @return Clipped 16-bit sample
	 * @note Time-critical
	 */
	inline int16_t clipSample(int32_t smp) throw() {
		return clip(smp, -32768, 32767);
	}

	/**
	 * @brief An order list item
	 * @ingroup GenMod
	 */
	class GenOrder {
		public:
			typedef std::shared_ptr<GenOrder> Ptr; //!< @brief Class pointer
			typedef std::vector<Ptr> Vector;
		private:
			uint8_t m_index; //!< @brief Pattern index of this order
			uint8_t m_count; //!< @brief Playback count of this order
			std::vector<BinStream::SpBinStream> m_states; //!< @brief Buffer for storing channel states for seeking
		public:
			/**
			 * @brief Constructor
			 * @param[in] idx Order index
			 */
			GenOrder(uint8_t idx) throw();
			/**
			 * @brief Get a state buffer
			 * @param[in] idx Buffer index
			 * @return Pointer to the buffer
			 * @exception PppException Thrown if @a idx is out of range
			 */
			BinStream::SpBinStream getState(uint16_t idx) throw(PppException);
			/**
			 * @brief Get the current state buffer
			 * @return Pointer to the buffer
			 * @exception PppException Thrown if GenOrder::m_count is out of range
			 */
			BinStream::SpBinStream getCurrentState() throw(PppException);
			/**
			 * @brief Return the pattern index associated with this order
			 * @return GenOrder::m_index
			 */
			uint8_t getIndex() const throw();
			/**
			 * @brief Set the pattern index
			 * @param[in] n New index
			 */
			void setIndex(const uint8_t n) throw();
			/**
			 * @brief Get the playback count
			 * @return GenOrder::m_count
			 */
			uint8_t getCount() const throw();
			/**
			 * @brief Set the playback count
			 * @param[in] n New playback count
			 */
			void setCount(const uint8_t n) throw();
			/**
			 * @brief Increase GenOrder::m_count
			 * @return New count
			 * @exception PppException Thrown if @c GenOrder::m_count==0xff before increasement
			 */
			uint8_t incCount() throw(PppException);
			/**
			 * @brief Sets GenOrder::m_count to 0
			 */
			void resetCount() throw();
	};

	/**
	 * @ingroup Common
	 * @brief Sinus lookup table
	 * @note Length is 256, amplitude is 64
	 */
	extern const int16_t SinLookup[256];

	/**
	 * @ingroup Common
	 * @brief Linear ramp lookup table
	 * @note Length is 256, amplitude is 64
	 */
	extern const int16_t RampLookup[256];

	/**
	 * @ingroup Common
	 * @brief Square lookup table
	 * @note Length is 256, amplitude is 64
	 */
	extern const int16_t SquareLookup[256];

	/**
	 * @ingroup Common
	 * @brief Protracker compatible Sinus lookup table
	 * @note Length is 256, amplitude is 256
	 */
	extern const int16_t ProtrackerLookup[256];

} // namespace ppp

#endif
