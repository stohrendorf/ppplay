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
		/**
		 * @ingroup S3mMod
		 * @brief Note periodic table for frequency calculations
		 */
		static const uint16_t Periods[12] = {1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016,  960,  907};

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
		 * @brief The highest available note (B-9)
		 * @ingroup S3mMod
		 * @remarks Even if ST says that the maximum note is B-7 (and it
		 * accepts only octave 7), this value is more accurate. Seems to be
		 * used by other trackers.
		 * @remarks And some other trackers don't even care about B-9... If
		 * the frequency value is not a floating point value, the maximum
		 * frequency should be ::FRQ_VALUE there.
		 */
		static const uint8_t MAX_NOTE = 0x9b;

		/**
		 * @brief The lowest available note (C-0)
		 * @ingroup S3mMod
		 */
		static const uint8_t MIN_NOTE = 0x00;

		/**
		 * @brief The highest available Amiga note (B-5)
		 * @ingroup S3mMod
		 * @remarks In "Return 2the Dream:Nightbeat" this should be C-6.
		 */
		static const uint8_t MAX_NOTE_AMIGA = 0x5b;

		/**
		 * @brief The lowest available Amiga note (C-2)
		 * @ingroup S3mMod
		 */
		static const uint8_t MIN_NOTE_AMIGA = 0x20;

		/**
		 * @brief Calculate the period for a given note and base frequency
		 * @ingroup S3mMod
		 * @param[in] note Note value
		 * @param[in] c2spd Base frequency of the sample
		 * @return S3M Period for note @a note and base frequency @a c2spd
		 * @see ::S3mSample
		 * @note Time-critical
		 */
		static inline Frequency st3Period( const uint8_t note, const Frequency c2spd ) throw( PppException ) {
			PPP_TEST( c2spd == 0 );
			return static_cast<Frequency>( 8363 << 4 )*( Periods[S3M_NOTE( note )] >> S3M_OCTAVE( note ) ) / c2spd;
		}

		/**
		 * @brief Calculate the amiga period for a given note and base frequency
		 * @ingroup S3mMod
		 * @param[in] note Note value
		 * @param[in] c2spd Base frequency of the sample
		 * @return Amiga Period for note @a note and base frequency @a c2spd
		 * @see ::S3mSample
		 * @note Time-critical
		 */
		static inline Frequency st3AmigaPeriod( const uint8_t note, const Frequency c2spd ) throw( PppException ) {
			PPP_TEST( c2spd == 0 );
			return (( 8363 << 2 )*Periods[S3M_NOTE( note )] >> S3M_OCTAVE( note ) ) / c2spd;
		}

		/**
		 * @brief Calculate the frequency for a given note and base frequency
		 * @ingroup S3mMod
		 * @param[in] note Note value
		 * @param[in] c2spd Base frequency of the sample
		 * @return Frequency for note @a note and base frequency @a c2spd
		 * @see ::S3mSample
		 * @note Time-critical
		 */
		static inline Frequency st3Frequency( const uint8_t note, const Frequency c2spd ) throw() {
			if ( c2spd == 0 )
				return 0;
			return ( FRQ_VALUE / 16.0f ) / 8363.0f * ( static_cast<int>( c2spd ) << S3M_OCTAVE( note ) ) / Periods[S3M_NOTE( note )];
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
		static inline std::string frequencyToNote( const Frequency frq, const Frequency c2spd ) throw() {
			if ( frq == 0 )
				return "f??";
			if ( c2spd == 0 )
				return "c??";
			float nper = ( c2spd * FRQ_VALUE ) / ( frq * 16.0f * 8363.0f );
			// nper = Periods[note] / 2^oct
			// note = totalnote % 12; oct = totalnote / 12
			// -> nper = Periods[0] / 2^(totalnote/12)
			// -> 2^(totalnote/12) = Periods[0]/nper
			// -> totalnote = log2(Periods[0]/nper)*12
			float totalnote = log2( Periods[0] / nper ) * 12.0f;
			uint8_t minoct = totalnote / 12.0f;
			uint8_t minnote = std::fmod(totalnote, 12);
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

		/**
		 * @brief Apply a delta period to a frequency
		 * @param[in] frq Base frequency
		 * @param[in] delta Delta period
		 * @return New frequency
		 * @note Time-critical
		 */
		static inline Frequency deltaFrq( const Frequency frq, const int16_t delta ) throw() {
			if ( frq == 0 )
				return 0;
			double x = ( static_cast<double>( FRQ_VALUE ) / frq ) + delta;
			if ( x <= 1 )
				x = 1;
			return std::ceil( FRQ_VALUE / x );
		}
	} // namespace s3m
} // namespace ppp

