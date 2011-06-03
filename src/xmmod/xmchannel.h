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

#ifndef XMCHANNEL_H
#define XMCHANNEL_H

#include "genmod/genchannel.h"
#include "xmpattern.h"
#include "xminstrument.h"

namespace ppp {
	namespace xm {
		class XmModule;
		class XmChannel : public GenChannel {
			DISABLE_COPY(XmChannel)
			XmChannel() = delete;
		public:
			typedef std::shared_ptr<XmChannel> Ptr;
			typedef std::vector<Ptr> Vector;
		private:
			uint8_t m_baseVolume;
			uint8_t m_currentVolume;
			uint8_t m_panning;
			uint16_t m_basePeriod;
			uint16_t m_currentPeriod;
			int8_t m_finetune;
			uint8_t m_lastVolSlideFx;
			uint8_t m_lastPortaUpFx;
			uint8_t m_lastPortaDownFx;
			uint8_t m_lastPanSlideFx;
			uint8_t m_baseNote;
			uint8_t m_realNote;
			uint8_t m_instrumentIndex;
			XmModule* m_module;
			XmSample::Ptr currentSample() const;
		public:
			XmChannel(XmModule* module, int frq);
			virtual std::string getNoteName() throw( PppException );
			virtual std::string getFxName() const throw( PppException );
			virtual void mixTick( MixerFrameBuffer& mixBuffer ) throw( PppException );
			virtual void simTick( std::size_t bufSize );
			virtual void updateStatus() throw( PppException );
			virtual std::string getFxDesc() const throw( PppException );
			virtual std::string getCellString();
			void update( XmCell::Ptr const cell );
		private:
			void fxPortaUp(uint8_t fxByte);
			void fxPortaDown(uint8_t fxByte);
			void fxVolSlide(uint8_t fxByte);
			void fxPanSlide(uint8_t fxByte);
			void triggerNote(uint8_t note);
			void doKeyOff();
		};
	}
}

#endif