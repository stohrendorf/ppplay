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


#include "xmenvelopeprocessor.h"
#include "stuff/utils.h"

namespace ppp {
	namespace xm {
		XmEnvelopeProcessor::XmEnvelopeProcessor()
			: m_flags(), m_points(), m_numPoints(0), m_position(0xffff), m_nextIndex(0), m_sustainPoint(0), m_loopStart(0), m_loopEnd(0), m_currentRate(0), m_currentValue(0)
		{
		}
		XmEnvelopeProcessor::XmEnvelopeProcessor(XmEnvelopeProcessor::EnvelopeFlags flags, const std::array<EnvelopePoint, 12>& points, uint8_t numPoints, uint8_t sustainPt, uint8_t loopStart, uint8_t loopEnd)
			: m_flags(flags), m_points(points), m_numPoints(numPoints), m_position(0xffff), m_nextIndex(0), m_sustainPoint(sustainPt), m_loopStart(loopStart), m_loopEnd(loopEnd), m_currentRate(0), m_currentValue(0)
		{
		}
		void XmEnvelopeProcessor::increasePosition(bool keyOn)
		{
			if(!enabled()) {
				return;
			}
			m_position++;
			if(m_position == m_points[m_nextIndex].position) {
				m_currentValue = m_points[m_nextIndex].value<<8;
				uint8_t tmpNextIdx = m_nextIndex+1;
				uint8_t oldIdx = m_nextIndex;
				if((m_flags & EnvelopeFlags::Loop) && (oldIdx == m_loopEnd)) {
					// we're at the loop end
					if(!(m_flags & EnvelopeFlags::Sustain) || oldIdx!=m_sustainPoint || !keyOn) {
						// no sustain -> loop!
						tmpNextIdx = m_loopStart+1;
						m_position = m_points[m_loopStart].position;
						m_currentValue = m_points[m_loopStart].value<<8;
					}
				}
				if(tmpNextIdx < m_numPoints) {
					if(!(m_flags & EnvelopeFlags::Sustain) || !keyOn || oldIdx!=m_sustainPoint) {
						m_nextIndex = tmpNextIdx;
						int dx = m_points[tmpNextIdx+1].position - m_points[tmpNextIdx].position;
						if(dx>0) {
							int16_t dy = (m_points[tmpNextIdx+1].value<<8) - (m_points[tmpNextIdx].value<<8);
							m_currentRate = dy / dx;
						}
						else {
							m_currentRate = 0;
						}
					}
					else {
						m_currentRate = 0;
					}
				}
				else {
					m_currentRate = 0;
				}
			}
			else {
				m_currentValue = clip<int>(m_currentValue+m_currentRate, 0, 0xffff);
// 				m_currentValue += m_currentRate;
			}
		}
		bool XmEnvelopeProcessor::enabled() const
		{
			return m_flags & EnvelopeFlags::Enabled;
		}
		uint8_t XmEnvelopeProcessor::realVolume(uint8_t volume, uint8_t globalVolume, uint16_t scale)
		{
			if(volume == 0 || globalVolume == 0 || scale == 0) {
				return 0;
			}
			if(!enabled()) {
				return ( ((volume*scale)>>12) * globalVolume )>>8;
			}
			else {
				uint8_t tmp = m_currentValue>>8;
				if(tmp > 0xa0) {
					tmp = 0;
					m_currentRate = 0;
				}
				else if(tmp>0x40) {
					tmp = 0x40;
					m_currentRate = 0;
				}
				return ( (((tmp*volume>>6)*scale)>>12)*globalVolume )>>8;
			}
		}
		std::string XmEnvelopeProcessor::toString() const
		{
			if(!enabled()) {
				return "OFF";
			}
			else {
				return stringf("val=%d rate=%d pos=%d", m_currentValue, m_currentRate, m_position);
			}
		}
	}
}