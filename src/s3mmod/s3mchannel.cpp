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

#include "genmod/genbase.h"
#include "s3mchannel.h"
#include "s3mpattern.h"
#include "s3mmodule.h"
#include "logger/logger.h"

/**
 * @file
 * @ingroup S3mMod
 * @brief S3M Channel Definitions, implementation
 */

namespace ppp {
	namespace s3m {

		static const std::array<const int16_t, 64> S3mWaveSine = {
			{
				0, 24, 49, 74, 97, 120, 141, 161,
				180, 197, 212, 224, 235, 244, 250, 253,
				255, 253, 250, 244, 235, 224, 212, 197,
				180, 161, 141, 120, 97, 74, 49, 24,
				0, -24, -49, -74, -97, -120, -141, -161,
				-180, -197, -212, -224, -235, -244, -250, -253,
				-255, -253, -250, -244, -235, -224, -212, -197,
				-180, -161, -141, -120, -97, -74, -49, -24
			}
		};
		static const std::array<const int16_t, 64> S3mWaveRamp = {
			{
				0, -0xF8, -0xF0, -0xE8, -0xE0, -0xD8, -0xD0, -0xC8,
				-0xC0, -0xB8, -0xB0, -0xA8, -0xA0, -0x98, -0x90, -0x88,
				-0x80, -0x78, -0x70, -0x68, -0x60, -0x58, -0x50, -0x48, -0x40,
				-0x38, -0x30, -0x28, -0x20, -0x18, -0x10, -0x8, 0x0, 0x8, 0x10, 0x18,
				0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70,
				0x78, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0,
				0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8
			}
		};
		static const std::array<const int16_t, 64> S3mWaveSquare = {
			{
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0
			}
		};
		static const std::array<const uint16_t, 16> S3mFinetunes = {
			{
				8363, 8413, 8463, 8529, 8581, 8651, 8723, 8757,
				7895, 7941, 7985, 8046, 8107, 8169, 8232, 8280
			}
		};

		/**
		 * @ingroup S3mMod
		 * @brief Note periodic table for frequency calculations
		 */
		static const std::array<uint16_t, 12> Periods = {{1712 << 4, 1616 << 4, 1524 << 4, 1440 << 4, 1356 << 4, 1280 << 4, 1208 << 4, 1140 << 4, 1076 << 4, 1016 << 4,  960 << 4,  907 << 4}};

		/**
		 * @brief Get the octave out of a note
		 * @ingroup S3mMod
		 * @param[in] x Note value
		 * @return Octave of @a x
		 * @note Time-critical
		 */
		static inline uint8_t S3M_OCTAVE( const uint8_t x ) {
			return highNibble( x );
		}

		/**
		 * @brief Get the note out of a note
		 * @ingroup S3mMod
		 * @param[in] x Note value
		 * @return Note of @a x
		 * @note Time-critical
		 */
		static inline uint8_t S3M_NOTE( const uint8_t x ) {
			return lowNibble( x );
		}

		/**
		 * @brief A value for frequency calculation
		 * @ingroup S3mMod
		 */
		static const uint32_t FRQ_VALUE = 14317056;

		/**
		 * @brief Calculate the period for a given note, octave and base frequency
		 * @ingroup S3mMod
		 * @param[in] note Note value
		 * @param[in] oct Note octave
		 * @param[in] c4spd Base frequency of the sample
		 * @return S3M Period for note @a note and base frequency @a c4spd
		 * @see S3mSample st3Period
		 */
		static inline uint16_t st3PeriodEx( uint8_t note, uint8_t oct, uint16_t c4spd, uint16_t finetune = 8363 ) throw( PppException ) {
			PPP_TEST( c4spd == 0 );
			return ( Periods[note] >> oct ) * finetune / c4spd;
		}
		/**
		 * @brief Calculate the period for a given note and base frequency
		 * @ingroup S3mMod
		 * @param[in] note Note value
		 * @param[in] c4spd Base frequency of the sample
		 * @return S3M Period for note @a note and base frequency @a c4spd
		 * @see S3mSample st3PeriodEx
		 */
		static inline uint16_t st3Period( uint8_t note, uint16_t c4spd, uint16_t finetune = 8363 ) throw( PppException ) {
			return st3PeriodEx( S3M_NOTE( note ), S3M_OCTAVE( note ), c4spd, finetune );
		}

		/**
		 * @brief Reverse calculate a note from a given period and C4 frequency
		 * @ingroup S3mMod
		 * @param[in] per Note period
		 * @param[in] c4spd Base frequency of the sample
		 * @return Note offset (12*octave+note)
		 */
		static inline uint8_t periodToNoteOffset( uint16_t per, uint16_t c4spd, uint16_t finetune = 8363 ) {
			return -12 * std::log2( static_cast<float>( per ) * c4spd / ( finetune * Periods[0] ) );
		}

