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
#include "stmchannel.h"
#include "stmpattern.h"
#include "breseninter.h"
//#include <cmath>

/**
 * @file
 * @ingroup StmMod
 * @brief STM Channel Definitions, implementation
 */

namespace ppp {
	namespace stm {
		/**
		 * @brief Get the octave out of a note
		 * @ingroup StmMod
		 * @param[in] x Note value
		 * @return Octave of @a x
		 */
		inline unsigned char STM_OCTAVE( const unsigned char x ) {
			return highNibble( x );
		}

		/**
		 * @brief Get the note out of a note
		 * @ingroup StmMod
		 * @param[in] x Note value
		 * @return Note of @a x
		 */
		inline unsigned char STM_NOTE( const unsigned char x ) {
			return lowNibble( x );
		}

		/**
		 * @brief A value for frequency calculation
		 * @ingroup StmMod
		 */
		const unsigned int FRQ_VALUE = 14317056 >> 2;

		/**
		 * @brief The lowest available note (C-0)
		 * @ingroup StmMod
		 */
		const unsigned char MIN_NOTE = 0x00;

		/**
		 * @brief Calculate the frequency for a given note and base frequency
		 * @ingroup StmMod
		 * @param[in] note Note value
		 * @param[in] c3spd Base frequency of the sample
		 * @return Frequency for note @a note and base frequency @a c2spd
		 * @see ::StmSample
		 */
		inline Frequency stmFrequency( const unsigned char note, const Frequency c3spd ) throw( PppException ) {
			PPP_TEST( c3spd == 0 );
			return FRQ_VALUE*c3spd / Periods[STM_OCTAVE( note )][STM_NOTE( note )] / 8363;
		}

		/**
		 * @brief Reverse-calculate the Note from the given frequency
		 * @ingroup StmMod
		 * @param[in] frq Playback frequency
		 * @param[in] c2spd Base frequency of the sample
		 * @return Note string
		 */
		inline std::string frequencyToNote( const Frequency frq, const Frequency c2spd ) throw( PppException ) {
			PPP_TEST( frq == 0 || c2spd == 0 );
			//PPP_TEST( c2spd == 0 );
			return "???";
		}

		/**
		 * @brief Add/subtract semitones to/from a note
		 * @ingroup StmMod
		 * @param[in] note Base note
		 * @param[in] delta Delta value
		 * @return New note
		 */
		inline unsigned char deltaNote( const unsigned char note, const signed char delta ) throw() {
			unsigned short x = STM_OCTAVE( note ) * 12 + STM_NOTE( note ) + delta;
			return (( x / 12 ) << 4 ) | ( x % 12 );
		}

		/**
		 * @brief Apply a delta period to a frequency
		 * @param[in] frq Base frequency
		 * @param[in] delta Delta period
		 * @return New frequency
		 */
		inline Frequency deltaFrq( const Frequency frq, const short delta ) throw( PppException ) {
			PPP_TEST( frq == 0 );
			double x = ( static_cast<double>( FRQ_VALUE ) / frq ) + delta;
			if ( x <= 1 )
				x = 1;
			return ( FRQ_VALUE / x );
		}
	} // namespace s3m
} // namespace ppp

using namespace ppp;
using namespace ppp::stm;

StmChannel::StmChannel( Frequency frq, const GenSampleList::Ptr& smp ) throw() : GenChannel( frq, smp ),
		aNote( stmEmptyNote ), aLastFx( 0 ), aLastPitchFx( 0 ), aLastVibratoFx( 0 ),
		aVibratoPhase( 0 ), aTremoloPhase( 0 ), aTargetNote( stmEmptyNote ), aNoteChanged( false ), aDeltaFrq( 0 ),
		aDeltaVolume( 0 ), aMinFrequency( 0 ), aMaxFrequency( 0 ), aGlobalVol( 0x40 ), aNextGlobalVol( 0x40 ),
		aRetrigCount( -1 ) {
	m_currCell.reset( new StmCell() );
}

