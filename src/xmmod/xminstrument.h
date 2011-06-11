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
#include "xmenvelopeprocessor.h"

namespace ppp {
	namespace xm {
		class XmInstrument {
				DISABLE_COPY( XmInstrument )
			private:
				XmSample::Vector m_samples;
				uint8_t m_map[96];
				std::string m_title;
				XmEnvelopeProcessor::EnvelopeFlags m_panEnvFlags;
				XmEnvelopeProcessor::EnvelopeFlags m_volEnvFlags;
				std::array<XmEnvelopeProcessor::EnvelopePoint, 12> m_panPoints;
				std::array<XmEnvelopeProcessor::EnvelopePoint, 12> m_volPoints;
				uint8_t m_numVolPoints;
				uint8_t m_volLoopStart;
				uint8_t m_volLoopEnd;
				uint8_t m_volSustainPoint;
				uint16_t m_fadeout;
			public:
				typedef std::shared_ptr<XmInstrument> Ptr;
				typedef std::vector<Ptr> Vector;
				XmInstrument();
				bool load( BinStream& str );
				/**
				 * @brief Map a note index into its sample index
				 * @param[in] note Note to map
				 * @return Mapped sample index
				 * @note @a note is zero-based!
				 */
				uint8_t mapNoteIndex( uint8_t note ) const;
				/**
				 * @brief Map a note index into its sample
				 * @param[in] note Note to map
				 * @return Mapped sample
				 * @note @a note is zero-based!
				 */
				XmSample::Ptr mapNoteSample( uint8_t note ) const;
				std::string title() const;
				XmEnvelopeProcessor::EnvelopeFlags panEnvFlags() const;
				XmEnvelopeProcessor::EnvelopePoint panPoint(int idx) const;
				uint16_t fadeout() const { return m_fadeout; }
				XmEnvelopeProcessor volumeProcessor() const;
		};
	}
}

#endif // XMINSTRUMENT_H