		/**
		 * @brief Reverse-calculate the Note from the given period
		 * @ingroup S3mMod
		 * @param[in] per Period
		 * @param[in] c2spd Base frequency of the sample
		 * @return Note string
		 * @note Time-critical
		 * @todo OPTIMIZE!!!
		 */
		static inline std::string periodToNote( uint16_t per, uint16_t c2spd, uint16_t finetune = 8363 ) throw() {
			if( per == 0 )
				return "p??";
			if( c2spd == 0 )
				return "c??";
			// per = (8363<<4)*( Periods[S3M_NOTE( note )] >> S3M_OCTAVE( note ) ) / c4spd;
			// per*c4spd/(8363<<4) == Periods[note] * (2^-octave)
			//                     ~= Periods[0] * (2^-(octave+note/12))
			// log2( per*c4spd/(8363<<4 * Periods[0]) ) == -(octave+note/12)
			uint8_t totalnote = periodToNoteOffset( per, c2spd, finetune );
			uint8_t minnote = totalnote % 12;
			uint8_t minoct = totalnote / 12;
			if( ( minoct > 9 ) || ( minnote > 11 ) ) {
				return "???";
			}
			return stringf( "%s%d", NoteNames[minnote], minoct );
		}

		/**
		 * @brief Add/subtract semitones to/from a note
		 * @ingroup S3mMod
		 * @param[in] note Base note
		 * @param[in] delta Delta value
		 * @return New note
		 */
		static inline uint8_t deltaNote( uint8_t note, int8_t delta ) throw() {
			uint16_t x = S3M_OCTAVE( note ) * 12 + S3M_NOTE( note ) + delta;
			return ( ( x / 12 ) << 4 ) | ( x % 12 );
		}
	} // namespace s3m
} // namespace ppp

using namespace ppp;
using namespace ppp::s3m;

static uint16_t clipPeriod(bool amiga, uint16_t period) {
	if(amiga)
		return clip<uint16_t>(period, 0x15c, 0xd60);
	else
		return clip<uint16_t>(period, 0x40, 0x7fff);
}

static uint16_t clipPeriodMax(bool amiga, uint16_t period) {
	if(amiga)
		return std::min<uint16_t>( period, 0xd60 );
	else
		return std::min<uint16_t>( period, 0x7fff );
}

void S3mChannel::setSampleIndex( int32_t idx ) {
	m_sampleIndex = idx;
	if( !currentSample() || currentSample()->getBaseFrq() == 0 )
		setActive( false );
}

S3mChannel::S3mChannel( uint16_t frq, S3mModule* const module ) throw() : GenChannel( frq ),
	m_note( ::s3mEmptyNote ),
	m_lastFxByte(0),
	m_lastVibratoData( 0 ),
	m_lastPortaSpeed( 0 ),
	m_tremorVolume( 0 ),
	m_noteChanged( false ),
	m_currentVolume( 0 ),
	m_realVolume( 0 ),
	m_baseVolume( 0 ),
	m_tremorMute( false ),
	m_retrigCount( 0 ),
	m_tremorCount( 0 ),
	m_zeroVolCounter( 0 ),
	m_module(module),
	m_basePeriod( 0 ),
	m_realPeriod( 0 ),
	m_portaTargetPeriod( 0 ),
	m_vibratoPhase( 0 ),
	m_vibratoWaveform( 0 ),
	m_countdown( 0 ),
	m_tremorCounter( 0 ),
	m_glissando( false ),
	m_currentCell( new S3mCell() ),
	m_bresen(1,1),
	m_sampleIndex( 101 )
{
}

S3mChannel::~S3mChannel() throw() {
}

S3mSample::Ptr S3mChannel::currentSample() throw( PppException ) {
	if( !inRange<int>( m_sampleIndex, 0, m_module->numSamples() - 1 ) ) {
		setActive( false );
		return S3mSample::Ptr();
	}
	return m_module->sampleAt(m_sampleIndex);
}

std::string S3mChannel::getNoteName() throw( PppException ) {
	if( m_note == s3mEmptyNote )
		return "   ";
	if( ( !isActive() ) || isDisabled() )
		return "   ";
// 	return NoteNames[S3M_NOTE(aNote)]+toString(S3M_OCTAVE(aNote));
	return periodToNote( m_realPeriod, currentSample()->getBaseFrq() );
}

std::string S3mChannel::getFxName() const throw() {
	if( !m_currentCell )
		return "...";
	if( !m_currentCell->isActive() ) {
		return "...";
	}
	if( m_currentCell->getEffect() == s3mEmptyCommand ) {
		return "...";
	}
	uint8_t fx = m_currentCell->getEffect();
	bool fxRangeOk = inRange<int>( fx, 1, 27 );
	if( fxRangeOk )
		return stringf( "%c%.2x", fx + 'A' - 1, m_currentCell->getEffectValue() );
	else {
		LOG_ERROR( "Effect out of range: 0x%.2x", fx );
		return stringf( "?%.2x", m_currentCell->getEffectValue() );
	}
}

