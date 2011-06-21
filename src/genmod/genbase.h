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

#ifndef GENBASE_H
#define GENBASE_H

#include "stuff/utils.h"
#include "stuff/pppexcept.h"
#include "logger/logger.h"
#include "stream/iserializable.h"
#include "output/audiofifo.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>
#include <array>

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
 */

/**
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
inline int16_t clipSample(int32_t smp) throw() {
	return clip(smp, -32768, 32767);
}

/**
 * @brief An order list item
 * @ingroup GenMod
 */
class GenOrder : public ISerializable {
		DISABLE_COPY(GenOrder)
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
		GenOrder(uint8_t idx);
		/**
		 * @brief Return the pattern index associated with this order
		 * @return GenOrder::m_index
		 */
		uint8_t index() const;
		/**
		 * @brief Set the pattern index
		 * @param[in] n New index
		 */
		void setIndex(const uint8_t n);
		virtual IArchive& serialize(IArchive* data);
};

} // namespace ppp

#endif
