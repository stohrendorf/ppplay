/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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
#include "s3mbase.h"
#include "s3mchannel.h"
#include "s3mpattern.h"
#include "s3mmodule.h"
#include "s3msample.h"
#include "s3mcell.h"

/**
 * @ingroup S3mMod
 * @{
 */

namespace ppp
{
namespace s3m
{

namespace
{
/**
 * @brief S3M sine wave lookup
 */
constexpr std::array<const int16_t, 64> S3mWaveSine = {
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

/**
 * @brief S3M ramp wave lookup
 */
constexpr std::array<const int16_t, 64> S3mWaveRamp = {
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

/**
 * @brief S3M square wave lookup
 */
constexpr std::array<const int16_t, 64> S3mWaveSquare = {
	{
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0
	}
};

/**
 * @brief S3M finetunes lookup
 */
constexpr std::array<const uint16_t, 16> S3mFinetunes = {
	{
		8363, 8413, 8463, 8529, 8581, 8651, 8723, 8757,
		7895, 7941, 7985, 8046, 8107, 8169, 8232, 8280
	}
};

/**
 * @brief Note periodic table for frequency calculations
 */
const std::array<const uint16_t, 12> Periods = {{1712 << 4, 1616 << 4, 1524 << 4, 1440 << 4, 1356 << 4, 1280 << 4, 1208 << 4, 1140 << 4, 1076 << 4, 1016 << 4,  960 << 4,  907 << 4}};

/**
 * @brief Calculate the period for a given note, octave and base frequency
 * @param[in] note Note value (without octave)
 * @param[in] oct Note octave
 * @param[in] c4spd Base frequency of the sample
 * @param[in] finetune Optional finetune
 * @return S3M Period for note @a note, base frequency @a c4spd and finetune @a finetune
 */
inline uint16_t st3PeriodEx( uint8_t note, uint8_t oct, uint16_t c4spd, uint16_t finetune = 8363 )
{
	return ( Periods[note] >> oct ) * finetune / (c4spd==0 ? 8363 : c4spd);
}
/**
 * @brief Calculate the period for a given note and base frequency
 * @param[in] note Note value (including octave)
 * @param[in] c4spd Base frequency of the sample
 * @param[in] finetune Optional finetune
 * @return S3M Period for note @a note, base frequency @a c4spd and finetune @a finetune
 */
inline uint16_t st3Period( uint8_t note, uint16_t c4spd, uint16_t finetune = 8363 )
{
	return st3PeriodEx( lowNibble( note ), highNibble( note ), c4spd, finetune );
}

/**
 * @brief Reverse calculate a note from a given period and C4 frequency
 * @param[in] per Note period
 * @param[in] c4spd Base frequency of the sample
 * @param[in] finetune Optional finetune
 * @return Note offset (12*octave+note)
 */
inline uint8_t periodToNoteOffset( uint16_t per, uint16_t c4spd, uint16_t finetune = 8363 )
{
	return std::round( -12 * std::log2( static_cast<float>( per ) * c4spd / ( finetune * Periods[0] ) ) );
}

/**
 * @brief Reverse-calculate the Note from the given period
 * @param[in] per Period
 * @param[in] c2spd Base frequency of the sample
 * @param[in] finetune Optional finetune
 * @return Note string
 * @note Time-critical
 */
inline std::string periodToNote( uint16_t per, uint16_t c2spd, uint16_t finetune = 8363 )
{
	if( per == 0 ) {
		return "p??";
	}
	if( c2spd == 0 ) {
		return "c??";
	}
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
	return stringFmt( "%s%d", NoteNames[minnote], int(minoct) );
}

/**
 * @brief Add/subtract semitones to/from a note
 * @param[in] note Base note
 * @param[in] delta Delta value
 * @return New note
 */
inline uint8_t deltaNote( uint8_t note, int8_t delta )
{
	uint16_t x = highNibble( note ) * 12 + lowNibble( note ) + delta;
	return ( ( x / 12 ) << 4 ) | ( x % 12 );
}

/**
 * @brief Clip a period if necessary
 * @param[in] amiga Set to @c true to use amiga limits
 * @param[in] period The period to clip
 * @return Clipped period
 */
uint16_t clipPeriod( bool amiga, uint16_t period )
{
	if( amiga ) {
		return clip<uint16_t>( period, 0x15c, 0xd60 );
	}
	else {
		return clip<uint16_t>( period, 0x40, 0x7fff );
	}
}
} // anonymous namespace

void S3mChannel::setSampleIndex( int32_t idx )
{
	m_sampleIndex = idx;
	if( !currentSample() || currentSample()->frequency() == 0 )
		setActive( false );
}

S3mChannel::S3mChannel( S3mModule* const module ) : GenChannel(),
	m_note( 0xff ),
	m_lastFxByte( 0 ),
	m_lastVibratoData( 0 ),
	m_lastPortaSpeed( 0 ),
	m_tremorVolume( 0 ),
	m_noteChanged( false ),
	m_currentVolume( 0 ),
	m_realVolume( 0 ),
	m_baseVolume( 0 ),
	m_tremorMute( false ),
	m_retrigCount( 0 ),
	m_zeroVolCounter( 3 ),
	m_module( module ),
	m_basePeriod( 0 ),
	m_realPeriod( 0 ),
	m_portaTargetPeriod( 0 ),
	m_vibratoPhase( 0 ),
	m_vibratoWaveform( 0 ),
	m_tremoloPhase( 0 ),
	m_tremoloWaveform( 0 ),
	m_countdown( 0 ),
	m_tremorCounter( 0 ),
	m_c2spd( 0 ),
	m_glissando( false ),
	m_currentCell(new S3mCell()),
	m_bresen( 1, 1 ),
	m_currentFxStr( "      " ),
	m_sampleIndex( 101 ),
	m_panning( 0x20 )
{
}

S3mChannel::~S3mChannel()
{
	delete m_currentCell;
}

const S3mSample* S3mChannel::currentSample() const
{
	if( !inRange<int>( m_sampleIndex, 0, m_module->numSamples() - 1 ) ) {
		//setActive( false );
		return nullptr;
	}
	return m_module->sampleAt( m_sampleIndex );
}

std::string S3mChannel::internal_noteName() const
{
	if( m_note == s3mEmptyNote ) {
		return "   ";
	}
	if( !isActive() || isDisabled() ) {
		return "   ";
	}
	return periodToNote( m_realPeriod, currentSample()->frequency() );
}

std::string S3mChannel::internal_effectName() const
{
	if( m_currentCell->effect() == s3mEmptyCommand ) {
		return "...";
	}
	uint8_t fx = m_currentCell->effect();
	if( inRange<int>( fx, 1, 27 ) ) {
		return stringFmt( "%c%02X", char( fx - 1 + 'A' ), int(m_currentCell->effectValue()) );
	}
	else {
		logger()->warn( L4CXX_LOCATION, "Effect out of range: %#x", int(fx) );
		return stringFmt( "?%02X", int(m_currentCell->effectValue()) );
	}
}

std::string S3mChannel::internal_effectDescription() const
{
	return m_currentFxStr;
}

void S3mChannel::update( const S3mCell* cell, bool patDelay, bool estimateOnly )
{
	if( isDisabled() ) {
		return;
	}
	if( m_module->state().tick == 0 ) {
		if(estimateOnly) {
			if(!cell) {
				return;
			}
			switch( static_cast<S3mEffects>( cell->effect() ) ) {
				case s3mFxSpeed:
					fxSpeed( cell->effectValue() );
					break;
				case s3mFxTempo:
					fxTempo( cell->effectValue() );
					break;
				default:
					// silence ;-)
					break;
			}
			return;
		}
		m_noteChanged = false;
		m_currentFxStr = "      ";
		m_currentCell->clear();
		if( cell && !patDelay ) {
			*m_currentCell = *cell;
		}

		if( m_currentCell->note() != s3mEmptyNote || m_currentCell->instrument() != s3mEmptyInstr || m_currentCell->volume() != s3mEmptyVolume || m_currentCell->effect() != s3mEmptyCommand ) {
			triggerNote();
		}

		if( m_currentCell->effect() != 0 ) {
			m_zeroVolCounter = 3;
		}
		else if( m_module->hasZeroVolOpt() ) {
			if( m_currentVolume == 0 && m_currentCell->volume() == 0 && m_currentCell->instrument() == s3mEmptyInstr && m_currentCell->note() == s3mEmptyNote ) {
				m_zeroVolCounter--;
				if( m_zeroVolCounter == 0 ) {
					setActive( false );
					return;
				}
			}
			else {
				m_zeroVolCounter = 3;
			}
		}

		reuseIfZeroEx( m_lastFxByte, m_currentCell->effectValue() ); // TODO check if right here...
		if( m_currentCell->effect() == s3mEmptyCommand ) {
			if( m_module->hasAmigaLimits() ) {
				m_countdown = 0;
			}
			if( m_basePeriod != m_realPeriod ) {
				m_realPeriod = m_basePeriod;
				recalcFrequency();
			}
		}
		else if( m_currentCell->effect() == s3mFxVolSlide ) {
			m_countdown = 0;
			if( m_basePeriod != m_realPeriod ) {
				m_realPeriod = m_basePeriod;
				recalcFrequency();
			}
		}
		else if( m_currentCell->effect() != s3mFxTremor ) {
			m_tremorCounter = 0;
			m_tremorMute = false;
		}
		else {
			// better readable than an if-expression
			switch( m_currentCell->effect() ) {
				case s3mFxVibrato:
				case s3mFxFineVibrato:
				case s3mFxVibVolSlide:
				case s3mFxTremolo:
					m_vibratoPhase |= 0x40;
			}
		}

		switch( static_cast<S3mEffects>( m_currentCell->effect() ) ) {
			case s3mFxSpeed:
				fxSpeed( m_currentCell->effectValue() );
				break;
			case s3mFxVolSlide:
				fxVolSlide( m_currentCell->effectValue() );
				break;
			case s3mFxPitchDown:
				fxPitchSlideDown( m_currentCell->effectValue() );
				break;
			case s3mFxPitchUp:
				fxPitchSlideUp( m_currentCell->effectValue() );
				break;
			case s3mFxTremor:
				fxTremor( m_currentCell->effectValue() );
				break;
			case s3mFxArpeggio:
				fxArpeggio( m_currentCell->effectValue() );
				break;
			case s3mFxRetrig:
				fxRetrigger( m_currentCell->effectValue() );
				break;
			case s3mFxSpecial:
				fxSpecial( m_currentCell->effectValue() );
				break;
			case s3mFxTempo:
				fxTempo( m_currentCell->effectValue() );
				break;
			case s3mFxJumpOrder:
				m_currentFxStr = "JmOrd\x1a";
				break;
			case s3mFxBreakPat:
				m_currentFxStr = "PBrk \xf6";
				break;
			case s3mFxSetPanning:
				// this is a non-standard effect, so we ignore the fx byte
				m_currentFxStr = "StPan\x1d";
				if( m_currentCell->effectValue() <= 0x80 ) {
					m_panning = m_currentCell->effectValue() >> 1;
				}
				else if( m_currentCell->effectValue() == 0xa4 ) {
					m_panning = m_currentCell->effectValue();
				}
				break;
			case s3mFxVibVolSlide:
				m_currentFxStr = "VibVo\xf7";
				break;
			case s3mFxPortaVolSlide:
				m_currentFxStr = "PrtVo\x12";
				break;
			case s3mFxPorta:
				m_currentFxStr = "Porta\x12";
				break;
			case s3mFxVibrato:
				m_currentFxStr = "Vibr \xf7";
				break;
			case s3mFxFineVibrato:
				m_currentFxStr = "FnVib\xf7";
				break;
			case s3mFxTremolo:
				m_currentFxStr = "Tremo\xec";
				break;
			case s3mFxGlobalVol:
				m_currentFxStr = "GloVol";
				break;
			case s3mFxOffset:
				// handled...
				break;
		}
	} // endif(tick==0)
	else if( m_currentCell->effect() != 0 && !estimateOnly ) { // if(tick!=0)
		switch( static_cast<S3mEffects>( m_currentCell->effect() ) ) {
			case s3mFxVolSlide:
				fxVolSlide( m_currentCell->effectValue() );
				break;
			case s3mFxPitchDown:
				fxPitchSlideDown( m_currentCell->effectValue() );
				break;
			case s3mFxPitchUp:
				fxPitchSlideUp( m_currentCell->effectValue() );
				break;
			case s3mFxPorta:
				fxPorta( m_currentCell->effectValue(), false );
				break;
			case s3mFxVibrato:
				fxVibrato( m_currentCell->effectValue(), false, false );
				break;
			case s3mFxTremor:
				fxTremor( m_currentCell->effectValue() );
				break;
			case s3mFxArpeggio:
				fxArpeggio( m_currentCell->effectValue() );
				break;
			case s3mFxVibVolSlide:
				fxVolSlide( m_currentCell->effectValue() );
				fxVibrato( m_lastVibratoData, false, true );
				m_currentFxStr = "VibVo\xf7";
				break;
			case s3mFxPortaVolSlide:
				fxVolSlide( m_currentCell->effectValue() );
				fxPorta( m_lastPortaSpeed, true );
				m_currentFxStr = "PrtVo\x12";
				break;
			case s3mFxRetrig:
				fxRetrigger( m_currentCell->effectValue() );
				break;
			case s3mFxTremolo:
				fxTremolo( m_currentCell->effectValue() );
				break;
			case s3mFxSpecial:
				fxSpecial( m_currentCell->effectValue() );
				break;
			case s3mFxFineVibrato:
				fxVibrato( m_currentCell->effectValue(), true, false );
				break;
			case s3mFxGlobalVol:
				fxGlobalVolume( m_currentCell->effectValue() );
				break;
			case s3mFxOffset:
			case s3mFxJumpOrder:
			case s3mFxBreakPat:
			case s3mFxTempo:
			case s3mFxSetPanning:
			case s3mFxSpeed:
				// already handled...
				break;
		}
	}
	updateStatus();
}

void S3mChannel::internal_mixTick( MixerFrameBuffer* mixBuffer )
{
	if( !mixBuffer ) {
		return;
	}
	if( isDisabled() ) {
		return;
	}
	if( !isActive() || !currentSample() || m_basePeriod == 0 ) {
		setActive( false );
		return;
	}
	if( m_module->state().tick == 0 && m_zeroVolCounter != -1 && isActive() ) {
		if( m_currentVolume == 0 ) {
			m_zeroVolCounter++;
		}
		else {
			m_zeroVolCounter = 0;
		}
		if( m_zeroVolCounter == 3 ) {
			m_zeroVolCounter = 0;
			setActive( false );
			m_note = s3mEmptyNote;
			return;
		}
	}
	if( !isActive() ) {
		return;
	}
	if( m_module->frequency() * (*mixBuffer)->size() == 0 ) {
		setActive( false );
		return;
	}
	m_bresen.reset( m_module->frequency(), 8363*1712 / m_realPeriod );
	recalcVolume();
	uint16_t currVol = m_realVolume;
	const S3mSample* currSmp = currentSample();
	GenSample::PositionType pos = position();
	uint8_t volL = 0x20, volR = 0x20;
	if( m_panning > 0x20 && m_panning != 0xa4 ) {
		volL = 0x40 - m_panning;
	}
	if( m_panning < 0x20 ) {
		volR = m_panning;
	}
	else if( m_panning == 0xa4 ) {
		volR = 0xa4;
	}
	for( MixerSampleFrame & frame : **mixBuffer ) {
		BasicSampleFrame sampleVal = currSmp->sampleAt( pos );
		if( m_panning != 0xa4 ) {
			sampleVal.mulRShift( volL, volR, 5 );
		}
		else {
			sampleVal.right = -sampleVal.right;
		}
		sampleVal.mulRShift( currVol, 6 );
		frame += sampleVal;
		if( pos == GenSample::EndOfSample ) {
			break;
		}
		m_bresen.next( pos );
	}
	if( pos != GenSample::EndOfSample ) {
		currentSample()->adjustPosition( pos );
	}
	setPosition( pos );
	if( pos == GenSample::EndOfSample ) {
		setActive( false );
	}
}

std::string S3mChannel::internal_cellString() const
{
	return m_currentCell->trackerString();
}

void S3mChannel::internal_updateStatus()
{
	const S3mSample* smp = currentSample();
	if( !smp ) {
		setActive( false );
		setStatusString( "         ..." );
		return;
	}
	if( isActive() ) {
		std::string panStr;
		if( m_panning == 0xa4 ) {
			panStr = "Srnd ";
		}
		else if( m_panning == 0x00 ) {
			panStr = "Left ";
		}
		else if( m_panning == 0x20 ) {
			panStr = "Centr";
		}
		else if( m_panning == 0x40 ) {
			panStr = "Right";
		}
		else {
			panStr = stringFmt( "%4d%%", ( m_panning - 0x20 ) * 100 / 0x40 );
		}
		std::string volStr = stringFmt( "%3d%%", clip<int>( m_currentVolume , 0, 0x3f ) * 100 / 0x3f );
		setStatusString( stringFmt(
			"%02d: %s%s %s %s P:%s V:%s %s",
			m_sampleIndex + 1,
			m_noteChanged ? "*" : " ",
			noteName(),
			effectName(),
			effectDescription(),
			panStr,
			volStr,
			smp->title()
			)
		);
	}
	else {
		setStatusString( stringFmt(
			"     %s %s %s",
			m_currentCell->note() == s3mKeyOffNote ? "^^ " : "   ",
			effectName(),
			effectDescription()
			)
		);
	}
}

IArchive& S3mChannel::serialize( IArchive* data )
{
	GenChannel::serialize( data )
	% m_note
	% m_lastFxByte
	% m_lastVibratoData
	% m_lastPortaSpeed
	% m_tremorVolume
	% m_noteChanged
	% m_currentVolume
	% m_realVolume
	% m_baseVolume
	% m_tremorMute
	% m_retrigCount
	% m_zeroVolCounter
	% m_basePeriod
	% m_realPeriod
	% m_portaTargetPeriod
	% m_vibratoPhase
	% m_vibratoWaveform
	% m_tremoloPhase
	% m_tremoloWaveform
	% m_countdown
	% m_tremorCounter
	% m_c2spd
	% m_glissando
	% m_sampleIndex;

	data->archive( &m_bresen );
	return data->archive( m_currentCell );
}

void S3mChannel::fxPitchSlideUp( uint8_t fxByte )
{
	if( m_basePeriod == 0 )
		return;
	reuseIfZero( m_lastFxByte, fxByte );
	uint16_t delta = 0;
	if( m_module->state().tick == 0 ) {
		if( fxByte <= 0xe0 ) {
			m_currentFxStr = "Ptch\x1e\x1e";
			return;
		}
		if( fxByte <= 0xf0 ) {
			m_currentFxStr = "Ptch \x18";
			delta = lowNibble( fxByte );
		}
		else {
			m_currentFxStr = "Ptch \x1e";
			delta = lowNibble( fxByte ) << 2;
		}
	}
	else {
		if( fxByte >= 0xe0 )
			return;
		delta = fxByte << 2;
	}
	m_basePeriod = m_realPeriod = std::max( 0, m_basePeriod - delta );
	recalcFrequency();
}

void S3mChannel::fxPitchSlideDown( uint8_t fxByte )
{
	if( m_basePeriod == 0 )
		return;
	reuseIfZero( m_lastFxByte, fxByte );
	uint16_t delta = 0;
	if( m_module->state().tick == 0 ) {
		if( fxByte <= 0xe0 ) {
			m_currentFxStr = "Ptch\x1f\x1f";
			return;
		}
		if( fxByte <= 0xf0 ) {
			m_currentFxStr = "Ptch \x19";
			delta = lowNibble( fxByte );
		}
		else {
			m_currentFxStr = "Ptch \x1f";
			delta = lowNibble( fxByte ) << 2;
		}
	}
	else {
		if( fxByte >= 0xe0 )
			return;
		delta = fxByte << 2;
	}
	m_basePeriod = m_realPeriod = std::min( 0x7fff, m_basePeriod + delta );
	recalcFrequency();
}

void S3mChannel::fxVolSlide( uint8_t fxByte )
{
	reuseIfZero( m_lastFxByte, fxByte );
	if( lowNibble( fxByte ) == 0x0f ) {
		if( highNibble( fxByte ) == 0 ) {
			m_currentFxStr = "VSld \x1f";
			m_currentVolume = std::max( 0, m_currentVolume - lowNibble( fxByte ) );
		}
		else {
			m_currentFxStr = "VSld \x18";
			if( m_module->state().tick == 0 ) {
				m_currentVolume = std::min( 63, m_currentVolume + highNibble( fxByte ) );
			}
		}
	}
	else if( highNibble( fxByte ) == 0x0f ) {
		if( lowNibble( fxByte ) == 0 ) {
			m_currentFxStr = "VSld \x1e";
			m_currentVolume = std::min( 63, m_currentVolume + highNibble( fxByte ) );
		}
		else {
			m_currentFxStr = "VSld \x19";
			if( m_module->state().tick == 0 ) {
				m_currentVolume = std::max( 0, m_currentVolume - lowNibble( fxByte ) );
			}
		}
	}
	else {
		if( lowNibble( fxByte ) == 0 ) {
			m_currentFxStr = "VSld \x1e";
			if( m_module->hasFastVolSlides() || m_module->state().tick != 0 ) {
				m_currentVolume = std::min( 63, m_currentVolume + highNibble( fxByte ) );
			}
		}
		else {
			m_currentFxStr = "VSld \x1f";
			if( m_module->hasFastVolSlides() || m_module->state().tick != 0 ) {
				m_currentVolume = std::max( 0, m_currentVolume - lowNibble( fxByte ) );
			}
		}
	}
	recalcVolume();
}

void S3mChannel::recalcFrequency()
{
	if( m_module->hasAmigaLimits() ) {
		m_basePeriod = clipPeriod( true, m_basePeriod );
	}
	uint16_t per = m_realPeriod;
	if( per == 0 ) {
		per = m_basePeriod;
	}
	m_realPeriod = clipPeriod( m_module->hasAmigaLimits(), per );
}

void S3mChannel::recalcVolume()
{
	m_realVolume = ( static_cast<int>( m_module->state().globalVolume ) * m_currentVolume ) >> 6;
}

void S3mChannel::fxPorta( uint8_t fxByte, bool noReuse )
{
	m_currentFxStr = "Porta\x12";
	if( m_module->state().tick == 0 ) {
		return;
	}
	if( m_basePeriod == 0 ) {
		if( m_portaTargetPeriod == 0 )
			return;
		m_realPeriod = m_basePeriod = m_portaTargetPeriod;
		fxByte = m_portaTargetPeriod & 0xff;
	}
	if( !noReuse )
		reuseIfZero( m_lastPortaSpeed, fxByte );
	if( m_basePeriod == m_portaTargetPeriod )
		return;
	if( m_basePeriod > m_portaTargetPeriod ) {
		int tmp = m_basePeriod - ( fxByte << 2 );
		if( tmp < m_portaTargetPeriod )
			tmp = m_portaTargetPeriod;
		if( m_glissando )
			tmp = glissando( tmp );
		m_realPeriod = m_basePeriod = tmp;
		recalcFrequency();
	}
	else {
		int tmp = m_basePeriod + ( fxByte << 2 );
		if( tmp > m_portaTargetPeriod )
			tmp = m_portaTargetPeriod;
		if( m_glissando )
			tmp = glissando( tmp );
		m_realPeriod = m_basePeriod = tmp;
		recalcFrequency();
	}
}

/**
 * @brief Look up a wave value
 * @param[in] waveform Waveform selector
 * @param[in] phase Wave phase
 * @return Lookup value
 */
static int16_t waveValue( uint8_t waveform, uint8_t phase )
{
	switch( waveform & 7 ) {
		case 1:
			if( phase & 0x40 )
				return S3mWaveRamp[0];
		case 5:
			return S3mWaveRamp[phase & 0x3f];
		case 2:
			if( phase & 0x40 )
				return S3mWaveSquare[0];
		case 6:
			return S3mWaveSquare[phase & 0x3f];
		case 0:
		case 3:
			if( phase & 0x40 )
				return S3mWaveSine[0];
		case 4:
		case 7:
			return S3mWaveSine[phase & 0x3f];
		default:
			return 0;
	}
}

void S3mChannel::fxVibrato( uint8_t fxByte, bool fine, bool noReuse )
{
	if( !fine ) {
		m_currentFxStr = "Vibr \xf7";
	}
	else {
		m_currentFxStr = "FnVib\xf7";
	}
	if( !noReuse )
		reuseNibblesIfZero( m_lastVibratoData, fxByte );
	if( m_basePeriod == 0 )
		return;
	int val = waveValue( m_vibratoWaveform, m_vibratoPhase );
	if( ( m_vibratoWaveform & 3 ) == 3 ) { // random vibrato
		m_vibratoPhase = ( m_vibratoPhase + ( std::rand() & 0x0f ) ) & 0x3f;
	}
	m_vibratoPhase = ( m_vibratoPhase + highNibble( fxByte ) ) & 0x3f;
	val = ( val * lowNibble( fxByte ) ) >> 4;
	if( m_module->st2Vibrato() )
		val >>= 1;
	if( fine )
		val >>= 2;
	m_realPeriod = m_basePeriod + val;
	recalcFrequency();
}

void S3mChannel::fxNoteCut( uint8_t fxByte )
{
	m_currentFxStr = "NCut \xd4";
	fxByte = lowNibble( fxByte );
	if( m_module->state().tick == 0 ) {
		m_countdown = fxByte;
		return;
	}
	if( m_countdown == 0 )
		return;
	m_countdown--;
	if( m_countdown == 0 ) {
		setActive( false );
	}
}

void S3mChannel::fxNoteDelay( uint8_t fxByte )
{
	m_currentFxStr = "Delay\xc2";
	fxByte = lowNibble( fxByte );
	if( m_module->state().tick == 0 ) {
		m_countdown = fxByte;
		return;
	}
	if( m_countdown == 0 )
		return;
	m_countdown--;
	if( m_countdown == 0 ) {
		setActive( true );
		playNote();
	}
}

void S3mChannel::fxGlobalVolume( uint8_t fxByte )
{
	m_currentFxStr = "GloVol";
	if( fxByte <= 64 )
		m_module->state().globalVolume = fxByte;
}

void S3mChannel::fxFineTune( uint8_t fxByte )
{
	m_currentFxStr = "FTune\xe6";
	if( m_module->state().tick != 0 )
		return;
	fxByte = lowNibble( fxByte );
	m_c2spd = m_basePeriod = m_realPeriod = S3mFinetunes[fxByte];
	recalcFrequency();
}

void S3mChannel::fxSetVibWaveform( uint8_t fxByte )
{
	m_currentFxStr = "VWave\x9f";
	m_vibratoWaveform = lowNibble( fxByte );
}

void S3mChannel::fxSetTremWaveform( uint8_t fxByte )
{
	m_currentFxStr = "TWave\x9f";
	m_tremoloWaveform = lowNibble( fxByte );
}

void S3mChannel::fxRetrigger( uint8_t fxByte )
{
	m_currentFxStr = "Retr \xec";
	reuseIfZero( m_lastFxByte, fxByte );
	if( lowNibble( fxByte ) == 0 || lowNibble( fxByte ) > m_countdown ) {
		m_countdown++;
		return;
	}
	m_countdown = 0;
	setPosition( 0 );
	int nvol = m_currentVolume;
	switch( highNibble( fxByte ) ) {
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
	m_currentVolume = clip<int>( nvol, 0, 63 );
	recalcVolume();
}

void S3mChannel::fxOffset( uint8_t fxByte )
{
	m_currentFxStr = "Offs \xaa";
	setPosition( fxByte << 8 );
}

void S3mChannel::fxTremor( uint8_t fxByte )
{
	m_currentFxStr = "Tremr\xec";
	reuseIfZero( m_lastFxByte, fxByte );
	if( m_tremorCounter != 0 ) {
		m_tremorCounter--;
		return;
	}
	if( m_tremorMute ) {
		m_tremorMute = false;
		m_currentVolume = 0;
		recalcVolume();
		m_tremorCounter = lowNibble( fxByte );
	}
	else {
		m_tremorMute = true;
		m_currentVolume = m_baseVolume;
		recalcVolume();
		m_tremorCounter = highNibble( fxByte );
	}
}

void S3mChannel::fxTempo( uint8_t fxByte )
{
	m_currentFxStr = "Tempo\x7f";
	if( fxByte <= 0x20 )
		return;
	m_module->setTempo( fxByte );
}

void S3mChannel::fxSpeed( uint8_t fxByte )
{
	m_currentFxStr = "Speed\x7f";
	if( fxByte == 0 )
		return;
	m_module->setSpeed( fxByte );
}

void S3mChannel::fxArpeggio( uint8_t fxByte )
{
	m_currentFxStr = "Arp  \xf0";
	if( !currentSample() )
		return;
	reuseIfZero( m_lastFxByte, fxByte );
	switch( m_module->state().tick % 3 ) {
		case 0: // normal note
			m_realPeriod = st3Period( m_note, currentSample()->frequency() );
			break;
		case 1: // +x half notes...
			m_realPeriod = st3Period( deltaNote( m_note, highNibble( m_lastFxByte ) ), currentSample()->frequency() );
			break;
		case 2: // +y half notes...
			m_realPeriod = st3Period( deltaNote( m_note, lowNibble( m_lastFxByte ) ), currentSample()->frequency() );
			break;
	}
	recalcFrequency();
}

void S3mChannel::fxSpecial( uint8_t fxByte )
{
	reuseIfZero( m_lastFxByte, fxByte );
	switch( static_cast<S3mSpecialEffects>( highNibble( fxByte ) ) ) {
		case s3mSFxNoteDelay:
			fxNoteDelay( fxByte );
			break;
		case s3mSFxNoteCut:
			fxNoteCut( fxByte );
			break;
		case s3mSFxSetGlissando:
			m_currentFxStr = "Gliss\xcd";
			m_glissando = fxByte != 0;
			break;
		case s3mSFxSetFinetune:
			fxFineTune( fxByte );
			break;
		case s3mSFxSetVibWave:
			fxSetVibWaveform( fxByte );
			break;
		case s3mSFxSetTremWave:
			fxSetTremWaveform( fxByte );
			break;
		case s3mSFxSetPan:
			m_currentFxStr = "StPan\x1d";
			m_panning = lowNibble( fxByte ) * 0x40 / 0x0f;
			break;
		case s3mSFxStereoCtrl:
			m_currentFxStr = "SCtrl\x1d";
			if( lowNibble( fxByte ) <= 7 )
				m_panning = ( lowNibble( fxByte ) + 8 ) * 0x40 / 0x0f;
			else
				m_panning = ( lowNibble( fxByte ) - 8 ) * 0x40 / 0x0f;
			break;
		case s3mSFxPatLoop:
		case s3mSFxPatDelay:
			// handled...
			break;
	}
}

void S3mChannel::fxTremolo( uint8_t fxByte )
{
	m_currentFxStr = "Tremo\xec";
	reuseNibblesIfZero( m_lastFxByte, fxByte );
	if( m_baseVolume == 0 )
		return;
	int val = m_baseVolume + ( ( lowNibble( fxByte ) * waveValue( m_tremoloWaveform, m_tremoloPhase ) ) >> 7 );
	if( val < 0 )
		val = 0;
	else if( val >= 64 )
		val = 63;
	if( ( m_tremoloWaveform & 3 ) == 3 ) { // random vibrato
		m_tremoloPhase = ( m_tremoloPhase + ( std::rand() & 0x0f ) ) & 0x3f;
	}
	m_vibratoPhase = ( m_vibratoPhase + highNibble( fxByte ) ) & 0x3f;
	recalcVolume();
}

uint16_t S3mChannel::glissando( uint16_t period )
{
	uint8_t no = periodToNoteOffset( period, currentSample()->frequency() );
	return st3PeriodEx( no % 12, no / 12, currentSample()->frequency() );
}

void S3mChannel::triggerNote()
{
	if( m_currentCell->effect() == s3mFxSpecial && highNibble( m_currentCell->effectValue() ) == s3mSFxNoteDelay ) {
		return;
	}
	playNote();
}

void S3mChannel::playNote()
{
	if( m_currentCell->instrument() >= 101 ) {
		setSampleIndex( -1 );
	}
	else if( m_currentCell->instrument() != s3mEmptyInstr ) {
		setSampleIndex( m_currentCell->instrument() - 1 );
		if( currentSample() ) {
			m_baseVolume = m_currentVolume = std::min<uint8_t>( currentSample()->volume(), 63 );
			m_c2spd = currentSample()->frequency();
			recalcVolume();
		}
		else {
			setActive( false );
			return;
		}
	}

	if( m_currentCell->note() != s3mEmptyNote ) {
		if( m_currentCell->note() == s3mKeyOffNote ) {
			m_realPeriod = 0;
			recalcFrequency();
			m_currentVolume = 0;
			recalcVolume();
			setActive( false );
		}
		else if( currentSample() ) {
			setActive( true );
			m_portaTargetPeriod = st3Period( m_currentCell->note(), m_c2spd );
			if( m_currentCell->effect() != s3mFxPorta && m_currentCell->effect() != s3mFxPortaVolSlide ) {
				m_realPeriod = m_basePeriod = m_portaTargetPeriod;
				m_tremoloPhase = m_vibratoPhase = 0;
				recalcFrequency();
				if( m_currentCell->effect() == s3mFxOffset ) {
					fxOffset( m_currentCell->effectValue() );
				}
				else {
					setPosition( 0 );
				}
				m_note = m_currentCell->note();
				m_noteChanged = true;
			}
		}
	}
	uint8_t vol = m_currentCell->volume();
	if( vol != s3mEmptyVolume ) {
		vol = std::min<uint8_t>( vol, 63 );
		m_currentVolume = vol;
		recalcVolume();
		m_baseVolume = vol;
	}
}

void S3mChannel::setPanning( uint8_t pan )
{
	if( pan > 0x40 && pan != 0xa4 ) {
		return;
	}
	m_panning = pan;
}

light4cxx::Logger* S3mChannel::logger()
{
	return light4cxx::Logger::get( GenChannel::logger()->name() + ".s3m" );
}

} // namespace s3m
} // namespace ppp

/**
 * @}
 */