void S3mChannel::update( S3mCell::Ptr const cell, bool noRetrigger ) throw() {
	if( isDisabled() )
		return;
	
	recalcFrequency();
	recalcVolume();
	
	if(m_module->tick() == 0) {
		if( !cell )
			m_currentCell->reset();
		else
			*m_currentCell = *cell;

		triggerNote();
		if(m_currentCell->getEffect() != 0) {
			m_zeroVolCounter = 3;
		}
		else if(m_module->hasZeroVolOpt()) {
			if(m_currentVolume!=0 && m_currentCell->getVolume()!=0xff && m_currentCell->getInstr()!=0 && m_currentCell->getNote()!=0xff) {
				m_zeroVolCounter = 3;
			}
			else {
				m_zeroVolCounter--;
				if(m_zeroVolCounter == 0)
					return setActive(false);
			}
		}
		
		if(m_currentCell->getEffectValue() != 0)
			m_lastFxByte = m_currentCell->getEffectValue();
		if(m_currentCell->getEffect()==0 || m_currentCell->getEffect()==s3mFxVolSlide) {
			m_countdown = 0;
		}
		if((m_currentCell->getEffect()==s3mFxVolSlide || m_currentCell->getEffect()==0) && m_basePeriod!=m_realPeriod) {
			m_realPeriod = m_basePeriod;
			recalcFrequency();
		}
		if(m_currentCell->getEffect()!=s3mFxTremor) {
			m_tremorCount = 0;
			m_tremorMute = 0;
		}
		
		if(m_currentCell->getEffect()!=s3mFxVibrato
			&& m_currentCell->getEffect()!=s3mFxFineVibrato
			&& m_currentCell->getEffect()!=s3mFxVibVolSlide
			&& m_currentCell->getEffect()!=s3mFxTremolo)
		{
			m_vibratoPhase |= 0x40;
		}
		switch(m_currentCell->getEffect()) {
			case s3mFxSpeed: fxSpeed(m_currentCell->getEffectValue()); break;
			case s3mFxJumpOrder: /*TODO*/ break;
			case s3mFxBreakPat: /*TODO*/ break;
			case s3mFxVolSlide: fxVolSlide(m_currentCell->getEffectValue()); break;
			case s3mFxPitchDown: fxPitchSlideDown(m_currentCell->getEffectValue()); break;
			case s3mFxPitchUp: fxPitchSlideUp(m_currentCell->getEffectValue()); break;
			case s3mFxTremor: fxTremor(m_currentCell->getEffectValue()); break;
			case s3mFxArpeggio: fxArpeggio(m_currentCell->getEffectValue()); break;
			case s3mFxRetrig: fxRetrigger(m_currentCell->getEffectValue()); break;
			case s3mFxSpecial: fxSpecial(m_currentCell->getEffectValue()); break;
		}
	} // if(tick==0)
	else if(m_currentCell->getEffect()!=0) { // if(tick!=0)
		switch(m_currentCell->getEffect()) {
			case s3mFxVolSlide: fxVolSlide(m_currentCell->getEffectValue()); break;
			case s3mFxPitchDown: fxPitchSlideDown(m_currentCell->getEffectValue()); break;
			case s3mFxPitchUp: fxPitchSlideUp(m_currentCell->getEffectValue()); break;
			case s3mFxPorta: fxPorta(m_currentCell->getEffectValue(), false); break;
			case s3mFxVibrato: fxVibrato(m_currentCell->getEffectValue(), false, false); break;
			case s3mFxTremor: fxTremor(m_currentCell->getEffectValue()); break;
			case s3mFxArpeggio: fxArpeggio(m_currentCell->getEffectValue()); break;
			case s3mFxVibVolSlide:
				fxVolSlide(m_currentCell->getEffectValue());
				fxVibrato(m_lastVibratoData, false, true);
				break;
			case s3mFxPortVolSlide:
				fxVolSlide(m_currentCell->getEffectValue());
				fxPorta(m_lastPortaSpeed, true);
				break;
			case s3mFxRetrig: fxRetrigger(m_currentCell->getEffectValue()); break;
			case s3mFxTremolo: fxTremolo(m_currentCell->getEffectValue()); break;
			case s3mFxSpecial: fxSpecial(m_currentCell->getEffectValue()); break;
			case s3mFxFineVibrato: fxVibrato(m_currentCell->getEffectValue(), true, false); break;
			case s3mFxGlobalVol: fxGlobalVolume(m_currentCell->getEffectValue()); break;
		}
	}
	
	updateStatus();
	if( m_module->tick() == 0 )
		m_noteChanged = false;
	if( !m_currentCell->isActive() )
		return;
	if( m_currentCell->getEffect() != s3mFxTremor )
		m_tremorCount = -1;
	else
		m_tremorCount++;
	char smpDelay = 0;
	if( m_currentCell->getEffect() == s3mFxSpecial && highNibble( m_currentCell->getEffectValue() ) == s3mSFxNoteDelay ) {
		smpDelay = lowNibble( m_currentCell->getEffectValue() );
		if( smpDelay == 0 )
			return;
	}
	if( m_currentCell->getEffect() == s3mEmptyCommand && m_currentCell->getEffectValue() != 0x00 )
	m_noteChanged = ( m_currentCell->getNote() != s3mEmptyNote ) && ( m_currentCell->getNote() != s3mKeyOffNote ) && !noRetrigger;
	if( m_module->tick() == smpDelay && !noRetrigger ) {
		if( m_currentCell->getInstr() != s3mEmptyInstr ) {
			setSampleIndex( m_currentCell->getInstr() - 1 );
			if( !currentSample() || currentSample()->getBaseFrq() == 0 ) {
				setActive( false );
				return;
			}
			m_currentVolume = currentSample()->getVolume();
			recalcVolume();
		}
		if( m_currentCell->getVolume() != s3mEmptyVolume ) {
			m_currentVolume = m_currentCell->getVolume();
			recalcVolume();
		}
		if( ( m_currentCell->getNote() != s3mEmptyNote ) && ( m_currentCell->getNote() != s3mKeyOffNote ) ) {
			if( !currentSample() ) {
				setActive( false );
				return;
			}
			if( m_currentCell->getEffect() != s3mFxPorta ) {
				m_note = m_currentCell->getNote();
				m_basePeriod = st3Period( m_note, currentSample()->getBaseFrq() );
				setPosition( 0 );
			}
		}
	}
	if( m_module->tick() == smpDelay ) {
		setActive( ( currentSample() ) && ( m_note != s3mEmptyNote ) );
	}
	if( !isActive() )
		return;
	setActive( isActive() && ( m_currentCell->getNote() != s3mKeyOffNote ) );
}