using namespace ppp;
using namespace ppp::s3m;

S3mChannel::S3mChannel( const Frequency frq, const S3mSample::List::Ptr &smp ) throw() : GenChannel( frq ),
		m_note( ::s3mEmptyNote ), m_lastFx( 0 ), m_lastPortaFx( 0 ), m_lastVibratoFx( 0 ), m_lastVibVolFx( 0 ), m_lastPortVolFx( 0 ),
		m_tremorVolume( 0 ), m_targetNote( ::s3mEmptyNote ), m_noteChanged( false ), m_deltaFrq( 0 ),
		m_deltaVolume( 0 ), m_minFrequency( 0 ), m_maxFrequency( 0 ), m_globalVol( 0x40 ), m_nextGlobalVol( 0x40 ),
		m_retrigCount( -1 ), m_tremorCount( -1 ), m_300VolSlides( false ), m_amigaLimits( false ), m_immediateGlobalVol( false ),
		m_maybeSchism( false ), m_zeroVolCounter( -1 ), m_sampleList(smp) {
	setCurrentCell(GenCell::Ptr(new S3mCell()));
}

S3mChannel::~S3mChannel() throw() {
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
	return frequencyToNote( getAdjustedFrq(), m_sampleList->at(getCurrentSmpIdx())->getBaseFrq() );
}

std::string S3mChannel::getFxName() const throw() {
	S3mCell::Ptr s3mcell = std::static_pointer_cast<S3mCell>( getCurrentCell() );
	if ( !s3mcell )
		return "...";
	if ( !s3mcell->isActive() ) {
		return "...";
	}
	if ( s3mcell->getEffect() == s3mEmptyCommand ) {
		return "...";
	}
	uint8_t fx = s3mcell->getEffect();
	bool fxRangeOk = inRange<int>( fx, 1, 27 );
	if ( fxRangeOk )
		return stringf( "%c%.2x", fx + 'A' - 1, s3mcell->getEffectValue() );
	else {
		LOG_ERROR( "Effect out of range: 0x%.2x", fx );
		return stringf( "?%.2x", s3mcell->getEffectValue() );
	}
}

Frequency S3mChannel::getAdjustedFrq() throw() {
	Frequency r = deltaFrq( getBareFrq(), m_deltaFrq );
	r = clip<Frequency>( r, m_minFrequency, m_maxFrequency );
	if(r==0)
		setActive(false);
	return r;
}

