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

#ifndef s3mpatternH
#define s3mpatternH

#include "genmod/genpattern.h"
#include "s3mbase.h"

/**
 * @file
 * @brief S3M Pattern class
 * @ingroup S3mMod
 */

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
				S3mCell() throw();
				virtual ~S3mCell() throw();
				bool load(BinStream &str) throw(PppException);
				virtual void reset() throw();
				virtual std::string trackerString() const throw();
				/**
				 * @brief Get the cell's note
				 * @return #aNote
				 */
				uint8_t getNote() const throw();
				/**
				 * @brief Get the cell's instrument
				 * @return #aInstr
				 */
				uint8_t getInstr() const throw();
				/**
				 * @brief Get the cell's volume
				 * @return #aVolume
				 */
				uint8_t getVolume() const throw();
				/**
				 * @brief Get the cell's effect
				 * @return #aEffect
				 */
				uint8_t getEffect() const throw();
				/**
				 * @brief Get the cell's effect value
				 * @return #aEffectValue
				 */
				uint8_t getEffectValue() const throw();
				virtual IArchive& serialize(IArchive* data);
		};

		/**
		 * @class S3mPattern
		 * @ingroup S3mMod
		 * @brief Pattern class for S3M Patterns
		 */
		class S3mPattern {
			public:
				typedef std::shared_ptr<S3mPattern> Ptr;
				typedef std::vector<Ptr> Vector;
			private:
				std::vector<S3mCell::Vector> m_tracks;
				S3mCell::Ptr createCell(uint16_t trackIndex, int16_t row) throw(PppException);
			public:
				S3mPattern() throw(PppException);
				~S3mPattern() throw();
				bool load(BinStream& str, std::size_t pos) throw(PppException);
				S3mCell::Ptr getCell(uint16_t trackIndex, int16_t row) throw();
		};
	} // namespace s3m
} // namespace ppp

#endif