void S3mChannel::doSpecialFx( uint8_t fx, uint8_t fxVal ) throw( PppException ) {
	int16_t tempVar;
	PPP_TEST( !m_currentCell );
	switch( fx ) {
		case s3mFxOffset:
			reuseIfZero( m_lastFxByte, fxVal );
			if( ( m_module->tick() == 0 ) && ( currentSample() ) && ( m_currentCell->getNote() != s3mEmptyNote ) ) {
				setPosition( fxVal << 8 );
				int32_t pos = getPosition();
				currentSample()->adjustPos( pos );
				setPosition( pos );
				if( getPosition() == GenSample::EndOfSample ) {
					setActive( false );
				}
			}
			break;
		case s3mFxPanSlide:
			reuseIfZero( m_lastFxByte, fxVal );
			if( highNibble( fxVal ) == 0x00 ) {  // panning slide left
				if( m_module->tick() != 0 ) {
					fxVal = lowNibble( fxVal );
					tempVar = getPanning();
					if( tempVar != 0xa4 )
						tempVar -= ( fxVal << 2 );
					setPanning( std::max<int16_t>( tempVar, 0 ) );
				}
			}
			else if( lowNibble( fxVal ) == 0x00 ) { // panning slide right
				if( m_module->tick() != 0 ) {
					fxVal = highNibble( fxVal );
					tempVar = getPanning();
					if( tempVar != 0xa4 )
						tempVar += ( fxVal << 2 );
					setPanning( std::min<int16_t>( tempVar, 0x80 ) );
				}
			}
			else if( highNibble( fxVal ) == 0x0f ) { // fine panning slide left
				if( m_module->tick() == 0 ) {
					fxVal = lowNibble( fxVal );
					if( fxVal == 0x00 )
						fxVal = 0x0f;
					tempVar = getPanning();
					if( tempVar != 0xa4 )
						tempVar -= fxVal;
					setPanning( std::max<int16_t>( tempVar, 0 ) );
				}
			}
			else if( lowNibble( fxVal ) == 0x0f ) { // fine panning slide right
				if( m_module->tick() == 0 ) {
					fxVal = highNibble( fxVal );
					if( fxVal == 0x00 )
						fxVal = 0x0f;
					tempVar = getPanning();
					if( tempVar != 0xa4 )
						tempVar += fxVal;
					setPanning( std::min<int16_t>( tempVar, 0x80 ) );
				}
			}
			break;
		case s3mFxSpecial:
			reuseIfZero( m_lastFxByte, fxVal );
			switch( highNibble( fxVal ) ) {
				case s3mSFxNoteDelay:
					fxNoteDelay( m_currentCell->getEffectValue() );
					break;
				case s3mSFxPatLoop:
				case s3mSFxPatDelay:
					// not handled here, because they're global (or not...)
					break;
				case s3mSFxSetPan:
					setPanning( lowNibble( fxVal ) * 0x80 / 0x0f );
					break;
				case s3mSFxStereoCtrl:
					if( lowNibble( fxVal ) <= 7 )
						setPanning( ( lowNibble( fxVal ) + 8 ) * 0x80 / 0x0f );
					else
						setPanning( ( lowNibble( fxVal ) - 8 ) * 0x80 / 0x0f );
					break;
				case s3mSFxNoteCut:
					if( lowNibble( fxVal ) > 0 ) {
						if( m_module->tick() - 1 == lowNibble( fxVal ) )
							setActive( false );
					}
					break;
				case s3mSFxFunkRpt:
					LOG_MESSAGE( "Funk Repeat not supported" );
					break;
				case s3mSFxSetFilter:
					LOG_MESSAGE( "Set Filter not supported" );
					break;
				case s3mSFxSetFinetune:
					fxFineTune( m_currentCell->getEffectValue() );
					break;
					//LOG_WARNING( "Set Finetune (currently) not implemented" );
					//break;
				case s3mSFxSetGlissando:
					m_glissando = fxVal != 0;
					LOG_WARNING( "Set Glissando Control is experimental" );
					break;
				default:
					LOG_WARNING( "UNSUPPORTED SPECIAL FX FOUND: %s", getFxName().c_str() );
					break;
			}
			break;
		case s3mFxSetPanning:
			if( ( fxVal <= 0x80 ) || ( fxVal == 0xa4 ) )
				setPanning( fxVal );
			else
				LOG_WARNING( "Panning value out of range: 0x%.2x", fxVal );
	}
}

