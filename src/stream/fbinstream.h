/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef FBINSTREAM_H
#define FBINSTREAM_H

#include "binstream.h"

/**
 * @class FBinStream
 * @ingroup Common
 * @brief Class derived from BinStream for files
 * @note This is a read-only stream
 */
class FBinStream : public BinStream {
		DISABLE_COPY( FBinStream )
		FBinStream() = delete;
	private:
		std::string m_filename; //!< @brief Filename of the file
	public:
		/**
		 * @brief Default contructor
		 * @param[in] filename Filename of the file to open
		 */
		explicit FBinStream( const std::string& filename );
		/**
		 * @brief Destructor
		 */
		virtual ~FBinStream();
		/**
		 * @brief Check if the file is opened
		 * @return @c true if the file is opened
		 */
		bool isOpen() const;
		/**
		 * @brief Get the filename
		 * @return m_filename
		 */
		std::string filename() const;
};

#endif