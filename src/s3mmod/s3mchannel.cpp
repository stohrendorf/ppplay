/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "genbase.h"
#include "s3mchannel.h"
#include "s3mpattern.h"
#include "breseninter.h"
#include "logger.h"
//#include <cmath>

/**
 * @file
 * @ingroup S3mMod
 * @brief S3M Channel Definitions, implementation
 */

namespace ppp {
	namespace s3m {
		
		const std::array<const int16_t, 64> S3mWaveSine = {{
			0, 24, 49, 74, 97, 120, 141, 161, 180, 197, 212, 224,
			235, 244, 250, 253, 255, 253, 250, 244, 235, 224, 212,
			197, 180, 161, 141, 120, 97, 74, 49, 24, 0, -24, -49,
			-74, -97, -120, -141, -161, -180, -197, -212, -224,
			-235, -244, -250, -253, -255, -253, -250, -244, -235,
			-224, -212, -197, -180, -161, -141, -120, -97, -74,
			-49, -24
		}};
		const std::array<const int16_t, 64> S3mWaveRamp = {{
			0, -0xF8, -0xF0, -0xE8, -0xE0, -0xD8, -0xD0, -0xC8,
			-0xC0, -0xB8, -0xB0, -0xA8, -0xA0, -0x98, -0x90, -0x88,
			-0x80, -0x78, -0x70, -0x68, -0x60, -0x58, -0x50, -0x48, -0x40,
			-0x38, -0x30, -0x28, -0x20, -0x18, -0x10, -0x8, 0x0, 0x8, 0x10, 0x18,
			0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70,
			0x78, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0,
			0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8
		}};
		const std::array<const int16_t, 64> S3mWaveSquare = {{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0
		}};

		/**
		 * @ingroup S3mMod
		 * @brief Note periodic table for frequency calculations
		 */
		static const std::array<uint16_t, 12> Periods = {{1712<<4, 1616<<4, 1524<<4, 1440<<4, 1356<<4, 1280<<4, 1208<<4, 1140<<4, 1076<<4, 1016<<4,  960<<4,  907<<4}};

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
		 * @brief Calculate the period for a given note and base frequency
		 * @ingroup S3mMod
		 * @param[in] note Note value
		 * @param[in] c4spd Base frequency of the sample
		 * @return S3M Period for note @a note and base frequency @a c4spd
		 * @see ::S3mSample
		 * @note Time-critical
		 */
		static inline uint16_t st3PeriodEx( const uint8_t note, const uint8_t oct, uint16_t c4spd ) throw( PppException ) {
			PPP_TEST(c4spd==0);
			return (Periods[note] >> oct) * 8363 / c4spd;
		}
		static inline uint16_t st3Period( const uint8_t note, uint16_t c4spd ) throw( PppException ) {
			return st3PeriodEx(S3M_NOTE(note), S3M_OCTAVE(note), c4spd);
		}

		static inline uint8_t periodToNoteOffset( const uint16_t per, const uint16_t c4spd) {
			return -std::log2( static_cast<float>(per)*c4spd/(8363*Periods[0]) )*12;
		}