void S3mChannel::mixTick( MixerFrameBuffer& mixBuffer ) throw( PppException ) {
	if( isDisabled() )
		return;
	if( !isActive() || !currentSample() || m_basePeriod == 0 ) {
		setActive( false );
		return;
	}
	if( m_module->tick() == 0 && m_zeroVolCounter != -1 && isActive() ) {
		if( m_currentVolume == 0 )
			m_zeroVolCounter++;
		else
			m_zeroVolCounter = 0;
		if( m_zeroVolCounter == 3 ) {
			m_zeroVolCounter = 0;
			setActive( false );
			m_note = s3mEmptyNote;
			return;
		}
	}
	if( !isActive() )
		return;
	LOG_TEST_ERROR( getPlaybackFrq() == 0 );
	LOG_TEST_ERROR( mixBuffer->size() == 0 );
	if( getPlaybackFrq() * mixBuffer->size() == 0 ) {
		setActive( false );
		return;
	}
	m_bresen.reset( getPlaybackFrq(), FRQ_VALUE / m_realPeriod );
	recalcVolume();
	uint16_t currVol = m_realVolume; //clip( getVolume() + m_deltaVolume, 0, 0x40 ) * m_globalVol;
	MixerSample* mixBufferPtr = &mixBuffer->front().left;
	S3mSample::Ptr currSmp = currentSample();
	int32_t pos = getPosition();
	uint8_t volL = 0x40, volR = 0x40;
	if( getPanning() > 0x40 && getPanning() != 0xa4 )
		volL = 0x80 - getPanning();
// 	//LOG_DEBUG("VolL=%d", volL);
	if( getPanning() < 0x40 )
		volR = getPanning();
	else if( getPanning() == 0xa4 )
		volR = 0xa4;
	for( std::size_t i = 0; i < mixBuffer->size(); i++ ) {
		int16_t sampleVal = currSmp->getLeftSampleAt( pos );
		if( sampleVal != 0xa4 )
			sampleVal = ( sampleVal * volL ) >> 6;
		*( mixBufferPtr++ ) += ( sampleVal * currVol ) >> 6;
		sampleVal = currSmp->getRightSampleAt( pos );
		if( volR == 0xa4 )
			sampleVal = -sampleVal;
		else
			sampleVal = ( sampleVal * volR ) >> 6;
		*( mixBufferPtr++ ) += ( sampleVal * currVol ) >> 6;
		if( pos == GenSample::EndOfSample )
			break;
		m_bresen.next( pos );
	}
	if( pos != GenSample::EndOfSample )
		currentSample()->adjustPos( pos );
	setPosition( pos );
	if( pos == GenSample::EndOfSample )
		setActive( false );
}

void S3mChannel::simTick( std::size_t bufSize ) {
	if( isDisabled() )
		return;
	if( !isActive() || !currentSample() || m_basePeriod == 0 )
		return setActive( false );
	if( m_module->tick() == 0 && m_zeroVolCounter != -1 && isActive() ) {
		if( m_currentVolume == 0 )
			m_zeroVolCounter++;
		else
			m_zeroVolCounter = 0;
		if( m_zeroVolCounter == 3 ) {
			m_zeroVolCounter = 0;
			setActive( false );
			m_note = s3mEmptyNote;
			return;
		}
	}
	PPP_TEST( getPlaybackFrq() == 0 );
	PPP_TEST( bufSize == 0 );
	if( m_basePeriod == 0 ) {
		setActive( false );
		setPosition( 0 );
		return;
	}
	if( !isActive() )
		return;
	int32_t pos = getPosition() + ( FRQ_VALUE / getPlaybackFrq() * bufSize / m_realPeriod );
	currentSample()->adjustPos( pos );
	if( pos == GenSample::EndOfSample )
		setActive( false );
	setPosition( pos );
}

std::string S3mChannel::getCellString() {
	if( !m_currentCell )
		return std::string();
	return m_currentCell->trackerString();
}

std::string S3mChannel::getFxDesc() const throw( PppException ) {
	PPP_TEST( !m_currentCell );
	if( !m_currentCell->isActive() )
		return "      ";
	if( m_currentCell->getEffect() == s3mEmptyCommand )
		return "      ";
	switch( m_currentCell->getEffect() ) {
		case s3mFxSpeed:
			return "Speed\x7f";
		case s3mFxJumpOrder:
			return "JmOrd\x1a";
		case s3mFxBreakPat:
			return "PBrk \xf6";
		case s3mFxVolSlide:
			if( highNibble( m_lastFxByte ) == 0x0f ) {
				if( lowNibble( m_lastFxByte ) == 0 )
					return "VSld \x1e";
				else
					return "VSld \x19";
			}
			else if( lowNibble( m_lastFxByte ) == 0x0f ) {
				if( highNibble( m_lastFxByte ) == 0 )
					return "VSld \x1f";
				else
					return "VSld \x18";
			}
			else {
				if( highNibble( m_lastFxByte ) == 0 )
					return "VSld \x1f";
				else
					return "VSld \x1e";
			}
		case s3mFxPitchDown:
			if( highNibble( m_lastFxByte ) == 0x0f )
				return "Ptch \x1f";
			else if( highNibble( m_lastFxByte ) == 0x0e )
				return "Ptch \x19";
			else
				return "Ptch\x1f\x1f";
		case s3mFxPitchUp:
			if( highNibble( m_lastFxByte ) == 0x0f )
				return "Ptch \x1e";
			else if( highNibble( m_lastFxByte ) == 0x0e )
				return "Ptch \x18";
			else
				return "Ptch\x1e\x1e";
		case s3mFxPorta:
			return "Porta\x12";
		case s3mFxVibrato:
			return "Vibr \xf7";
		case s3mFxTremor:
			return "Tremr\xec";
		case s3mFxArpeggio:
			return "Arp  \xf0";
		case s3mFxVibVolSlide:
			return "VibVo\xf7";
		case s3mFxPortVolSlide:
			return "PrtVo\x12";
		case s3mFxChanVolume:
			return "ChVol\x04";
		case s3mFxChanVolSlide:
			return "ChVSl\x07";
		case s3mFxOffset:
			return "Offs \xaa";
		case s3mFxPanSlide:
			return "PanSl\x1d";
		case s3mFxRetrig:
			return "Retr \xec";
		case s3mFxTremolo:
			return "Tremo\xec";
		case s3mFxSpecial:
			switch( highNibble( m_lastFxByte ) ) {
				case s3mSFxSetFilter:
					return "Fltr \xe0";
				case s3mSFxSetGlissando:
					return "Gliss\xcd";
				case s3mSFxSetFinetune:
					return "FTune\xe6";
				case s3mSFxSetVibWave:
					return "VWave\x9f";
				case s3mSFxSetTremWave:
					return "TWave\x9f";
				case s3mSFxSetPan:
					return "StPan\x1d";
				case s3mSFxStereoCtrl:
					return "SCtrl\x1d";
				case s3mSFxPatLoop:
					return "PLoop\xe8";
				case s3mSFxNoteCut:
					return "NCut \xd4";
				case s3mSFxNoteDelay:
					return "Delay\xc2";
				case s3mSFxPatDelay:
					return "PDly \xc2";
				case s3mSFxFunkRpt:
					return "FRpt ^";
				default:
					return "Specl ";
			}
		case s3mFxTempo:
			return "Tempo\x7f";
		case s3mFxFineVibrato:
			return "FnVib\xf7";
		case s3mFxGlobalVol:
			return "GloVol";
		case s3mFxGlobVolSlide:
			return "GVolSl";
		case s3mFxSetPanning:
			return "StPan\x1d";
		case s3mFxPanbrello:
			return "Panbr\x1d";
		case s3mFxResonance:
			return "Reson ";
	}
	return "??????";
}