void S3mChannel::update( GenCell::Ptr const cell, const uint8_t tick, bool noRetrigger ) throw() {
	LOG_BEGIN();
	if ( isDisabled() )
		return;
	setTick(tick);
	if ( !cell )
		getCurrentCell()->reset();
	else
		*std::static_pointer_cast<S3mCell>( getCurrentCell() ) = *std::static_pointer_cast<const S3mCell>( cell );
	S3mCell::Ptr s3mcell = std::static_pointer_cast<S3mCell>( getCurrentCell() );
	if ( !s3mcell )
		return setActive(false);
	updateStatus();
	if ( tick == 0 )
		m_noteChanged = false;
	if ( !s3mcell->isActive() )
		return;
	if ( s3mcell->getEffect() != s3mFxRetrig )
		m_retrigCount = -1;
	else
		m_retrigCount++;
	if ( s3mcell->getEffect() != s3mFxTremor )
		m_tremorCount = -1;
	else
		m_tremorCount++;
	char smpDelay = 0;
	if (( s3mcell->getEffect() == s3mFxSpecial ) && ( highNibble( s3mcell->getEffectValue() ) == s3mSFxNoteDelay ) ) {
		smpDelay = lowNibble( s3mcell->getEffectValue() );
		if ( smpDelay == 0 )
			return;
	}
	if (( s3mcell->getEffect() == s3mEmptyCommand ) && ( s3mcell->getEffectValue() != 0x00 ) )
		m_lastFx = s3mcell->getEffectValue();
	if (( s3mcell->getEffect() != s3mFxVibrato ) && ( s3mcell->getEffect() != s3mFxFineVibrato ) && ( s3mcell->getEffect() != s3mFxVibVolSlide ) ) {
		m_deltaFrq = 0;
		vibrato().resetPhase();
	}
	if ( s3mcell->getEffect() != s3mFxTremolo ) {
		m_deltaVolume = 0;
		tremolo().resetPhase();
	}
	m_noteChanged = ( s3mcell->getNote() != s3mEmptyNote ) && ( s3mcell->getNote() != s3mKeyOffNote ) && !noRetrigger;
	if (( tick == smpDelay ) && !noRetrigger ) {
		if ( s3mcell->getInstr() != s3mEmptyInstr ) {
			setSampleIndex( s3mcell->getInstr() - 1 );
			if ( !m_sampleList->at(getCurrentSmpIdx()) ) {
				setActive( false );
				return;
			}
			try {
				if ( !m_amigaLimits ) {
					m_minFrequency = st3Frequency( MIN_NOTE, m_sampleList->at(getCurrentSmpIdx())->getBaseFrq() );
					if ( m_minFrequency == 0 ) {
						m_maxFrequency = 0;
						setActive( false );
						return;
					}
					else
						m_maxFrequency = FRQ_VALUE;
				}
				else {
					m_minFrequency = st3Frequency( MIN_NOTE_AMIGA, m_sampleList->at(getCurrentSmpIdx())->getBaseFrq() );
					if ( m_minFrequency == 0 ) {
						m_maxFrequency = 0;
						setActive( false );
						return;
					}
					else
						m_maxFrequency = FRQ_VALUE >> 2;
				}
			}
			catch ( ... ) {
				m_minFrequency = 0;
				m_maxFrequency = 0;
				setActive( false );
				LOG_ERROR_( "Exception" );
				return;
			}
			setVolume( m_sampleList->at(getCurrentSmpIdx())->getVolume() );
		}
		if ( s3mcell->getVolume() != s3mEmptyVolume ) {
			setVolume( s3mcell->getVolume() );
			m_deltaVolume = 0;
		}
		if (( s3mcell->getNote() != s3mEmptyNote ) && ( s3mcell->getNote() != s3mKeyOffNote ) ) {
			if ( s3mcell->getEffect() != s3mFxPorta ) {
				m_note = s3mcell->getNote();
				if ( m_sampleList->at(getCurrentSmpIdx()) ) {
					setBareFrq( clip<Frequency>(st3Frequency( m_note, m_sampleList->at(getCurrentSmpIdx())->getBaseFrq() ), m_minFrequency, m_maxFrequency) );
				}
				setPosition( 0 );
			}
			m_deltaFrq = 0;
			vibrato().resetPhase();
		}
	}
	if ( s3mcell->getEffect() == s3mFxPorta ) {
		if (( s3mcell->getNote() != s3mEmptyNote ) && ( s3mcell->getNote() != s3mKeyOffNote ) )
			m_targetNote = s3mcell->getNote();
		if (( m_note == s3mEmptyNote ) && ( m_targetNote != s3mEmptyNote ) ) {
			m_note = m_targetNote;
			if ( m_sampleList->at(getCurrentSmpIdx()) ) {
				setBareFrq( clip<Frequency>(st3Frequency( m_note, m_sampleList->at(getCurrentSmpIdx())->getBaseFrq() ), m_minFrequency, m_maxFrequency) );
			}
		}
	}
	if ( getTick() == smpDelay ) {
		setActive( ( m_sampleList->at(getCurrentSmpIdx()) ) && ( m_note != s3mEmptyNote ) );
	}
	if ( !isActive() )
		return;
	if ( s3mcell->getEffect() != s3mEmptyCommand ) {
		switch ( s3mcell->getEffect() ) {
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
				doVolumeFx( s3mcell->getEffect(), s3mcell->getEffectValue() );
				break;
			case s3mFxPitchDown:
			case s3mFxPitchUp:
			case s3mFxPorta:
				doPitchFx( s3mcell->getEffect(), s3mcell->getEffectValue() );
				break;
			case s3mFxVibrato:
			case s3mFxFineVibrato:
				doVibratoFx( s3mcell->getEffect(), s3mcell->getEffectValue() );
				break;
			case s3mFxSpecial:
			case s3mFxOffset:
			case s3mFxRetrig:
			case s3mFxTremor:
			case s3mFxArpeggio:
			case s3mFxPanSlide:
			case s3mFxSetPanning:
				if ( !( noRetrigger && ( s3mcell->getEffect() == s3mFxOffset ) ) )
					doSpecialFx( s3mcell->getEffect(), s3mcell->getEffectValue() );
				break;
			case s3mFxVibVolSlide:
				doVibratoFx( s3mFxVibVolSlide, 0 );
				doVolumeFx( s3mFxVibVolSlide, s3mcell->getEffectValue() );
				break;
			case s3mFxPortVolSlide:
				doPitchFx( s3mFxPortVolSlide, 0 );
				doVolumeFx( s3mFxPortVolSlide, s3mcell->getEffectValue() );
				break;
			default:
				LOG_WARNING( "UNSUPPORTED FX FOUND: %s", getFxName().c_str() );
				break;
		}
	}
	setActive( isActive() && ( s3mcell->getNote() != s3mKeyOffNote ) );
}

