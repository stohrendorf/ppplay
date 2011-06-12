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

#ifndef XMSAMPLE_H
#define XMSAMPLE_H

#include "genmod/gensample.h"

namespace ppp {
	namespace xm {
		class XmSample : public GenSample {
				DISABLE_COPY( XmSample )
			private:
				int8_t m_finetune;
				uint8_t m_panning;
				int8_t m_relativeNote;
				bool m_16bit;
			public:
				typedef std::shared_ptr<XmSample> Ptr;
				typedef std::vector<Ptr> Vector;
				XmSample();
				virtual bool load( BinStream& str, std::size_t pos ) throw( PppException );
				bool loadData( BinStream& str );
				int8_t finetune() const {
					return m_finetune;
				}
				uint8_t panning() const {
					return m_panning;
				}
				int8_t relativeNote() const {
					return m_relativeNote;
				}
				bool is16bit() const {
					return m_16bit;
				}
		};
	}
}

#endif // XMSAMPLE_H