void S3mChannel::updateStatus() throw() {
	S3mSample::Ptr smp = currentSample();
	if( !smp ) {
		setActive( false );
		setStatusString( "         ..." );
		return;
	}
	if( isActive() ) {
		std::string panStr;
		if( getPanning() == 0xa4 )
			panStr = "Srnd ";
		else if( getPanning() == 0x00 )
			panStr = "Left ";
		else if( getPanning() == 0x40 )
			panStr = "Centr";
		else if( getPanning() == 0x80 )
			panStr = "Right";
		else
			panStr = stringf( "%4d%%", ( getPanning() - 0x40 ) * 100 / 0x40 );
		std::string volStr = stringf( "%3d%%", clip<int>( m_currentVolume , 0, 0x40 ) * 100 / 0x40 );
		setStatusString( stringf( "%.2d: %s%s %s %s P:%s V:%s %s",
		                          m_sampleIndex + 1,
		                          ( m_noteChanged ? "*" : " " ),
		                          getNoteName().c_str(),
		                          getFxName().c_str(),
		                          getFxDesc().c_str(),
		                          panStr.c_str(),
		                          volStr.c_str(),
		                          smp->getTitle().c_str()
		                        ) );
	}
	else {
		setStatusString( stringf( "     %s %s %s",
		                          ( m_currentCell->getNote() == s3mKeyOffNote ? "^^ " : "   " ),
		                          getFxName().c_str(),
		                          getFxDesc().c_str()
		                        ) );
	}
}

IArchive& S3mChannel::serialize( IArchive* data ) {
	GenChannel::serialize( data )
	& m_note& m_lastFxByte& m_lastVibratoData& m_lastPortaSpeed& m_tremorVolume
	& m_noteChanged
	& m_retrigCount& m_tremorCount
	& m_zeroVolCounter& m_basePeriod& m_glissando;
	data->archive( &m_bresen );
	return data->archive( m_currentCell.get() );
}

void S3mChannel::fxPitchSlideUp(uint8_t fxByte) {
	if(m_basePeriod == 0)
		return;
	reuseIfZero( m_lastFxByte, fxByte );
	if(m_module->tick() == 0) {
		if(fxByte <= 0xe0)
			return;
		if(fxByte <= 0xf0)
			fxByte = lowNibble(fxByte);
		else
			fxByte = lowNibble(fxByte)<<2;
	}
	else {
		if(fxByte >= 0xe0)
			return;
		fxByte = lowNibble(fxByte)<<2;
	}
	m_basePeriod = m_realPeriod = std::max( 0, m_basePeriod-fxByte );
	recalcFrequency();
}

void S3mChannel::fxPitchSlideDown(uint8_t fxByte) {
	if(m_basePeriod == 0)
		return;
	reuseIfZero(m_lastFxByte, fxByte);
	if(m_module->tick() == 0) {
		if(fxByte <= 0xe0)
			return;
		if(fxByte <= 0xf0)
			fxByte = lowNibble(fxByte);
		else
			fxByte = lowNibble(fxByte)<<2;
	}
	else {
		if(fxByte >= 0xe0)
			return;
		fxByte = lowNibble(fxByte)<<2;
	}
	m_basePeriod = m_realPeriod = std::min( 0x7fff, m_basePeriod + fxByte );
	recalcFrequency();
}

