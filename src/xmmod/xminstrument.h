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

#ifndef XMINSTRUMENT_H
#define XMINSTRUMENT_H

#include "xmsample.h"

namespace ppp {
	namespace xm {
		class XmInstrument {
				DISABLE_COPY( XmInstrument )
			public:
				enum class EnvelopeFlags : uint8_t {
					Enabled = 0x01,
					Sustain = 0x02,
					Loop = 0x04
				};
			private:
				XmSample::Vector m_samples;
				uint8_t m_map[96];
				std::string m_title;
				EnvelopeFlags m_panEnvFlags;
				EnvelopeFlags m_volEnvFlags;
				struct EnvelopePoint {
					int16_t position;
					int16_t value;
				};
				std::vector<EnvelopePoint> m_panPoints;
				std::vector<EnvelopePoint> m_volPoints;
			public:
				typedef std::shared_ptr<XmInstrument> Ptr;
				typedef std::vector<Ptr> Vector;
				XmInstrument();
				bool load( BinStream& str );
				uint8_t mapNoteIndex( uint8_t note ) const;
				XmSample::Ptr mapNoteSample( uint8_t note ) const;
				std::string title() const;
				EnvelopeFlags panEnvFlags() const;
				EnvelopePoint panPoint(int idx) const;
				EnvelopeFlags volEnvFlags() const;
				EnvelopePoint volPoint(int idx) const;
		};
		inline bool operator&(const XmInstrument::EnvelopeFlags& a, const XmInstrument::EnvelopeFlags& b) {
			return static_cast<uint8_t>(a) & static_cast<uint8_t>(b);
		}
	}
}

#endif // XMINSTRUMENT_H