		/**
		 * @brief Reverse-calculate the Note from the given frequency
		 * @ingroup S3mMod
		 * @param[in] frq Playback frequency
		 * @param[in] c2spd Base frequency of the sample
		 * @return Note string
		 * @note Time-critical
		 * @todo OPTIMIZE!!!
		 */
		static inline std::string periodToNote( const uint16_t per, const uint16_t c2spd ) throw() {
			if ( per == 0 )
				return "p??";
			if ( c2spd == 0 )
				return "c??";
			// per = (8363<<4)*( Periods[S3M_NOTE( note )] >> S3M_OCTAVE( note ) ) / c4spd;
			// per*c4spd/(8363<<4) == Periods[note] * (2^-octave)
			//                     ~= Periods[0] * (2^-(octave+note/12))
			// log2( per*c4spd/(8363<<4 * Periods[0]) ) == -(octave+note/12)
			uint8_t totalnote = periodToNoteOffset(per,c2spd);
			uint8_t minnote = totalnote%12;
			uint8_t minoct = totalnote/12;
			if (( minoct > 9 ) || ( minnote > 11 ) ) {
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
		 * @note Time-critical
		 */
		static inline uint8_t deltaNote( const uint8_t note, const int8_t delta ) throw() {
			uint16_t x = S3M_OCTAVE( note ) * 12 + S3M_NOTE( note ) + delta;
			return (( x / 12 ) << 4 ) | ( x % 12 );
		}
	} // namespace s3m
} // namespace ppp

using namespace ppp;
using namespace ppp::s3m;

void S3mChannel::setSampleIndex(int32_t idx) {
	GenChannel::setSampleIndex(idx);
	if(!currentSample() || currentSample()->getBaseFrq()==0)
		setActive(false);
}

S3mChannel::S3mChannel( const uint16_t frq, const S3mSample::Vector* const smp ) throw() : GenChannel( frq ),
		m_note( ::s3mEmptyNote ), m_lastFx( 0 ), m_lastPortaSpeed( 0 ), m_lastVibratoData( 0 ),
		m_tremorVolume( 0 ), m_targetNote( ::s3mEmptyNote ), m_noteChanged( false ), m_deltaPeriod( 0 ),
		m_deltaVolume( 0 ), m_globalVol( 0x40 ), m_nextGlobalVol( 0x40 ),
		m_retrigCount( -1 ), m_tremorCount( -1 ), m_300VolSlides( false ), m_amigaLimits( false ), m_immediateGlobalVol( false ),
		m_maybeSchism( false ), m_zeroVolCounter( -1 ), m_sampleList(smp), m_basePeriod(0), m_glissando(false), m_currentCell(new S3mCell())
{
}

S3mChannel::~S3mChannel() throw() {
}

S3mSample::Ptr S3mChannel::currentSample() throw(PppException) {
	PPP_TEST( !m_sampleList );
	if(!inRange<int>(getCurrentSmpIdx(), 0, m_sampleList->size()-1)) {
		setActive(false);
		return S3mSample::Ptr();
	}
	return m_sampleList->at(getCurrentSmpIdx());
}

void S3mChannel::useLastFxData( uint8_t &oldFx, uint8_t &newFx ) const throw() {
	if ( newFx == 0 )
		newFx = oldFx;
	else
		oldFx = newFx;
	newFx = oldFx;
}

void S3mChannel::combineLastFxData( uint8_t &oldFx, uint8_t &newFx ) const throw() {
	if ( newFx == 0 )
		newFx = oldFx;
	else if ( highNibble( newFx ) == 0 )
		oldFx = ( newFx & 0x0f ) | ( oldFx & 0xf0 );
	else if ( lowNibble( newFx ) == 0 )
		oldFx = ( newFx & 0xf0 ) | ( oldFx & 0x0f );
	else
		oldFx = newFx;
	newFx = oldFx;
}

std::string S3mChannel::getNoteName() throw( PppException ) {
	if ( m_note == s3mEmptyNote )
		return "   ";
	if (( !isActive() ) || isDisabled() )
		return "   ";
// 	return NoteNames[S3M_NOTE(aNote)]+toString(S3M_OCTAVE(aNote));
	return periodToNote( getAdjustedPeriod(), currentSample()->getBaseFrq() );
}

std::string S3mChannel::getFxName() const throw() {
	if ( !m_currentCell )
		return "...";
	if ( !m_currentCell->isActive() ) {
		return "...";
	}
	if ( m_currentCell->getEffect() == s3mEmptyCommand ) {
		return "...";
	}
	uint8_t fx = m_currentCell->getEffect();
	bool fxRangeOk = inRange<int>( fx, 1, 27 );
	if ( fxRangeOk )
		return stringf( "%c%.2x", fx + 'A' - 1, m_currentCell->getEffectValue() );
	else {
		LOG_ERROR( "Effect out of range: 0x%.2x", fx );
		return stringf( "?%.2x", m_currentCell->getEffectValue() );
	}
}

uint16_t S3mChannel::basePeriod() {
	if(!currentSample() || currentSample()->getBaseFrq()==0) {
		setActive(false);
		return 0;
	}
	return m_basePeriod;
}

void S3mChannel::setBasePeriod(uint16_t per) {
	if(per==0)
		return setActive(false);
	if(!m_amigaLimits)
		m_basePeriod = clip<uint16_t>(per, 0x40, 0x7fff);
	else
		m_basePeriod = clip<uint16_t>(per, 0xda, 0xd60);
}

uint16_t S3mChannel::getAdjustedPeriod() throw() {
	int res = basePeriod()+m_deltaPeriod;
	if(!isActive())
		return 0;
	if(m_glissando) {
		uint8_t no = periodToNoteOffset(res, currentSample()->getBaseFrq());
		res = st3PeriodEx(no%12, no/12, currentSample()->getBaseFrq());
	}
	if(!m_amigaLimits)
		res = clip<int>(res, 0x40, 0x7fff);
	else
		res = clip<int>(res, 0x1c5, 0xd60);
	return res;
}

void S3mChannel::update( S3mCell::Ptr const cell, const uint8_t tick, bool noRetrigger ) throw() {
	LOG_BEGIN();
	if ( isDisabled() )
		return;
	setTick(tick);
	if ( !cell )
		m_currentCell->reset();
	else
		*m_currentCell = *cell;
	updateStatus();
	if ( tick == 0 )
		m_noteChanged = false;
	if ( !m_currentCell->isActive() )
		return;
	if ( m_currentCell->getEffect() != s3mFxRetrig )
		m_retrigCount = -1;
	else
		m_retrigCount++;
	if ( m_currentCell->getEffect() != s3mFxTremor )
		m_tremorCount = -1;
	else
		m_tremorCount++;
	char smpDelay = 0;
	if (( m_currentCell->getEffect() == s3mFxSpecial ) && ( highNibble( m_currentCell->getEffectValue() ) == s3mSFxNoteDelay ) ) {
		smpDelay = lowNibble( m_currentCell->getEffectValue() );
		if ( smpDelay == 0 )
			return;
	}
	if (( m_currentCell->getEffect() == s3mEmptyCommand ) && ( m_currentCell->getEffectValue() != 0x00 ) )
		m_lastFx = m_currentCell->getEffectValue();
/*	if (( m_currentCell->getEffect() != s3mFxVibrato ) && ( m_currentCell->getEffect() != s3mFxFineVibrato ) && ( m_currentCell->getEffect() != s3mFxVibVolSlide ) ) {
		m_deltaPeriod = 0;
		vibrato().resetPhase();
	}
	if ( m_currentCell->getEffect() != s3mFxTremolo ) {
		m_deltaVolume = 0;
		tremolo().resetPhase();
	}*/
	m_noteChanged = ( m_currentCell->getNote() != s3mEmptyNote ) && ( m_currentCell->getNote() != s3mKeyOffNote ) && !noRetrigger;
	if (( tick == smpDelay ) && !noRetrigger ) {
		if(m_noteChanged)
			m_deltaPeriod = 0;
		if ( m_currentCell->getInstr() != s3mEmptyInstr ) {
			setSampleIndex( m_currentCell->getInstr() - 1 );
			if ( !currentSample() || currentSample()->getBaseFrq()==0 ) {
				setActive( false );
				return;
			}
			setVolume( currentSample()->getVolume() );
		}
		if ( m_currentCell->getVolume() != s3mEmptyVolume ) {
			setVolume( m_currentCell->getVolume() );
			m_deltaVolume = 0;
		}
		if (( m_currentCell->getNote() != s3mEmptyNote ) && ( m_currentCell->getNote() != s3mKeyOffNote ) ) {
			if ( m_currentCell->getEffect() != s3mFxPorta ) {
				m_note = m_currentCell->getNote();
				setBasePeriod(st3Period(m_note, currentSample()->getBaseFrq()));
				setPosition( 0 );
			}
			m_deltaPeriod = 0;
			vibrato().resetPhase();
		}
	}
	if ( m_currentCell->getEffect() == s3mFxPorta ) {
		if (( m_currentCell->getNote() != s3mEmptyNote ) && ( m_currentCell->getNote() != s3mKeyOffNote ) )
			m_targetNote = m_currentCell->getNote();
		if (( m_note == s3mEmptyNote ) && ( m_targetNote != s3mEmptyNote ) ) {
			m_note = m_targetNote;
			setBasePeriod(st3Period(m_note, currentSample()->getBaseFrq()));
		}
	}
	if ( getTick() == smpDelay ) {
		setActive( ( currentSample() ) && ( m_note != s3mEmptyNote ) );
	}
	if ( !isActive() )
		return;
	if ( m_currentCell->getEffect() != s3mEmptyCommand ) {
		switch ( m_currentCell->getEffect() ) {
			case s3mFxSpeed:
			case s3mFxJumpOrder:
			case s3mFxBreakPat:
			case s3mFxTempo:
			case s3mFxGlobalVol:
				// already updated effects...
				break;
			case s3mFxVolSlide:
			case s3mFxTremolo:
			case s3mFxChanVolume:
				doVolumeFx( m_currentCell->getEffect(), m_currentCell->getEffectValue() );
				break;
			case s3mFxPitchDown:
			case s3mFxPitchUp:
			case s3mFxPorta:
				doPitchFx( m_currentCell->getEffect(), m_currentCell->getEffectValue() );
				break;
			case s3mFxVibrato:
			case s3mFxFineVibrato:
				doVibratoFx( m_currentCell->getEffect(), m_currentCell->getEffectValue() );
				break;
			case s3mFxSpecial:
			case s3mFxOffset:
			case s3mFxRetrig:
			case s3mFxTremor:
			case s3mFxArpeggio:
			case s3mFxPanSlide:
			case s3mFxSetPanning:
				if ( !( noRetrigger && ( m_currentCell->getEffect() == s3mFxOffset ) ) )
					doSpecialFx( m_currentCell->getEffect(), m_currentCell->getEffectValue() );
				break;
			case s3mFxVibVolSlide:
				doVolumeFx( s3mFxVibVolSlide, m_currentCell->getEffectValue() );
				doVibratoFx( s3mFxVibVolSlide, m_lastVibratoData );
				break;
			case s3mFxPortVolSlide:
				doVolumeFx( s3mFxPortVolSlide, m_currentCell->getEffectValue() );
				doPitchFx( s3mFxPortVolSlide, m_lastPortaSpeed );
				break;
			default:
				LOG_WARNING( "UNSUPPORTED FX FOUND: %s", getFxName().c_str() );
				break;
		}
	}
	setActive( isActive() && ( m_currentCell->getNote() != s3mKeyOffNote ) );
}

void S3mChannel::doVolumeFx( const uint8_t fx, uint8_t fxVal ) throw() {
	int16_t tempVar;
	uint8_t H, L;
	switch ( fx ) {
		case s3mFxVolSlide:
		case s3mFxVibVolSlide:
		case s3mFxPortVolSlide:
/*			if ( fx == s3mFxVibVolSlide || fx==s3mFxPortVolSlide ) {
				if(getTick()==0)
					break;
				useLastFxData( m_lastVolFx, fxVal );
			}
			else*/
				useLastFxData( m_lastFx, fxVal );
			tempVar = getVolume();
			H = highNibble(fxVal);
			L = lowNibble(fxVal);
			if(H==0x0f) {
				if(L==0)
					tempVar += H;
				else if(getTick()==0)
					tempVar -= L;
			}
			else if(L==0x0f) {
				if(H==0)
					tempVar -= L;
				else if(getTick()==0)
					tempVar += H;
			}
			else if(getTick()!=0 || m_300VolSlides) {
				if(H==0)
					tempVar -= L;
				else
					tempVar += H;
			}
			setVolume( clip<int16_t>( tempVar, 0, 0x40 ) );
			break;
		case s3mFxTremolo:
			combineLastFxData( m_lastFx, fxVal );
			if ( getTick() != 0 ) {
				tremolo() += highNibble( fxVal ) << 2;
				m_deltaVolume = ( tremolo().get() * lowNibble( fxVal ) ) >> 7;
			}
			break;
		case s3mFxChanVolume:
			setVolume( fxVal );
			break;
	}
}

void S3mChannel::doVibratoFx( const uint8_t fx, uint8_t fxVal ) throw() {
	switch ( fx ) {
		case s3mFxVibrato:
		case s3mFxVibVolSlide:
			combineLastFxData(m_lastVibratoData, fxVal);
			if(getTick()==0)
				break;
			vibrato() += highNibble( fxVal ) << 2;
			m_deltaPeriod = vibrato().get() * lowNibble( fxVal ) >> 5;
			break;
		case s3mFxFineVibrato:
			combineLastFxData( m_lastVibratoData, fxVal );
			if ( getTick() == 0 )
				break;
			vibrato() += highNibble( fxVal ) << 2;
			m_deltaPeriod = vibrato().get() * lowNibble( fxVal ) >> 7;
			break;
	}
}

void S3mChannel::doPitchFx( const uint8_t fx, uint8_t fxVal ) throw() {
	switch ( fx ) {
		case s3mFxPitchDown:
			useLastFxData( m_lastFx, fxVal );
			if(getTick()==0) {
				if(fxVal<=0xe0)
					break;
				if(fxVal<=0xf0)
					pitchDown(lowNibble(fxVal)<<2);
				else
					pitchDown(lowNibble(fxVal));
			}
			else {
				if(fxVal>=0xe0)
					break;
				pitchDown(fxVal<<2);
			}
			break;
		case s3mFxPitchUp:
			useLastFxData( m_lastFx, fxVal );
			if(getTick()==0) {
				if(fxVal<=0xe0)
					break;
				if(fxVal<=0xf0)
					pitchUp(lowNibble(fxVal)<<2);
				else
					pitchUp(lowNibble(fxVal));
			}
			else {
				if(fxVal>=0xe0)
					break;
				pitchUp(fxVal<<2);
			}
			break;
		case s3mFxPorta:
			useLastFxData( m_lastPortaSpeed, fxVal );
		case s3mFxPortVolSlide:
/*			if ( fx == s3mFxPortVolSlide && getTick()==0) {
				break;
			}
			else {
				useLastFxData( m_lastPortaSpeed, fxVal );
				m_lastVolFx = m_lastPortaSpeed;
			}
			fxVal = m_lastPortaSpeed;*/
			if ( currentSample() && ( getTick() != 0 ) ) {
				int32_t basePer = 0;
				int32_t targetPer = 0;
				targetPer = st3Period( m_targetNote, currentSample()->getBaseFrq() ); // calculate target frq
				if ( m_basePeriod == 0 )
					setBasePeriod(targetPer);
				else {
					if ( targetPer > m_basePeriod ) { // pitch down...
						basePer = m_basePeriod + (fxVal<<2);
						if ( basePer >= targetPer ) {
							setBasePeriod(targetPer);
							m_note = m_targetNote;
						}
						else
							setBasePeriod(basePer);
					}
					else { // pitch up...
						basePer = m_basePeriod - (fxVal<<2);
						if( basePer < 0 )
							basePer = 0;
						if ( basePer <= targetPer ) {
							setBasePeriod(targetPer);
							m_note = m_targetNote;
						}
						else
							setBasePeriod(basePer);
					}
				}
			}
			break;
	}
}

void S3mChannel::doSpecialFx( const uint8_t fx, uint8_t fxVal ) throw( PppException ) {
	LOG_BEGIN();
	int16_t nvol = getVolume();
	int16_t tempVar;
	PPP_TEST( !m_currentCell );
	switch ( fx ) {
		case s3mFxOffset:
			useLastFxData( m_lastFx, fxVal );
			if (( getTick() == 0 ) && ( currentSample() ) && ( m_currentCell->getNote() != s3mEmptyNote ) ) {
				setPosition( fxVal << 8 );
				int32_t pos = getPosition();
				currentSample()->adjustPos( pos );
				setPosition(pos);
				if ( getPosition() == GenSample::EndOfSample ) {
					setActive( false );
				}
			}
			break;
		case s3mFxRetrig:
			useLastFxData( m_lastFx, fxVal );
			if ( lowNibble( fxVal ) == 0 )
				break;
			m_retrigCount %= lowNibble( fxVal );
			if ( m_retrigCount == 0 )
				setPosition( 0 );
			switch ( highNibble( fxVal ) ) {
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
			setVolume( clip<int16_t>( nvol, 0, 0x40 ) );
			break;
		case s3mFxTremor:
			useLastFxData( m_lastFx, fxVal );
			if (( m_tremorCount == 0 ) && ( m_tremorVolume == 0 ) )
				m_tremorVolume = getVolume();
			nvol = m_tremorCount % ( lowNibble( fxVal ) + highNibble( fxVal ) + 2 );
			if ( nvol == lowNibble( fxVal ) + 1 )
				setVolume( 0 );
			else if ( nvol == 0 )
				setVolume( m_tremorVolume );
			break;
		case s3mFxArpeggio:
			if ( !currentSample() )
				break;
			useLastFxData( m_lastFx, fxVal );
			switch ( getTick() % 3 ) {
				case 0: // normal note
					setBasePeriod(st3Period(m_note, currentSample()->getBaseFrq()));
					break;
				case 1: // +x half notes...
					setBasePeriod(st3Period( deltaNote(m_note, highNibble(m_lastFx)), currentSample()->getBaseFrq() ));
					break;
				case 2: // +y half notes...
					setBasePeriod(st3Period( deltaNote(m_note, lowNibble(m_lastFx)), currentSample()->getBaseFrq() ));
					break;
			}
			break;
		case s3mFxPanSlide:
			useLastFxData( m_lastFx, fxVal );
			if ( highNibble( fxVal ) == 0x00 ) { // panning slide left
				if ( getTick() != 0 ) {
					fxVal = lowNibble( fxVal );
					tempVar = getPanning();
					if ( tempVar != 0xa4 )
						tempVar -= ( fxVal << 2 );
					setPanning( std::max<int16_t>( tempVar, 0 ) );
				}
			}
			else if ( lowNibble( fxVal ) == 0x00 ) { // panning slide right
				if ( getTick() != 0 ) {
					fxVal = highNibble( fxVal );
					tempVar = getPanning();
					if ( tempVar != 0xa4 )
						tempVar += ( fxVal << 2 );
					setPanning( std::min<int16_t>( tempVar, 0x80 ) );
				}
			}
			else if ( highNibble( fxVal ) == 0x0f ) { // fine panning slide left
				if ( getTick() == 0 ) {
					fxVal = lowNibble( fxVal );
					if ( fxVal == 0x00 )
						fxVal = 0x0f;
					tempVar = getPanning();
					if ( tempVar != 0xa4 )
						tempVar -= fxVal;
					setPanning( std::max<int16_t>( tempVar, 0 ) );
				}
			}
			else if ( lowNibble( fxVal ) == 0x0f ) { // fine panning slide right
				if ( getTick() == 0 ) {
					fxVal = highNibble( fxVal );
					if ( fxVal == 0x00 )
						fxVal = 0x0f;
					tempVar = getPanning();
					if ( tempVar != 0xa4 )
						tempVar += fxVal;
					setPanning( std::min<int16_t>( tempVar, 0x80 ) );
				}
			}
			break;
		case s3mFxSpecial:
			useLastFxData( m_lastFx, fxVal );
			switch ( highNibble( fxVal ) ) {
				case s3mSFxNoteDelay:
				case s3mSFxPatLoop:
				case s3mSFxPatDelay:
					// not handled here, because they're global (or not...)
					break;
				case s3mSFxSetPan:
					setPanning( lowNibble( fxVal )*0x80 / 0x0f );
					break;
				case s3mSFxStereoCtrl:
					if ( lowNibble( fxVal ) <= 7 )
						setPanning(( lowNibble( fxVal ) + 8 )*0x80 / 0x0f );
					else
						setPanning(( lowNibble( fxVal ) - 8 )*0x80 / 0x0f );
					break;
				case s3mSFxNoteCut:
					if (( lowNibble( fxVal ) > 0 ) || m_maybeSchism ) {
						if ( getTick() - 1 == lowNibble( fxVal ) )
							setActive( false );
					}
					break;
				case s3mSFxSetVibWave:
					switch ( fxVal&3 ) {
						case 0: vibrato().resetWave( S3mWaveSine, 256 ); break;
						case 1: vibrato().resetWave( S3mWaveRamp, 256 ); break;
						case 2: vibrato().resetWave( S3mWaveSquare, 256 ); break;
						case 3: vibrato().resetWave( ProtrackerLookup, 256, 256 ); break;
						default:
							break;
					}
					break;
				case s3mSFxSetTremWave:
					switch ( fxVal&3 ) {
						case 0: tremolo().resetWave( S3mWaveSine, 256 ); break;
						case 1: tremolo().resetWave( S3mWaveRamp, 256 ); break;
						case 2: tremolo().resetWave( S3mWaveSquare, 256 ); break;
						case 3: tremolo().resetWave( ProtrackerLookup, 256, 256 ); break;
						default:
							break;
					}
					break;
				case s3mSFxFunkRpt:
					LOG_MESSAGE_( "Funk Repeat not supported" );
					break;
				case s3mSFxSetFilter:
					LOG_MESSAGE_( "Set Filter not supported" );
					break;
				case s3mSFxSetFinetune:
					LOG_WARNING_( "Set Finetune (currently) not implemented" );
					break;
				case s3mSFxSetGlissando:
					m_glissando = fxVal!=0;
					LOG_WARNING_( "Set Glissando Control is experimental" );
					break;
				default:
					LOG_WARNING( "UNSUPPORTED SPECIAL FX FOUND: %s", getFxName().c_str() );
					break;
			}
			break;
		case s3mFxSetPanning:
			if (( fxVal <= 0x80 ) || ( fxVal == 0xa4 ) )
				setPanning( fxVal );
			else
				LOG_WARNING( "Panning value out of range: 0x%.2x", fxVal );
	}
}

void S3mChannel::pitchUp( const int16_t delta ) throw() {
	setBasePeriod(m_basePeriod - delta);
}

void S3mChannel::pitchDown( const int16_t delta ) throw() {
	setBasePeriod(m_basePeriod + delta);
}

void S3mChannel::pitchUp( uint16_t &per, const int16_t delta ) throw() {
	per -= delta;
}

void S3mChannel::pitchDown( uint16_t &per, const int16_t delta ) throw() {
	per += delta;
}

void S3mChannel::mixTick( MixerFrameBuffer &mixBuffer, const uint8_t volume ) throw( PppException ) {
	if ( isDisabled() )
		return;
	setGlobalVolume( volume, m_immediateGlobalVol );
	if (( !isActive() ) || ( !currentSample() ) || ( m_basePeriod == 0 ) ) {
		setActive( false );
		return;
	}
	if (( getTick() == 0 ) && ( m_zeroVolCounter != -1 ) && ( currentSample() ) && isActive() ) {
		if (( currentSample()->isLooped() ) && ( getVolume() == 0 ) )
			m_zeroVolCounter++;
		else
			m_zeroVolCounter = 0;
	}
	if ( m_zeroVolCounter == 3 ) {
		m_zeroVolCounter = 0;
		setActive( false );
		m_note = s3mEmptyNote;
		return;
	}
	uint16_t adjPer = getAdjustedPeriod();
	if(!isActive())
		return;
	LOG_TEST_ERROR(getPlaybackFrq() == 0);
	LOG_TEST_ERROR(mixBuffer->size() == 0);
	if( getPlaybackFrq() * mixBuffer->size() == 0 ) {
		setActive(false);
		return;
	}
	BresenInterpolation bres( mixBuffer->size(), FRQ_VALUE/getPlaybackFrq() * mixBuffer->size() / adjPer );
	uint16_t currVol = clip( getVolume() + m_deltaVolume, 0, 0x40 ) * m_globalVol;
	MixerSample *mixBufferPtr = &mixBuffer->front().left;
	S3mSample::Ptr currSmp = currentSample();
	int32_t pos = getPosition();
	for ( std::size_t i = 0; i < mixBuffer->size(); i++ ) {
		*( mixBufferPtr++ ) += ( currSmp->getLeftSampleAt( pos, getPanning() ) * currVol ) >> 12;
		*( mixBufferPtr++ ) += ( currSmp->getRightSampleAt( pos, getPanning() ) * currVol ) >> 12;
		if ( pos == GenSample::EndOfSample )
			break;
		bres.next( pos );
	}
	if ( pos != GenSample::EndOfSample )
		currentSample()->adjustPos( pos );
	setPosition(pos);
	if ( pos == GenSample::EndOfSample )
		setActive( false );
}

void S3mChannel::simTick( const std::size_t bufSize, const uint8_t volume ) {
	if ( isDisabled() )
		return;
	setGlobalVolume( volume, m_immediateGlobalVol );
	if (( !isActive() ) || ( !currentSample() ) || ( m_basePeriod == 0 ) )
		return setActive( false );
	if (( getTick() == 0 ) && ( m_zeroVolCounter != -1 ) && ( currentSample() ) && isActive() ) {
		if (( currentSample()->isLooped() ) && ( getVolume() == 0 ) )
			m_zeroVolCounter++;
		else
			m_zeroVolCounter = 0;
	}
	if ( m_zeroVolCounter == 3 ) {
		m_zeroVolCounter = 0;
		setActive( false );
		m_note = s3mEmptyNote;
		return;
	}
	PPP_TEST( getPlaybackFrq()==0 );
	PPP_TEST( bufSize==0);
	if( m_basePeriod==0 ) {
		setActive(false);
		setPosition(0);
		return;
	}
	uint16_t adj = getAdjustedPeriod();
	if(!isActive())
		return;
	int32_t pos = getPosition() + ( FRQ_VALUE / getPlaybackFrq() * bufSize / adj );
	currentSample()->adjustPos( pos );
	if ( pos == GenSample::EndOfSample )
		setActive( false );
	setPosition( pos );
}

std::string S3mChannel::getCellString() {
	if(!m_currentCell)
		return std::string();
	return m_currentCell->trackerString();
}

std::string S3mChannel::getFxDesc() const throw( PppException ) {
	PPP_TEST( !m_currentCell );
	if ( !m_currentCell->isActive() )
		return "      ";
	if ( m_currentCell->getEffect() == s3mEmptyCommand )
		return "      ";
	switch ( m_currentCell->getEffect() ) {
		case s3mFxSpeed:
			return "Speed\x7f";
		case s3mFxJumpOrder:
			return "JmOrd\x1a";
		case s3mFxBreakPat:
			return "PBrk \xf6";
		case s3mFxVolSlide:
			if(highNibble(m_lastFx)==0x0f) {
				if(lowNibble(m_lastFx)==0)
					return "VSld \x1e";
				else
					return "VSld \x19";
			}
			else if(lowNibble(m_lastFx)==0x0f) {
				if(highNibble(m_lastFx)==0)
					return "VSld \x1f";
				else
					return "VSld \x18";
			}
			else {
				if(highNibble(m_lastFx)==0)
					return "VSld \x1f";
				else
					return "VSld \x1e";
			}
		case s3mFxPitchDown:
			if ( highNibble( m_lastFx ) == 0x0f )
				return "Ptch \x1f";
			else if ( highNibble( m_lastFx ) == 0x0e )
				return "Ptch \x19";
			else
				return "Ptch\x1f\x1f";
		case s3mFxPitchUp:
			if ( highNibble( m_lastFx ) == 0x0f )
				return "Ptch \x1e";
			else if ( highNibble( m_lastFx ) == 0x0e )
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
			switch ( highNibble( m_currentCell->getEffectValue() ) ) {
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
	PPP_TEST( !m_sampleList );
	S3mSample::Ptr smp = currentSample();
	if ( !smp ) {
		setActive( false );
		setStatusString("         ...");
		return;
	}
	if ( isActive() ) {
		std::string panStr;
		if ( getPanning() == 0xa4 )
			panStr = "Srnd ";
		else if ( getPanning() == 0x00 )
			panStr = "Left ";
		else if ( getPanning() == 0x40 )
			panStr = "Centr";
		else if ( getPanning() == 0x80 )
			panStr = "Right";
		else
			panStr = stringf( "%4d%%", ( getPanning() - 0x40 ) * 100 / 0x40 );
		std::string volStr = stringf( "%3d%%", clip( getVolume() + m_deltaVolume, 0, 0x40 ) * 100 / 0x40 );
		setStatusString( stringf( "%.2x: %s%s %s %s P:%s V:%s %s",
		                         getCurrentSmpIdx(),
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

void S3mChannel::setGlobalVolume( const uint8_t gVol, const bool applyNow ) throw() {
	if ( getTick() == 0 )
		m_nextGlobalVol = gVol;
	if ( m_noteChanged || applyNow )
		m_globalVol = m_nextGlobalVol;
}

BinStream &S3mChannel::saveState( BinStream &str ) const throw( PppException ) {
	try {
		GenChannel::saveState( str )
		.write( &m_note )
		.write( &m_lastFx )
		.write( &m_lastPortaSpeed )
		.write( &m_lastVibratoData )
		.write( &m_tremorVolume )
		.write( &m_targetNote )
		.write( &m_globalVol )
		.write( &m_nextGlobalVol )
		.write( &m_retrigCount )
		.write( &m_tremorCount )
		.write( &m_noteChanged )
		.write( &m_300VolSlides )
		.write( &m_amigaLimits )
		.write( &m_deltaPeriod )
		.write( &m_deltaVolume )
		.write( &m_zeroVolCounter )
		.write( &m_basePeriod )
		.write( &m_glissando )
		.writeSerialisable( m_currentCell.get() );
	}
	PPP_CATCH_ALL();
	return str;
}

BinStream &S3mChannel::restoreState( BinStream &str ) throw( PppException ) {
	try {
		GenChannel::restoreState( str )
		.read( &m_note )
		.read( &m_lastFx )
		.read( &m_lastPortaSpeed )
		.read( &m_lastVibratoData )
		.read( &m_tremorVolume )
		.read( &m_targetNote )
		.read( &m_globalVol )
		.read( &m_nextGlobalVol )
		.read( &m_retrigCount )
		.read( &m_tremorCount )
		.read( &m_noteChanged )
		.read( &m_300VolSlides )
		.read( &m_amigaLimits )
		.read( &m_deltaPeriod )
		.read( &m_deltaVolume )
		.read( &m_zeroVolCounter )
		.read( &m_basePeriod )
		.read( &m_glissando )
		.readSerialisable( m_currentCell.get() );
	}
	PPP_CATCH_ALL();
	return str;
}
