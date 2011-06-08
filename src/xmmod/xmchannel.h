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

#include "genmod/breseninter.h"
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
			uint8_t m_lastOffsetFx;
			uint8_t m_baseNote;
			uint8_t m_realNote;
			uint8_t m_instrumentIndex;
			uint8_t m_volFadeoutVal;
			uint16_t m_panEnvPos;
			uint8_t m_panEnvIdx;
			uint16_t m_volEnvPos;
			uint8_t m_volEnvIdx;
			uint16_t m_volEnvVal;
			uint8_t m_lastFinePortaUpFx;
			uint8_t m_lastFinePortaDownFx;
			uint8_t m_lastFineVolUpFx;
			uint8_t m_lastFineVolDownFx;
			uint8_t m_lastXFinePortaUp;
			uint8_t m_lastXFinePortaDown;
			uint8_t m_lastGlobVolSlideFx;
			uint16_t m_portaSpeed;
			uint16_t m_portaTargetPeriod;
			uint8_t m_vibratoSpeed;
			uint8_t m_vibratoDepth;
			uint8_t m_vibratoPhase;
			uint8_t m_vibratoCtrl;
			bool m_glissandoCtrl;
			BresenInterpolation m_bres;
			XmModule* m_module;
			XmSample::Ptr currentSample();
			XmInstrument::Ptr currentInstrument();
			XmCell m_currentCell;
			XmSample::Ptr m_currentSample;
			XmInstrument::Ptr m_currentInstrument;
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
			void fxSetVolume(uint8_t fxByte);
			void fxSetPan(uint8_t fxByte);
			void fxSetTempoBpm(uint8_t fxByte);
			void fxOffset(uint8_t fxByte);
			void fxSetGlobalVolume(uint8_t fxByte);
			void fxExtraFinePorta(uint8_t fxByte);
			void fxExtended(uint8_t fxByte);
			void fxPorta();
			void fxArpeggio(uint8_t fxByte);
			void fxVibrato(uint8_t fxByte);
			void fxGlobalVolSlide(uint8_t fxByte);
			void vfxFineVolSlideDown(uint8_t fxByte);
			void vfxFineVolSlideUp(uint8_t fxByte);
			void vfxSetPan(uint8_t fxByte);
			void vfxSlideDown(uint8_t fxByte);
			void vfxSlideUp(uint8_t fxByte);
			void vfxSetVibratoSpeed(uint8_t fxByte);
			void vfxPanSlideLeft(uint8_t fxByte);
			void vfxPanSlideRight(uint8_t fxByte);
			void vfxVibrato(uint8_t fxByte);
			void efxFinePortaUp(uint8_t fxByte);
			void efxFinePortaDown(uint8_t fxByte);
			void efxFineVolUp(uint8_t fxByte);
			void efxFineVolDown(uint8_t fxByte);
			void triggerNote();
			void doKeyOff();
			void doVibrato();
			void applySampleDefaults();
			void calculatePortaTarget(uint8_t targetNote);
		};
	}
}

#endif