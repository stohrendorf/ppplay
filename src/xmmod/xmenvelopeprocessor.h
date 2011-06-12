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


#ifndef XMENVELOPEPROCESSOR_H
#define XMENVELOPEPROCESSOR_H

#include <array>
#include <cstdint>
#include <string>

namespace ppp {
	namespace xm {
		class XmEnvelopeProcessor
		{
			public:
				enum class EnvelopeFlags : uint8_t {
					Enabled = 0x01,
					Sustain = 0x02,
					Loop = 0x04
				};
				struct EnvelopePoint {
					int16_t position;
					int16_t value;
				};
			private:
				EnvelopeFlags m_flags;
				std::array<EnvelopePoint, 12> m_points;
				uint8_t m_numPoints;
				uint16_t m_position;
				uint8_t m_nextIndex;
				uint8_t m_sustainPoint;
				uint8_t m_loopStart;
				uint8_t m_loopEnd;
				int16_t m_currentRate;
				uint16_t m_currentValue;
				bool onSustain(uint8_t idx) const;
				bool atLoopEnd(uint8_t idx) const;
			public:
				XmEnvelopeProcessor();
				XmEnvelopeProcessor(EnvelopeFlags flags, const std::array<EnvelopePoint, 12>& points, uint8_t numPoints, uint8_t sustainPt, uint8_t loopStart, uint8_t loopEnd);
				void increasePosition(bool keyOn);
				bool enabled() const;
				uint8_t realVolume(uint8_t volume, uint8_t globalVolume, uint16_t scale);
				void setPosition(uint8_t pos);
				std::string toString() const;
		};

		inline bool operator&(const XmEnvelopeProcessor::EnvelopeFlags& a, const XmEnvelopeProcessor::EnvelopeFlags& b) {
			return static_cast<uint8_t>(a) & static_cast<uint8_t>(b);
		}
	}
}

#endif // XMENVELOPEPROCESSOR_H