StmChannel::~StmChannel() throw() {
}

void StmChannel::useLastFxData( unsigned char &oldFx, unsigned char &newFx ) const throw() {
	if ( newFx == 0 )
		newFx = oldFx;
	else
		oldFx = newFx;
	newFx = oldFx;
}

void StmChannel::combineLastFxData( unsigned char &oldFx, unsigned char &newFx ) const throw() {
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

std::string StmChannel::getNoteName() throw( PppException ) {
	if ( aNote == stmEmptyNote )
		return "   ";
	if (( !m_active ) || m_disabled )
		return "   ";
	return stringf( "%s%d", NoteNames[STM_NOTE( aNote )], STM_OCTAVE( aNote ) );
// 	return frequencyToNote(deltaFrq(aFrequency, aDeltaFrq), aCurrSmp->getBaseFrq());
}

std::string StmChannel::getFxName() const throw( PppException ) {
	StmCell::Ptr cell = std::static_pointer_cast<StmCell>( m_currCell );
	if ( !cell->isActive() )
		return "...";
	if ( cell->getEffect() == stmEmptyEffect )
		return "...";
	return stringf( "%c%.2x", cell->getEffect() + 'A' - 1, cell->getEffectValue() );
}

std::size_t StmChannel::getPosition() const throw() {
	return m_position;
}

Frequency StmChannel::getFrq() const throw() {
	Frequency r = deltaFrq( m_frequency, aDeltaFrq );
	return clip<Frequency>( r, aMinFrequency, aMaxFrequency );
}

void StmChannel::update( GenCell::Ptr const cell, const unsigned char tick, bool noRetrigger ) throw( PppException ) {
	LOG_BEGIN();
	if ( m_disabled )
		return;
	m_tick = tick;
	if ( !cell ) {
		m_currCell->reset();
	}
	else {
		*m_currCell = *std::static_pointer_cast<const StmCell>( cell );
	}
	StmCell::Ptr stmcell = std::static_pointer_cast<StmCell>( m_currCell );
	updateStatus();
	if ( tick == 0 )
		aNoteChanged = false;
	if ( !stmcell->isActive() )
		return;
	char smpDelay = 0;
	aNoteChanged = ( stmcell->getNote() != stmEmptyNote ) && !noRetrigger;
	if ( m_tick == 0 ) {
		aTremoloPhase = aVibratoPhase = 0;
	}
	if (( tick == smpDelay ) && !noRetrigger ) {
		if ( stmcell->getInstr() != stmEmptyInstr ) {
			updateSamplePtr( stmcell->getInstr() - 1 );
			if ( !getCurrentSample() ) {
				m_active = false;
				return;
			}
			try {
				aMinFrequency = stmFrequency( MIN_NOTE, getCurrentSample()->getBaseFrq() );
				aMaxFrequency = FRQ_VALUE;
			}
			catch ( ... ) {
				aMinFrequency = 0;
				aMaxFrequency = 0;
				m_active = false;
				return;
			}
			m_volume = getCurrentSample()->getVolume();
			aDeltaVolume = 0;
			aDeltaFrq = 0;
		}
		if ( stmcell->getVolume() != stmEmptyVolume ) {
			m_volume = stmcell->getVolume();
			aDeltaVolume = 0;
		}
		/*		if (aNoteChanged) {
					aVibratoPhase = 0;
				}*/
		if ( stmcell->getNote() != stmEmptyNote ) {
			if ( stmcell->getEffect() != stmFxPorta ) {
				aNote = stmcell->getNote();
				if ( getCurrentSample() ) {
					m_frequency = stmFrequency( aNote, getCurrentSample()->getBaseFrq() );
					m_frequency = clip( m_frequency, aMinFrequency, aMaxFrequency );
				}
				m_position = 0;
			}
			aDeltaFrq = 0;
		}
	}
	if ( stmcell->getEffect() == stmFxPorta ) {
		if ( stmcell->getNote() != stmEmptyNote )
			aTargetNote = stmcell->getNote();
		if (( aNote == stmEmptyNote ) && ( aTargetNote != stmEmptyNote ) ) {
			aNote = aTargetNote;
			if ( getCurrentSample() ) {
				m_frequency = stmFrequency( aNote, getCurrentSample()->getBaseFrq() );
				m_frequency = clip( m_frequency, aMinFrequency, aMaxFrequency );
			}
		}
	}
	if ( m_tick == smpDelay ) {
		m_active = ( getCurrentSample() ) && ( aNote != stmEmptyNote );
	}
	if ( !m_active )
		return;
	if ( stmcell->getEffect() != stmEmptyEffect ) {
		switch ( stmcell->getEffect() ) {
			case stmFxSetTempo:
			case stmFxPatJump:
			case stmFxPatBreak:
				// already updated effects...
				break;
			case stmFxVolSlide:
			case stmFxTremolo:
				doVolumeFx( stmcell->getEffect(), stmcell->getEffectValue() );
				break;
			case stmFxSlideDown:
			case stmFxSlideUp:
			case stmFxPorta:
				doPitchFx( stmcell->getEffect(), stmcell->getEffectValue(), false );
				break;
			case stmFxVibrato:
				doVibratoFx( stmcell->getEffect(), stmcell->getEffectValue(), false );
				break;
			case stmFxArpeggio:
				doSpecialFx( stmcell->getEffect(), stmcell->getEffectValue() );
				break;
			case stmFxVibVolSlide:
				doVibratoFx( stmFxVibrato, 0, true );
				doVolumeFx( stmFxVolSlide, stmcell->getEffectValue() );
				break;
			case stmFxPortVolSlide:
				doPitchFx( stmFxPorta, 0, true );
				doVolumeFx( stmFxVolSlide, stmcell->getEffectValue() );
				break;
			default:
				LOG_ERROR( "UNSUPPORTED FX FOUND: " + getFxName() );
				break;
		}
	}
	m_active = m_active && ( stmcell->getNote() != stmKeyOffNote );
}

void StmChannel::doVolumeFx( const unsigned char fx, unsigned char fxVal ) throw() {
	int tempVar;
	switch ( fx ) {
		case stmFxVolSlide:
			useLastFxData( aLastFx, fxVal );
			tempVar = m_volume;
			if ( m_tick != 0 ) {
				if ( highNibble( fxVal ) != 0 )
					tempVar += highNibble( fxVal );
				else
					tempVar -= lowNibble( fxVal );
				m_volume = clip( tempVar, 0, 0x40 );
			}
			break;
		case stmFxTremor:
			//! @todo Implement!
			break;
		case stmFxTremolo:
			if ( m_tick != 0 ) {
				aTremoloPhase = ( aTremoloPhase + ( highNibble( fxVal ) << 2 ) ) & 0xff;
				aDeltaVolume = ( SinLookup[aTremoloPhase] * lowNibble( fxVal ) ) >> 4;
			}
			break;
	}
}

void StmChannel::doVibratoFx( const unsigned char fx, unsigned char fxVal, const bool isCombined ) throw() {
	switch ( fx ) {
		case stmFxVibrato:
			if ( isCombined )
				combineLastFxData( aLastVibratoFx, fxVal );
			else
				combineLastFxData( aLastFx, fxVal );
			aLastVibratoFx = fxVal;
			if ( m_tick != 0 ) {
				aVibratoPhase = ( aVibratoPhase + ( highNibble( fxVal ) << 2 ) ) & 0xff;
				aDeltaFrq = ( SinLookup[aVibratoPhase] * lowNibble( fxVal ) ) >> 5;
			}
			break;
	}
}

void StmChannel::doPitchFx( const unsigned char fx, unsigned char fxVal, const bool isCombined ) throw() {
	switch ( fx ) {
		case stmFxSlideDown:
			if ( isCombined )
				useLastFxData( aLastPitchFx, fxVal );
			aLastPitchFx = fxVal;
			if ( m_tick != 0 )
				pitchDown( fxVal );
			break;
		case stmFxSlideUp:
			if ( isCombined )
				useLastFxData( aLastPitchFx, fxVal );
			aLastPitchFx = fxVal;
			if ( m_tick != 0 )
				pitchUp( fxVal );
			break;
		case stmFxPorta:
			if ( isCombined )
				useLastFxData( aLastPitchFx, fxVal );
			else
				useLastFxData( aLastFx, fxVal );
			aLastPitchFx = fxVal;
			if (( getCurrentSample() ) && ( m_tick != 0 ) ) {
				Frequency baseFrq = 0;
				Frequency targetFrq = 0;
				targetFrq = stmFrequency( aTargetNote, getCurrentSample()->getBaseFrq() ); // calculate target frq
				targetFrq = clip( targetFrq, aMinFrequency, aMaxFrequency );
				if ( m_frequency == 0 )
					m_frequency = targetFrq;
				else {
					if ( targetFrq < m_frequency ) { // pitch down...
						baseFrq = m_frequency;
						pitchDown( baseFrq, fxVal );
						if ( baseFrq <= targetFrq ) {
							m_frequency = targetFrq;
							aNote = aTargetNote;
						}
						else
							m_frequency = baseFrq;
					}
					else { // pitch up...
						baseFrq = m_frequency;
						pitchUp( baseFrq, fxVal );
						if ( baseFrq >= targetFrq ) {
							m_frequency = targetFrq;
							aNote = aTargetNote;
						}
						else
							m_frequency = baseFrq;
					}
				}
			}
			break;
	}
}

void StmChannel::doSpecialFx( const unsigned char fx, unsigned char fxVal ) throw( PppException ) {
	switch ( fx ) {
		case stmFxArpeggio:
			if ( !getCurrentSample() )
				break;
			useLastFxData( aLastFx, fxVal );
			switch ( m_tick % 3 ) {
				case 0: // normal note
					m_frequency = stmFrequency( aNote, getCurrentSample()->getBaseFrq() );
					break;
				case 1: // +x half notes...
					m_frequency = stmFrequency( deltaNote( aNote, highNibble( aLastFx ) ), getCurrentSample()->getBaseFrq() );
					break;
				case 2: // +y half notes...
					m_frequency = stmFrequency( deltaNote( aNote, lowNibble( aLastFx ) ), getCurrentSample()->getBaseFrq() );
					break;
			}
			m_frequency = clip( m_frequency, aMinFrequency, aMaxFrequency );
			break;
	}
}

void StmChannel::pitchUp( const short delta ) throw() {
	pitchUp( m_frequency, delta );
}

void StmChannel::pitchDown( const short delta ) throw() {
	pitchDown( m_frequency, delta );
}

void StmChannel::pitchUp( Frequency& frq, const short delta ) throw() {
	frq = deltaFrq( frq, -delta );
	frq = clip( frq, aMinFrequency, aMaxFrequency );
}

void StmChannel::pitchDown( Frequency& frq, const short delta ) throw() {
	frq = deltaFrq( frq, delta );
	frq = clip( frq, aMinFrequency, aMaxFrequency );
}

void StmChannel::mixTick( AudioFifo::MixerBuffer &mixBuffer, const std::size_t bufSize, const uint8_t volume ) throw( PppException ) {
	if ( m_disabled )
		return;
	setGlobalVolume( volume );
	if (( !m_active ) || ( !getCurrentSample() ) || ( m_frequency == 0 ) ) {
		m_active = false;
		return;
	}
	uint32_t dy = static_cast<uint32_t>( bufSize * deltaFrq( m_frequency, aDeltaFrq ) / m_playbackFrequency;
	PPP_TEST( dy == 0 );
	BresenInterpolation bres( bufSize, dy ) );
	unsigned short currVol = clip( m_volume + aDeltaVolume, 0, 0x40 ) * aGlobalVol;
	GenSample::Ptr currSmp = getCurrentSample();
	for ( unsigned int i = 0; i < bufSize; i++ ) {
		if ( m_position == GenSample::EndOfSample )
			break;
		mixBuffer[( i<<1 )+0] += ( currSmp->getLeftSampleAt( m_position, m_panning ) * currVol ) >> 12;
		mixBuffer[( i<<1 )+1] += ( currSmp->getRightSampleAt( m_position, m_panning ) * currVol ) >> 12;
		bres.next( m_position );
	}
	if ( m_position != GenSample::EndOfSample )
		getCurrentSample()->adjustPos( m_position );
	if ( m_position == GenSample::EndOfSample )
		m_active = false;
}

void StmChannel::simTick( const std::size_t bufSize, const uint8_t volume ) {
	if ( m_disabled )
		return;
	setGlobalVolume( volume );
	if (( !m_active ) || ( !getCurrentSample() ) || ( m_frequency == 0 ) ) {
		m_active = false;
		return;
	}
	uint32_t dy = static_cast<uint32_t>( bufSize * deltaFrq( m_frequency, aDeltaFrq ) / m_playbackFrequency;
	PPP_TEST( dy == 0 );
	m_position += dy;
	getCurrentSample()->adjustPos( m_position );
	if ( m_position == GenSample::EndOfSample )
		m_active = false;
}

std::string StmChannel::getFxDesc() const throw() {
	StmCell::Ptr stmcell = std::static_pointer_cast<StmCell>( m_currCell );
	if ( !stmcell->isActive() )
		return "      ";
	if ( stmcell->getEffect() == stmEmptyEffect )
		return "      ";
	switch ( stmcell->getEffect() ) {
		case stmFxSetTempo:
			return "Tempo\x7f";
			break;
		case stmFxPatJump:
			return "JmOrd\x1a";
			break;
		case stmFxPatBreak:
			return "PBrk \xf6";
			break;
		case stmFxVolSlide:
			if ( highNibble( stmcell->getEffectValue() ) != 0 )
				return "VSld \x1e";
			else
				return "VSld \x1f";
			break;
		case stmFxSlideDown:
			return "Ptch \x19";
			break;
		case stmFxSlideUp:
			return "Ptch \x18";
			break;
		case stmFxPorta:
			return "Porta\x12";
			break;
		case stmFxVibrato:
			return "Vibr \xf7";
			break;
		case stmFxTremor:
			return "Tremr\xec";
			break;
		case stmFxArpeggio:
			return "Arp  \xf0";
			break;
		case stmFxVibVolSlide:
			return "VibVo\xf7";
			break;
		case stmFxPortVolSlide:
			return "PrtVo\x12";
			break;
		case stmFxTremolo:
			return "Tremo\xec";
			break;
		default:
			return "??????";
			break;
	}
	return "??????";
}


void StmChannel::updateStatus() throw( PppException ) {
	if ( m_active ) {
		PPP_TEST( !getCurrentSample() );
		m_statusString = stringf( "%.2x: %s%s %s %s P:%.2x V:%.2x %s",
		                         m_currSmpIndex,
		                         ( aNoteChanged ? "*" : " " ),
		                         getNoteName().c_str(),
		                         getFxName().c_str(),
		                         getFxDesc().c_str(),
		                         m_panning,
		                         clip( m_volume + aDeltaVolume, 0, 0x40 ),
		                         getCurrentSample()->getTitle().c_str()
		                       );
	}
	else {
		m_statusString = stringf( "     %s %s %s",
		                         ( std::static_pointer_cast<StmCell>( m_currCell )->getNote() == stmKeyOffNote ? "^^ " : "   " ),
		                         getFxName().c_str(),
		                         getFxDesc().c_str()
		                       );
	}
}

void StmChannel::setGlobalVolume( const unsigned char gVol, const bool applyNow ) throw() {
	if ( m_tick == 0 ) {
		aNextGlobalVol = gVol;
	}
	if ( aNoteChanged || applyNow ) {
		aGlobalVol = aNextGlobalVol;
	}
}