void S3mChannel::doVolumeFx( const uint8_t fx, uint8_t fxVal ) throw() {
	int16_t tempVar;
	switch ( fx ) {
		case s3mFxVolSlide:
		case s3mFxVibVolSlide:
		case s3mFxPortVolSlide:
			if ( fx == s3mFxVibVolSlide ) {
				useLastFxData( m_lastVibVolFx, fxVal );
				if (( highNibble( fxVal ) == 0x0f ) || ( lowNibble( fxVal ) == 0x0f ) )
					break;
			}
			else if ( fx == s3mFxPortVolSlide ) {
				useLastFxData( m_lastPortVolFx, fxVal );
				if (( highNibble( fxVal ) == 0x0f ) || ( lowNibble( fxVal ) == 0x0f ) )
					break;
			}
			else
				useLastFxData( m_lastFx, fxVal );
			tempVar = getVolume();
			if ( highNibble( fxVal ) == 0x00 ) { // slide down
				if (( getTick() != 0 ) || m_300VolSlides )
					tempVar -= lowNibble( fxVal );
			}
			else if ( lowNibble( fxVal ) == 0x00 ) { // slide up
				if (( getTick() != 0 ) || m_300VolSlides )
					tempVar += highNibble( fxVal );
			}
			else if ( highNibble( fxVal ) == 0x0f ) { // fine slide down
				if ( getTick() == 0 )
					tempVar -= lowNibble( fxVal );
			}
			else if ( lowNibble( fxVal ) == 0x0f ) { // fine slide up
				if ( getTick() == 0 )
					tempVar += highNibble( fxVal );
			}
			else { // slide down
				if (( getTick() != 0 ) || m_300VolSlides )
					tempVar -= lowNibble( fxVal );
			}
			setVolume( clip<int16_t>( tempVar, 0, 0x40 ) );
			break;
		case s3mFxTremolo:
			combineLastFxData( m_lastFx, fxVal );
			if ( getTick() != 0 ) {
				tremolo() += highNibble( fxVal ) << 2;
				m_deltaVolume = ( tremolo().get() * lowNibble( fxVal ) >> 7 );
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
			if ( fx == s3mFxVibVolSlide ) {
				if (( highNibble( m_lastVibVolFx ) == 0x0f ) || ( lowNibble( m_lastVibVolFx ) == 0x0f ) )
					break;
			}
			else {
				combineLastFxData( m_lastVibratoFx, fxVal );
				m_lastVibVolFx = m_lastVibratoFx;
			}
			fxVal = m_lastVibratoFx;
			if ( getTick() != 0 ) {
				vibrato() += highNibble( fxVal ) << 2;
				m_deltaFrq = vibrato().get() * lowNibble( fxVal ) >> 5;
			}
			break;
		case s3mFxFineVibrato:
			combineLastFxData( m_lastVibratoFx, fxVal );
			if ( getTick() != 0 ) {
				vibrato() += highNibble( fxVal ) << 2;
				m_deltaFrq = vibrato().get() * lowNibble( fxVal ) >> 7;
			}
			break;
	}
}

void S3mChannel::doPitchFx( const uint8_t fx, uint8_t fxVal ) throw() {
	switch ( fx ) {
		case s3mFxPitchDown:
			useLastFxData( m_lastFx, fxVal );
			if ( highNibble( fxVal ) == 0x0f ) { // fine slide down
				if ( getTick() == 0 )
					pitchDown( lowNibble( fxVal ) << 2 );
			}
			else if ( highNibble( fxVal ) == 0x0e ) { // extra fine slide down
				if ( getTick() == 0 )
					pitchDown( lowNibble( fxVal ) );
			}
			else if ( getTick() != 0 ) // slide down
				pitchDown( fxVal << 2 );
			break;
		case s3mFxPitchUp:
			useLastFxData( m_lastFx, fxVal );
			if ( highNibble( fxVal ) == 0x0f ) { // fine slide up
				if ( getTick() == 0 )
					pitchUp( lowNibble( fxVal ) << 2 );
			}
			else if ( highNibble( fxVal ) == 0x0e ) { // extra fine slide up
				if ( getTick() == 0 )
					pitchUp( lowNibble( fxVal ) );
			}
			else if ( getTick() != 0 ) // slide up
				pitchUp( fxVal << 2 );
			break;
		case s3mFxPorta:
		case s3mFxPortVolSlide:
			if ( fx == s3mFxPortVolSlide ) {
				if (( highNibble( m_lastPortVolFx ) == 0x0f ) || ( lowNibble( m_lastPortVolFx ) == 0x0f ) )
					break;
			}
			else {
				useLastFxData( m_lastPortaFx, fxVal );
				m_lastPortVolFx = m_lastPortaFx;
			}
			fxVal = m_lastPortaFx;
			if ( m_sampleList->at(getCurrentSmpIdx()) && ( getTick() != 0 ) ) {
				Frequency baseFrq = 0;
				Frequency targetFrq = 0;
				targetFrq = st3Frequency( m_targetNote, m_sampleList->at(getCurrentSmpIdx())->getBaseFrq() ); // calculate target frq
				targetFrq = clip<Frequency>( targetFrq, m_minFrequency, m_maxFrequency );
				if ( getBareFrq() == 0 )
					setBareFrq( targetFrq );
				else {
					if ( targetFrq < getBareFrq() ) { // pitch down...
						baseFrq = getBareFrq();
						pitchDown( baseFrq, fxVal << 2 );
						if ( baseFrq <= targetFrq ) {
							setBareFrq( targetFrq );
							m_note = m_targetNote;
						}
						else
							setBareFrq( baseFrq );
					}
					else { // pitch up...
						baseFrq = getBareFrq();
						pitchUp( baseFrq, fxVal << 2 );
						if ( baseFrq >= targetFrq ) {
							setBareFrq( targetFrq );
							m_note = m_targetNote;
						}
						else
							setBareFrq( baseFrq );
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
	S3mCell::Ptr s3mcell = std::static_pointer_cast<S3mCell>( getCurrentCell() );
	PPP_TEST( !s3mcell );
	switch ( fx ) {
		case s3mFxOffset:
			useLastFxData( m_lastFx, fxVal );
			if (( getTick() == 0 ) && ( m_sampleList->at(getCurrentSmpIdx()) ) && ( s3mcell->getNote() != s3mEmptyNote ) ) {
				setPosition( fxVal << 8 );
				int32_t pos = getPosition();
				m_sampleList->at(getCurrentSmpIdx())->adjustPos( pos );
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
			if ( !m_sampleList->at(getCurrentSmpIdx()) )
				break;
			useLastFxData( m_lastFx, fxVal );
			switch ( getTick() % 3 ) {
				case 0: // normal note
					setBareFrq( st3Frequency( m_note, m_sampleList->at(getCurrentSmpIdx())->getBaseFrq() ) );
					break;
				case 1: // +x half notes...
					setBareFrq( st3Frequency( deltaNote( m_note, highNibble( m_lastFx ) ), m_sampleList->at(getCurrentSmpIdx())->getBaseFrq() ) );
					break;
				case 2: // +y half notes...
					setBareFrq( st3Frequency( deltaNote( m_note, lowNibble( m_lastFx ) ), m_sampleList->at(getCurrentSmpIdx())->getBaseFrq() ) );
					break;
			}
			setBareFrq( clip<Frequency>( getBareFrq(), m_minFrequency, m_maxFrequency ) );
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
						case 0:
							vibrato().resetWave( ProtrackerLookup, 256, 256 );
						case 1:
							vibrato().resetWave( RampLookup, 256, 64 );
						case 2:
							vibrato().resetWave( SquareLookup, 256, 64 );
						case 3:
							vibrato().resetWave( ProtrackerLookup, 256, 256 );
						default:
							break;
					}
					break;
				case s3mSFxSetTremWave:
					switch ( fxVal&3 ) {
						case 0:
							tremolo().resetWave( ProtrackerLookup, 256, 256 );
						case 1:
							tremolo().resetWave( RampLookup, 256, 64 );
						case 2:
							tremolo().resetWave( SquareLookup, 256, 64 );
						case 3:
							tremolo().resetWave( ProtrackerLookup, 256, 256 );
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
					LOG_MESSAGE_( "Set Glissando Control not supported" );
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
	Frequency f = getBareFrq();
	pitchUp( f, delta );
	setBareFrq(f);
}

void S3mChannel::pitchDown( const int16_t delta ) throw() {
	Frequency f = getBareFrq();
	pitchDown( f, delta );
	setBareFrq(f);
}

void S3mChannel::pitchUp( Frequency& frq, const int16_t delta ) throw() {
	frq = deltaFrq( frq, -delta );
	frq = clip<Frequency>( frq, m_minFrequency, m_maxFrequency );
}

void S3mChannel::pitchDown( Frequency& frq, const int16_t delta ) throw() {
	frq = deltaFrq( frq, delta );
	frq = clip<Frequency>( frq, m_minFrequency, m_maxFrequency );
}

void S3mChannel::mixTick( MixerFrameBuffer &mixBuffer, const uint8_t volume ) throw( PppException ) {
	if ( isDisabled() )
		return;
	setGlobalVolume( volume, m_immediateGlobalVol );
	if (( !isActive() ) || ( !m_sampleList->at(getCurrentSmpIdx()) ) || ( getBareFrq() == 0 ) ) {
		setActive( false );
		return;
	}
	if (( getTick() == 0 ) && ( m_zeroVolCounter != -1 ) && ( m_sampleList->at(getCurrentSmpIdx()) ) && isActive() ) {
		if (( m_sampleList->at(getCurrentSmpIdx())->isLooped() ) && ( getVolume() == 0 ) )
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
	Frequency adjFrq = getAdjustedFrq();
	if(!isActive())
		return;
	LOG_TEST_ERROR(getPlaybackFrq() == 0);
	LOG_TEST_ERROR(mixBuffer->size() == 0);
	if( getPlaybackFrq() * mixBuffer->size() == 0 ) {
		setActive(false);
		return;
	}
	BresenInterpolation bres( mixBuffer->size(), FRQ_VALUE/getPlaybackFrq() * mixBuffer->size()*adjFrq / FRQ_VALUE );
	uint16_t currVol = clip( getVolume() + m_deltaVolume, 0, 0x40 ) * m_globalVol;
	MixerSample *mixBufferPtr = &mixBuffer->front().left;
	S3mSample::Ptr currSmp = m_sampleList->at(getCurrentSmpIdx());
	int32_t pos = getPosition();
	for ( std::size_t i = 0; i < mixBuffer->size(); i++ ) {
		*( mixBufferPtr++ ) += ( currSmp->getLeftSampleAt( pos, getPanning() ) * currVol ) >> 12;
		*( mixBufferPtr++ ) += ( currSmp->getRightSampleAt( pos, getPanning() ) * currVol ) >> 12;
		if ( pos == GenSample::EndOfSample )
			break;
		bres.next( pos );
	}
	if ( pos != GenSample::EndOfSample )
		m_sampleList->at(getCurrentSmpIdx())->adjustPos( pos );
	setPosition(pos);
	if ( pos == GenSample::EndOfSample )
		setActive( false );
}

void S3mChannel::simTick( const std::size_t bufSize, const uint8_t volume ) {
	if ( isDisabled() )
		return;
	setGlobalVolume( volume, m_immediateGlobalVol );
	if (( !isActive() ) || ( !m_sampleList->at(getCurrentSmpIdx()) ) || ( getBareFrq() == 0 ) )
		return setActive( false );
	if (( getTick() == 0 ) && ( m_zeroVolCounter != -1 ) && ( m_sampleList->at(getCurrentSmpIdx()) ) && isActive() ) {
		if (( m_sampleList->at(getCurrentSmpIdx())->isLooped() ) && ( getVolume() == 0 ) )
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
	if( getAdjustedFrq()==0 ) {
		setActive(false);
		setPosition(0);
		return;
	}
	int32_t pos = getPosition() + ( FRQ_VALUE / getPlaybackFrq() * bufSize * getAdjustedFrq() / FRQ_VALUE );
	m_sampleList->at(getCurrentSmpIdx())->adjustPos( pos );
	if ( pos == GenSample::EndOfSample )
		setActive( false );
	setPosition( pos );
}

std::string S3mChannel::getFxDesc() const throw( PppException ) {
	S3mCell::Ptr s3mcell = std::static_pointer_cast<S3mCell>( getCurrentCell() );
	PPP_TEST( !s3mcell );
	if ( !s3mcell->isActive() )
		return "      ";
	if ( s3mcell->getEffect() == s3mEmptyCommand )
		return "      ";
	switch ( s3mcell->getEffect() ) {
		case s3mFxSpeed:
			return "Speed\x7f";
		case s3mFxJumpOrder:
			return "JmOrd\x1a";
		case s3mFxBreakPat:
			return "PBrk \xf6";
		case s3mFxVolSlide:
			if ( highNibble( m_lastFx ) == 0x00 )  // slide down
				return "VSld \x1f";
			else if ( lowNibble( m_lastFx ) == 0x00 ) // slide up
				return "VSld \x1e";
			else if ( highNibble( m_lastFx ) == 0x0f ) // fine slide down
				return "VSld \x19";
			else if ( lowNibble( m_lastFx ) == 0x0f ) // fine slide up
				return "VSld \x18";
			else // slide down
				return "VSld \x1f";
		case s3mFxPitchDown:
			if ( highNibble( m_lastFx ) == 0x0f )
				return "Ptch \x1f";
			else if ( highNibble( m_lastFx ) == 0x0e )
				return "Ptch \x19";
			else
				return "Ptch\x1f\x1f";
		case s3mFxPitchUp:
			if ( lowNibble( m_lastFx ) == 0x0f )
				return "Ptch \x1e";
			else if ( lowNibble( m_lastFx ) == 0x0e )
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
			switch ( highNibble( s3mcell->getEffectValue() ) ) {
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
	S3mSample::Ptr smp = m_sampleList->at(getCurrentSmpIdx());
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
		                         ( std::static_pointer_cast<S3mCell>( getCurrentCell() )->getNote() == s3mKeyOffNote ? "^^ " : "   " ),
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
		.write( &m_lastPortaFx )
		.write( &m_lastVibratoFx )
		.write( &m_lastVibVolFx )
		.write( &m_lastPortVolFx )
		.write( &m_tremorVolume )
		.write( &m_targetNote )
		.write( &m_globalVol )
		.write( &m_nextGlobalVol )
		.write( &m_retrigCount )
		.write( &m_tremorCount )
		.write( &m_noteChanged )
		.write( &m_300VolSlides )
		.write( &m_amigaLimits )
		.write( &m_deltaFrq )
		.write( &m_deltaVolume )
		.write( &m_zeroVolCounter )
		.write( &m_minFrequency )
		.write( &m_maxFrequency );
	}
	PPP_CATCH_ALL();
	return str;
}

BinStream &S3mChannel::restoreState( BinStream &str ) throw( PppException ) {
	try {
		GenChannel::restoreState( str )
		.read( &m_note )
		.read( &m_lastFx )
		.read( &m_lastPortaFx )
		.read( &m_lastVibratoFx )
		.read( &m_lastVibVolFx )
		.read( &m_lastPortVolFx )
		.read( &m_tremorVolume )
		.read( &m_targetNote )
		.read( &m_globalVol )
		.read( &m_nextGlobalVol )
		.read( &m_retrigCount )
		.read( &m_tremorCount )
		.read( &m_noteChanged )
		.read( &m_300VolSlides )
		.read( &m_amigaLimits )
		.read( &m_deltaFrq )
		.read( &m_deltaVolume )
		.read( &m_zeroVolCounter )
		.read( &m_minFrequency )
		.read( &m_maxFrequency );
	}
	PPP_CATCH_ALL();
	return str;
}
