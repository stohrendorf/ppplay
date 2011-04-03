/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#ifndef GENBASE_H
#define GENBASE_H

#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>
#include <array>

#include "stuff/utils.h"
#include "stuff/pppexcept.h"
#include "logger/logger.h"
#include "stream/binstream.h"
#include "output/audiofifo.h"

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
	extern const std::array<const char[3], 12> NoteNames;

	/**
	 * @brief Clip/convert a signed sample to a signed 16-bit PCM value
	 * @ingroup GenMod
	 * @param[in] smp Sample to be clipped
	 * @return Clipped 16-bit sample
	 * @note Time-critical
	 */
	inline int16_t clipSample( int32_t smp ) throw() {
		return clip( smp, -32768, 32767 );
	}

	/**
	 * @brief An order list item
	 * @ingroup GenMod
	 */
	class GenOrder : public ISerializable {
			DISABLE_COPY( GenOrder )
			GenOrder() = delete;
		public:
			typedef std::shared_ptr<GenOrder> Ptr; //!< @brief Class pointer
			typedef std::vector<Ptr> Vector;
		private:
			uint8_t m_index; //!< @brief Pattern index of this order
		public:
			/**
			 * @brief Constructor
			 * @param[in] idx Order index
			 */
			GenOrder( uint8_t idx ) throw();
			/**
			 * @brief Return the pattern index associated with this order
			 * @return GenOrder::m_index
			 */
			uint8_t getIndex() const throw();
			/**
			 * @brief Set the pattern index
			 * @param[in] n New index
			 */
			void setIndex( const uint8_t n ) throw();
			virtual IArchive& serialize( IArchive* data );
	};

	/**
	 * @brief Use the old Effect data if the new one is 0x00
	 * @ingroup GenMod
	 * @param[in,out] oldFx Old Effect Data
	 * @param[in,out] newFx New Effect Data
	 */
	inline void combineIfZero( uint8_t& oldFx, uint8_t& newFx ) {
		if( newFx == 0 )
			newFx = oldFx;
		else
			oldFx = newFx;
		newFx = oldFx;
	}
	
	/**
	 * @brief Use the old Effect data nibble if one of the new Effect nibbles is 0
	 * @ingroup GenMod
	 * @param[in,out] oldFx Old Effect Data
	 * @param[in,out] newFx New Effect Data
	 */
	inline void combineNibblesIfZero( uint8_t& oldFx, uint8_t& newFx ) {
		if( newFx == 0 )
			newFx = oldFx;
		else if( highNibble( newFx ) == 0 )
			oldFx = ( newFx & 0x0f ) | ( oldFx & 0xf0 );
		else if( lowNibble( newFx ) == 0 )
			oldFx = ( newFx & 0xf0 ) | ( oldFx & 0x0f );
		else
			oldFx = newFx;
		newFx = oldFx;
	}
} // namespace ppp

#endif