void S3mChannel::fxVolSlide(uint8_t fxByte) {
	reuseIfZero(m_lastFxByte, fxByte);
	if(lowNibble(fxByte) == 0x0f) {
		if(highNibble(fxByte)==0)
			m_currentVolume = std::max( 0, m_currentVolume-lowNibble(fxByte) );
		else if(m_module->tick()!=0)
			return;
		else
			m_currentVolume = std::min( 63, m_currentVolume+highNibble(fxByte) );
	}
	else if(highNibble(fxByte) == 0x0f) {
		if(lowNibble(fxByte) == 0)
			m_currentVolume = std::min( 63, m_currentVolume+highNibble(fxByte) );
		else if(m_module->tick()==0)
			m_currentVolume = std::max( 0, m_currentVolume-lowNibble(fxByte) );
	}
	else if(m_module->hasFastVolSlides() || m_module->tick()!=0) {
		if(lowNibble(fxByte)==0)
			m_currentVolume = std::min( 63, m_currentVolume+highNibble(fxByte) );
		else
			m_currentVolume = std::max( 0, m_currentVolume-lowNibble(fxByte) );
	}
	recalcVolume();
}

void S3mChannel::recalcFrequency() {
	if(m_module->hasAmigaLimits())
		m_basePeriod = clipPeriod(true, m_basePeriod);
	uint16_t per = m_realPeriod;
	if(m_module->hasAmigaLimits())
		per = clipPeriodMax(true, m_realPeriod);
	if(per == 0) {
		return;
	}
	m_realPeriod = clipPeriod(m_module->hasAmigaLimits(), per);
}

void S3mChannel::recalcVolume() {
	m_realVolume = (static_cast<int>(m_module->globalVolume())*m_currentVolume) >> 6;
}

void S3mChannel::fxPorta(uint8_t fxByte, bool noReuse) {
	if(m_module->tick()==0)
		return;
	if(m_basePeriod == 0) {
		if(m_portaTargetPeriod == 0)
			return;
		m_realPeriod = m_basePeriod = m_portaTargetPeriod;
		fxByte = m_portaTargetPeriod&0xff;
	}
	if(!noReuse)
		reuseIfZero( m_lastPortaSpeed, fxByte );
	//m_currentCell->setEffectValue(fxByte);
	if(m_basePeriod == m_portaTargetPeriod)
		return;
	if(m_basePeriod > m_portaTargetPeriod) {
		LOG_DEBUG("base=%u > target=%u", m_basePeriod, m_portaTargetPeriod);
		int tmp = m_basePeriod-(fxByte<<2);
		if(tmp<m_portaTargetPeriod)
			tmp = m_portaTargetPeriod;
		if(m_glissando)
			tmp = glissando(tmp);
		m_realPeriod = m_basePeriod = tmp;
		recalcFrequency();
	}
	else {
		LOG_DEBUG("base=%u < target=%u", m_basePeriod, m_portaTargetPeriod);
		int tmp = m_basePeriod+(fxByte<<2);
		if(tmp>m_portaTargetPeriod)
			tmp = m_portaTargetPeriod;
		if(m_glissando)
			tmp = glissando(tmp);
		m_realPeriod = m_basePeriod = tmp;
		recalcFrequency();
	}
}

void S3mChannel::fxVibrato(uint8_t fxByte, bool fine, bool noReuse) {
	if(!noReuse)
		reuseIfZero( m_lastVibratoData, fxByte );
	if(m_basePeriod==0)
		return;
	int val = 0;
	switch(m_vibratoWaveform & 7) {
		case 0:
		case 4:
			if((m_vibratoPhase&0x40)!=0 && (m_vibratoWaveform&7)==0)
				val = S3mWaveSine[0];
			else
				val = S3mWaveSine[m_vibratoPhase&0x3f];
			break;
		case 1:
		case 5:
			if((m_vibratoPhase&0x40)!=0 && (m_vibratoWaveform&7)==1)
				val = S3mWaveRamp[0];
			else
				val = S3mWaveRamp[m_vibratoPhase&0x3f];
			break;
		case 2:
		case 6:
			if((m_vibratoPhase&0x40)!=0 && (m_vibratoWaveform&7)==2)
				val = S3mWaveSquare[0];
			else
				val = S3mWaveSquare[m_vibratoPhase&0x3f];
			break;
		case 3:
		case 7:
			if((m_vibratoPhase&0x40)!=0 && (m_vibratoWaveform&7)==3)
				val = S3mWaveSine[0];
			else
				val = S3mWaveSine[m_vibratoPhase&0x3f];
			m_vibratoPhase = (m_vibratoPhase+(std::rand()&0x0f))&0x7f;
			break;
	}
	m_vibratoPhase = (m_vibratoPhase+highNibble(fxByte)) & 0x7f;
	val = (val*lowNibble(fxByte))>>4;
	if(m_module->st2Vibrato())
		val >>= 1;
	if(fine)
		val >>= 2;
	m_realPeriod = m_basePeriod+val;
	recalcFrequency();
	LOG_DEBUG("Vibrato: base=%u, real=%u", m_basePeriod, m_realPeriod);
}

void S3mChannel::fxNoteCut(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	if(m_module->tick() == 0) {
		m_countdown = fxByte;
		return;
	}
	if(m_countdown==0)
		return;
	m_countdown--;
	if(m_countdown==0)
		setActive(false);
}

void S3mChannel::fxNoteDelay(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	if(m_module->tick() == 0) {
		m_countdown = fxByte;
		return;
	}
	if(m_countdown==0)
		return;
	m_countdown--;
	if(m_countdown==0)
		triggerNote();
}

