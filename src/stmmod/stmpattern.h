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

#ifndef stmpatternH
#define stmpatternH

#include "genpattern.h"
#include "stmbase.h"

/**
* @file
* @brief STM Pattern class
* @ingroup StmMod
*/

namespace ppp {
	namespace stm {

		enum {
			stmEmptyNote = 0xff,
			stmEmptyInstr = 0x00,
			stmEmptyEffect = 0x00,
			stmEmptyVolume = 0x41,
			stmKeyOffNote = 0xfe
		};
		/**
		 * @class StmCell
		 * @ingroup StmMod
		 * @brief A ScreamTracker 2 and before cell
		 * @details
		 * The format is @code (byte)note (byte)instrVol (byte)volFx (byte)fxVal @endcode
		 * so that
		 * @code
		 * // oooo nnnn : iiii ivvv : VVVV ffff : xxxx xxxx
		 * // Octave = oooo, Note = nnnn, Instrument = i iiii,
		 * // Volume = VVV Vvvv, Effect = ffff, EffectData = xxxx xxxx
		 * realNote = note;
		 * realInstr = (instrVol&0xf8)>>3;
		 * realVol = (instrVol&0x07)|((volFx&0xf0)>>1);
		 * realFx = (volFx&0x0f);
		 * realFxVal = fxVal;
		 * @endcode
		 */
		class StmCell : public GenCell {
			public:
				typedef std::shared_ptr<StmCell> Ptr;
			private:
				unsigned char aNote; //!< @brief Note value
				unsigned char aInstr; //!< @brief Instrument value
				unsigned char aVolume; //!< @brief Volume value
				unsigned char aEffect; //!< @brief Effect
				unsigned char aEffectValue; //!< @brief Effect value
			public:
				StmCell() throw();
				virtual ~StmCell() throw();
				virtual bool load(BinStream &str) throw(PppException);
				virtual void reset() throw();
				virtual std::string trackerString() const throw();
				/**
				 * @brief Assignment operator
				 * @param[in] src Source cell
				 * @return Reference to *this
				 */
				StmCell &operator=(const StmCell &src) throw();
				/**
				 * @brief Get the cell's note
				 * @return #aNote
				 */
				unsigned char getNote() const throw();
				/**
				 * @brief Get the cell's instrument
				 * @return #aInstr
				 */
				unsigned char getInstr() const throw();
				/**
				 * @brief Get the cell's volume
				 * @return #aVolume
				 */
				unsigned char getVolume() const throw();
				/**
				 * @brief Get the cell's effect
				 * @return #aEffect
				 */
				unsigned char getEffect() const throw();
				/**
				 * @brief Get the cell's effect value
				 * @return #aEffectValue
				 */
				unsigned char getEffectValue() const throw();
		};

		/**
		* @class StmPattern
		* @ingroup StmMod
		* @brief Pattern class for STM Patterns
		*/
		class StmPattern : public GenPattern {
			public:
				typedef std::shared_ptr<StmPattern> CP;
			protected:
				virtual GenCell::Ptr createCell(int16_t trackIndex, int16_t row) throw();
			public:
				StmPattern() throw(PppException);
				virtual ~StmPattern() throw();
				virtual bool load(BinStream& str, std::size_t pos) throw(PppException);
		};
	} // namespace ppp
} // namespace s3m

#endif
