/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Syron <mr.syron@googlemail.com>

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

#ifndef S3MCELL_H
#define S3MCELL_H

#include "stream/binstream.h"
#include "genmod/gencell.h"

namespace ppp {
namespace s3m {

/**
 * @class S3mCell
 * @ingroup S3mMod
 * @brief A ScreamTracker cell
 */
class S3mCell : public GenCell {
	public:
		typedef std::shared_ptr<S3mCell> Ptr;
		typedef std::vector<Ptr> Vector;
	private:
		uint8_t m_note; //!< @brief Note value
		uint8_t m_instr; //!< @brief Instrument value
		uint8_t m_volume; //!< @brief Volume value
		uint8_t m_effect; //!< @brief Effect
		uint8_t m_effectValue; //!< @brief Effect value
	public:
		S3mCell();
		virtual ~S3mCell();
		bool load(BinStream& str);
		virtual void reset();
		virtual std::string trackerString() const;
		/**
		 * @brief Get the cell's note
		 * @return m_note
		 */
		uint8_t note() const;
		/**
		 * @brief Get the cell's instrument
		 * @return m_instr
		 */
		uint8_t instrument() const;
		/**
		 * @brief Get the cell's volume
		 * @return m_volume
		 */
		uint8_t volume() const;
		/**
		 * @brief Get the cell's effect
		 * @return m_effect
		 */
		uint8_t effect() const;
		/**
		 * @brief Get the cell's effect value
		 * @return m_effectValue
		 */
		uint8_t effectValue() const;
		virtual IArchive& serialize(IArchive* data);
};

}
}

#endif