void S3mChannel::fxGlobalVolume(uint8_t fxByte) {
	if(fxByte<=64)
		m_module->setGlobalVolume(fxByte);
}

void S3mChannel::fxFineTune(uint8_t fxByte) {
	fxByte = lowNibble(fxByte);
	uint16_t ft = S3mFinetunes[fxByte];
	// TODO apply finetune
	m_basePeriod = m_realPeriod;
	recalcFrequency();
}

void S3mChannel::fxSetVibWaveform(uint8_t fxByte) {
	m_vibratoWaveform = lowNibble(fxByte);
}

void S3mChannel::fxRetrigger(uint8_t fxByte) {
	reuseIfZero(m_lastFxByte, fxByte);
	if(lowNibble(fxByte)==0 || lowNibble(fxByte)>m_countdown) {
		m_countdown++;
		return;
	}
	m_countdown = 0;
	setPosition(0);
	int nvol = m_currentVolume;
	switch(highNibble(fxByte)) {
		case 0x0:
			break;
		case 0x1:
			nvol -= 1;
			break;
		case 0x2:
			nvol -= 2;
			break;
		case 0x3:
			nvol -= 4;
			break;
		case 0x4:
			nvol -= 8;
			break;
		case 0x5:
			nvol -= 16;
			break;
		case 0x6:
			nvol = nvol * 2 / 3;
			break;
		case 0x7:
			nvol /= 2;
			break;
		case 0x8:
			break;
		case 0x9:
			nvol += 1;
			break;
		case 0xa:
			nvol += 2;
			break;
		case 0xb:
			nvol += 4;
			break;
		case 0xc:
			nvol += 8;
			break;
		case 0xd:
			nvol += 16;
			break;
		case 0xe:
			nvol = nvol * 3 / 2;
			break;
		case 0xf:
			nvol *= 2;
			break;
	}
	m_currentVolume = clip<int>(nvol, 0, 63);
	recalcVolume();
}

void S3mChannel::fxOffset(uint8_t fxByte) {
	setPosition(fxByte<<8);
}

void S3mChannel::fxTremor(uint8_t fxByte) {
	reuseIfZero(m_lastFxByte, fxByte);
	if(m_tremorCounter != 0) {
		m_tremorCounter--;
		return;
	}
	if(m_tremorMute) {
		m_tremorMute = false;
		m_currentVolume = 0;
		recalcVolume();
		m_tremorCounter = lowNibble(fxByte);
	}
	else {
		m_tremorMute = true;
		m_currentVolume = m_baseVolume;
		recalcVolume();
		m_tremorCounter = highNibble(fxByte);
	}
}

void S3mChannel::fxTempo(uint8_t fxByte) {
	if(fxByte<=0x20)
		return;
	m_module->setTempo(fxByte);
}

void S3mChannel::fxSpeed(uint8_t fxByte) {
	if(fxByte==0)
		return;
	m_module->setSpeed(fxByte);
}

void S3mChannel::fxArpeggio(uint8_t fxByte) {
	if( !currentSample() )
		return;
	reuseIfZero( m_lastFxByte, fxByte );
	switch( m_module->tick() % 3 ) {
		case 0: // normal note
			m_realPeriod = st3Period( m_note, currentSample()->getBaseFrq() );
			break;
		case 1: // +x half notes...
			m_realPeriod = st3Period( deltaNote( m_note, highNibble( m_lastFxByte ) ), currentSample()->getBaseFrq() );
			break;
		case 2: // +y half notes...
			m_realPeriod = st3Period( deltaNote( m_note, lowNibble( m_lastFxByte ) ), currentSample()->getBaseFrq() );
			break;
	}
	recalcFrequency();
}

void S3mChannel::fxSpecial(uint8_t fxByte) {
	// TODO
}

void S3mChannel::fxTremolo(uint8_t fxByte) {
	// TODO
}

uint16_t S3mChannel::glissando(uint16_t period) {
	uint8_t no = periodToNoteOffset( period, currentSample()->getBaseFrq() );
	return st3PeriodEx( no % 12, no / 12, currentSample()->getBaseFrq() );
}

void S3mChannel::triggerNote() {
	uint8_t instr = m_currentCell->getInstr();
	if(instr>=101)
		instr = 0;
	if(instr!=0) {
		setSampleIndex(instr-1);
		if(currentSample()) {
			m_currentVolume = currentSample()->getVolume();
			recalcVolume();
		}
	}
	if(m_currentCell->getNote()!=0xff) {
		if(currentSample()) {
			uint16_t p = st3Period(m_currentCell->getNote(), currentSample()->getBaseFrq());
			if(m_currentCell->getEffect() != s3mFxPorta) {
				m_realPeriod = m_basePeriod = m_portaTargetPeriod = p;
				LOG_DEBUG("Set porta target = %u (base=%u, real=%u)", m_portaTargetPeriod, m_basePeriod, m_realPeriod);
				recalcFrequency();
			}
			else {
				/*m_basePeriod =*/ m_portaTargetPeriod = p;
				LOG_DEBUG("PORTA (target=%u, base=%u, real=%u)", m_portaTargetPeriod, m_basePeriod, m_realPeriod);
			}
		}
	}
	uint8_t vol = m_currentCell->getVolume();
	if(vol!=0xff) {
		if(vol>63)
			vol = 63;
		m_currentVolume = m_baseVolume = vol;
		recalcVolume();
	}
}
