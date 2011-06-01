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

#ifndef S3MCHANNEL_H
#define S3MCHANNEL_H

#include "genmod/genchannel.h"
#include "genmod/breseninter.h"
#include "s3mbase.h"
#include "s3msample.h"
#include "s3mpattern.h"

#include <array>

/**
* @file
* @ingroup S3mMod
* @brief S3M Channel Definitions
*/

namespace ppp {
	namespace s3m {
		class S3mModule;
		/**
		 * @class S3mChannel
		 * @ingroup S3mMod
		 * @brief The S3M Channel
		 */
		class S3mChannel : public GenChannel {
				S3mChannel() = delete; //!< @brief No default constructor
				DISABLE_COPY( S3mChannel )
			public:
				typedef std::shared_ptr<S3mChannel> Ptr;
				typedef std::vector<Ptr> Vector;
			private:
				uint8_t m_note;          //!< @brief Currently playing note
				uint8_t m_lastFxByte;        //!< @brief Last FX Value
				uint8_t m_lastVibratoData; //!< @brief Last Vibrato FX
				uint8_t m_lastPortaSpeed;
				uint8_t m_tremorVolume;  //!< @brief Backup variable for Tremor FX
				bool m_noteChanged;            //!< @brief @c true when a new note triggered in the current frame
				uint8_t m_currentVolume;
				uint8_t m_realVolume;
				uint8_t m_baseVolume;
				bool m_tremorMute;
				int8_t m_retrigCount;     //!< @brief Used for Retrigger Effect
				int16_t m_zeroVolCounter;        //!< @brief Zero Volume Optimization counter, -1 if disabled
				S3mModule* const m_module;
				uint16_t m_basePeriod; //!< @brief The channel's period without the sample's c4speed applied
				uint16_t m_realPeriod;
				uint16_t m_portaTargetPeriod;
				uint8_t m_vibratoPhase;
				uint8_t m_vibratoWaveform;
				uint8_t m_tremoloPhase;
				uint8_t m_tremoloWaveform;
				uint8_t m_countdown;
				uint8_t m_tremorCounter;
				uint16_t m_c2spd;
				bool m_glissando;
				S3mCell m_currentCell;
				BresenInterpolation m_bresen;
				std::string m_currentFxStr;
				int m_sampleIndex;

				S3mSample::Ptr currentSample() throw( PppException );
				void setSampleIndex( int32_t idx );
				
				// new implementation
				void fxPitchSlideUp(uint8_t fxByte);
				void fxPitchSlideDown(uint8_t fxByte);
				void fxVolSlide(uint8_t fxByte);
				void fxPorta(uint8_t fxByte, bool noReuse);
				void fxVibrato(uint8_t fxByte, bool fine, bool noReuse);
				void fxNoteCut(uint8_t fxByte);
				void fxNoteDelay(uint8_t fxByte);
				void fxGlobalVolume(uint8_t fxByte);
				void fxFineTune(uint8_t fxByte);
				void fxSetVibWaveform(uint8_t fxByte);
				void fxSetTremWaveform(uint8_t fxByte);
				void fxRetrigger(uint8_t fxByte);
				void fxOffset(uint8_t fxByte);
				void fxTremor(uint8_t fxByte);
				void fxTempo(uint8_t fxByte);
				void fxSpeed(uint8_t fxByte);
				void fxArpeggio(uint8_t fxByte);
				void fxSpecial(uint8_t fxByte);
				void fxTremolo(uint8_t fxByte);

				void triggerNote();
				void playNote();
				void recalcFrequency();
				uint16_t glissando(uint16_t period);
			public:
				/**
				 * @copydoc GenChannel::GenChannel
				 */
				S3mChannel( uint16_t frq, S3mModule* const module ) throw();
				virtual ~S3mChannel() throw();
				virtual std::string getNoteName() throw( PppException );
				/**
				 * @brief Update the channel
				 * @param[in] cell Pointer to a note cell
				 * @param[in] tick Current tick
				 * @param[in] patDelay For pattern delays
				 * @remarks A new value in the Instrument Column changes the instrument with the old playback position
				 * @note Time-critical
				 */
				void update( const S3mCell::Ptr cell, bool patDelay = false ) throw();
				virtual void mixTick( MixerFrameBuffer& mixBuffer ) throw( PppException );
				virtual void simTick( std::size_t bufSize );
				virtual void updateStatus() throw();
				virtual IArchive& serialize( IArchive* data );
				virtual std::string getCellString();
				virtual std::string getFxName() const throw( PppException );
				virtual std::string getFxDesc() const throw( PppException );
				void recalcVolume();
		};
	} // namespace s3m
} // namespace ppp

#endif
