/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#include "modchannel.h"
#include "genmod/genbase.h"

#include "modmodule.h"
#include "modbase.h"
#include "modcell.h"
#include "modsample.h"

#include <cmath>
#include <boost/assert.hpp>
#include <boost/format.hpp>

namespace ppp
{
namespace mod
{

namespace
{
#ifdef MOD_USE_NTSC_FREQUENCY
constexpr uint32_t FrequencyBase = 7159090.5 / 2;
#else
constexpr uint32_t FrequencyBase = 7093789.2 / 2;
#endif

constexpr std::array<const int16_t, 32> WaveSine = {{
		0, 24, 49, 74, 97, 120, 141, 161,
		180, 197, 212, 224, 235, 244, 250, 253,
		255, 253, 250, 244, 235, 224, 212, 197,
		180, 161, 141, 120, 97, 74, 49, 24
	}
};

inline constexpr double finetuneMultiplicator( uint8_t finetune )
{
	return pow( 2.0, -( finetune > 7 ? finetune - 16 : finetune ) / ( 12 * 8 ) );
}
} // anonymouse namespace

/**
 * @todo According to some Amiga assembler sources, periods
 * are clipped to a range of 0x71..0x358 (113..856 decimal respectively).
 */

ModChannel::ModChannel( ModModule* parent ) :
	m_module( parent ), m_currentCell(new ModCell()), m_volume( 0 ), m_physVolume( 0 ),
	m_finetune( 0 ), m_tremoloWaveform( 0 ), m_tremoloPhase( 0 ),
	m_vibratoWaveform( 0 ), m_vibratoPhase( 0 ), m_glissando( false ),
	m_period( 0 ), m_physPeriod( 0 ), m_portaTarget( 0 ),
	m_lastVibratoFx( 0 ), m_lastTremoloFx( 0 ), m_portaSpeed( 0 ),
	m_lastOffsetFx( 0 ), m_sampleIndex( 0 ), m_bresen( 1, 1 ),
	m_effectDescription( "      " )
{
	BOOST_ASSERT_MSG( parent != nullptr, "ModChannel may not have a nullptr as a parent" );
}

ModChannel::~ModChannel()
{
	delete m_currentCell;
}

// TODO mt_setfinetune
void ModChannel::update( const ModCell* cell, bool patDelay )
{
// 	if(isDisabled())
// 		return;
	uint8_t delayTick = 0;
	if( cell && cell->effect() == 0x0e && highNibble( cell->effectValue() ) == 0x0d ) {
		delayTick = lowNibble( cell->effectValue() );
	}
	if( m_module->state().tick == delayTick ) {
// 		m_noteChanged = false;
// 		m_currentFxStr = "      ";
		m_currentCell->clear();
		if( cell && !patDelay ) {
			*m_currentCell = *cell;
		}

		if( m_currentCell->sampleNumber() != 0 ) {
			m_sampleIndex = m_currentCell->sampleNumber();
			if( currentSample() ) {
				m_physVolume = m_volume = currentSample()->volume();
				m_finetune = currentSample()->finetune();
			}
		}
		if( m_currentCell->period() != 0 ) {
			if( m_period == 0 || (m_currentCell->effect() != 3 && m_currentCell->effect() != 5) ) {
				setCellPeriod();
				setPosition( 0 );
				setActive( true );
			}
// 			else {
			setTonePortaTarget();
// 			}
			setActive( m_period != 0 );
			if( ( m_vibratoWaveform & 4 ) == 0 ) {
				// reset phase to 0 on a new note
				m_vibratoPhase = 0;
			}
			if( ( m_tremoloWaveform & 4 ) == 0 ) {
				m_tremoloPhase = 0;
			}
		}
		setActive( isActive() && m_period != 0 && currentSample() );
	} // endif(tick==0)
	//if(!isActive()) {
	//return;
	//}
	switch( m_currentCell->effect() ) {
		case 0x00:
			fxArpeggio( m_currentCell->effectValue() );
			break;
		case 0x01:
			fxPortaUp( m_currentCell->effectValue() );
			break;
		case 0x02:
			fxPortaDown( m_currentCell->effectValue() );
			break;
		case 0x03:
			fxPorta( m_currentCell->effectValue() );
			break;
		case 0x04:
			fxVibrato( m_currentCell->effectValue() );
			break;
		case 0x05:
			fxPortaVolSlide( m_currentCell->effectValue() );
			break;
		case 0x06:
			fxVibVolSlide( m_currentCell->effectValue() );
			break;
		case 0x07:
			fxTremolo( m_currentCell->effectValue() );
			break;
		case 0x08:
			fxSetFinePan( m_currentCell->effectValue() );
			break;
		case 0x09:
			fxOffset( m_currentCell->effectValue() );
			break;
		case 0x0a:
			fxVolSlide( m_currentCell->effectValue() );
			break;
		case 0x0b:
			fxPosJmp( m_currentCell->effectValue() );
			break;
		case 0x0c:
			fxSetVolume( m_currentCell->effectValue() );
			break;
		case 0x0d:
			fxPatBreak( m_currentCell->effectValue() );
			break;
		case 0x0e:
			switch( highNibble( m_currentCell->effectValue() ) ) {
				case 0x01:
					efxFineSlideUp( m_currentCell->effectValue() );
					break;
				case 0x02:
					efxFineSlideDown( m_currentCell->effectValue() );
					break;
				case 0x03:
					efxGlissando( m_currentCell->effectValue() );
					break;
				case 0x04:
					efxSetVibWaveform( m_currentCell->effectValue() );
					break;
				case 0x05:
					efxSetFinetune( m_currentCell->effectValue() );
					break;
				case 0x06:
					efxPatLoop( m_currentCell->effectValue() );
					break;
				case 0x07:
					efxSetTremoloWaveform( m_currentCell->effectValue() );
					break;
				case 0x08:
					efxSetPanning( m_currentCell->effectValue() );
					break;
				case 0x09:
					efxRetrigger( m_currentCell->effectValue() );
					break;
				case 0x0a:
					efxFineVolSlideUp( m_currentCell->effectValue() );
					break;
				case 0x0b:
					efxFineVolSlideDown( m_currentCell->effectValue() );
					break;
				case 0x0c:
					efxNoteCut( m_currentCell->effectValue() );
					break;
				case 0x0d:
					efxNoteDelay( m_currentCell->effectValue() );
					break;
				case 0x0e:
					efxPatDelay( m_currentCell->effectValue() );
					break;
			}
			break;
		case 0x0f:
			fxSetSpeed( m_currentCell->effectValue() );
			break;
	}
	m_period = clip<uint16_t>( m_period, 0x71, 0x358 );
	updateStatus();
}

const ModSample* ModChannel::currentSample() const
{
	return m_module->sampleAt( m_sampleIndex );
}

std::string ModChannel::internal_cellString() const
{
	return m_currentCell->trackerString();
}

std::string ModChannel::internal_effectDescription() const
{
	return m_effectDescription;
}

std::string ModChannel::internal_effectName() const
{
	if( m_currentCell->effect() == 0 && m_currentCell->effectValue() == 0 ) {
		return "---";
	}
	return stringFmt( "%X%02X", int(m_currentCell->effect()), int(m_currentCell->effectValue()) );
}

std::string ModChannel::internal_noteName() const
{
	uint8_t idx = periodToNoteIndex( m_period );
	if( idx == 255 ) {
		return "^^^";
	}
	else {
		return stringFmt( "%s%u", NoteNames[idx % 12], idx / 12 );
	}
}

IArchive& ModChannel::serialize( IArchive* data )
{
	GenChannel::serialize( data )
	.archive( m_currentCell )
	% m_volume
	% m_physVolume
	% m_finetune
	% m_tremoloWaveform
	% m_tremoloPhase
	% m_vibratoWaveform
	% m_vibratoPhase
	% m_glissando
	% m_period
	% m_physPeriod
	% m_portaTarget
	% m_lastVibratoFx
	% m_lastTremoloFx
	% m_portaSpeed
	% m_lastOffsetFx
	% m_sampleIndex;
	return data->archive( &m_bresen );
}

void ModChannel::internal_mixTick( MixerFrameBuffer* mixBuffer )
{
	if( !isActive() || !currentSample() || m_physPeriod == 0 ) {
		return setActive( false );
	}
	if( mixBuffer && mixBuffer->get()->empty() ) {
		logger()->error( L4CXX_LOCATION, "mixBuffer is empty" );
		return;
	}
	if( mixBuffer && m_module->frequency() * mixBuffer->get()->size() == 0 ) {
		setActive( false );
		return;
	}
	m_bresen.reset( m_module->frequency(), FrequencyBase / m_physPeriod );
	// TODO glissando
	const ModSample* currSmp = currentSample();
	GenSample::PositionType pos = position();
	if( pos == GenSample::EndOfSample ) {
		setActive( false );
		return;
	}
	if( mixBuffer ) {
		for( MixerSampleFrame & frame : **mixBuffer ) {
			// TODO panning
			BasicSampleFrame sample = currSmp->sampleAt( pos );
			sample.mulRShift( m_physVolume, 6 );
			frame += sample;
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
}

void ModChannel::setCellPeriod()
{
	if( m_currentCell->period() == 0 ) {
		return;
	}
	size_t perIdx = 0;
	for( perIdx = 0; perIdx < fullPeriods.front().size(); perIdx++ ) {
		if( m_currentCell->period() >= fullPeriods.front().at( perIdx ) ) {
			break;
		}
	}
	if( perIdx >= fullPeriods.front().size() ) {
		m_period = fullPeriods.at( m_finetune ).back();
	}
	else {
		m_period = fullPeriods.at( m_finetune )[ perIdx ];
	}
// 	if( m_currentCell.effect() == 0x0e && highNibble( m_currentCell.effectValue() ) == 0x0d ) {
		// note delay
// 		return;
// 	}
	m_physPeriod = m_period;
	if( ( m_vibratoWaveform & 4 ) == 0 ) {
		m_vibratoPhase = 0;
	}
	if( ( m_tremoloWaveform & 4 ) == 0 ) {
		m_vibratoPhase = 0;
	}
	// mt_CheckMoreEfx
}

void ModChannel::setTonePortaTarget()
{

	if( m_currentCell->period() == 0 ) {
		return;
	}
	size_t perIdx = 0;
	for( perIdx = 0; perIdx < fullPeriods.at( m_finetune ).size(); perIdx++ ) {
		if( m_currentCell->period() >= fullPeriods.at( m_finetune ).at( perIdx ) ) {
			break;
		}
	}
	if( perIdx != 0 && ( m_finetune & 8 ) != 0 ) {
		perIdx--;
	}
	if( perIdx >= fullPeriods.at( m_finetune ).size() ) {
		m_portaTarget = fullPeriods.at( m_finetune ).back();
	}
	else {
		m_portaTarget = fullPeriods.at( m_finetune ).at( perIdx );
	}
	m_portaDirUp = 0;
	if( m_portaTarget == m_period ) {
		m_portaTarget = 0;
	}
	else if( m_period > m_portaTarget ) {
		m_portaDirUp = 1;
	}
}

void ModChannel::fxSetSpeed( uint8_t fxByte )
{
	m_effectDescription = "Tempo\x7f";
	if( fxByte == 0 ) {
		return;
	}
	else if( fxByte <= 0x1f ) {
		m_module->setSpeed( fxByte );
	}
	else {
		m_module->setTempo( fxByte );
	}
}

void ModChannel::efxNoteCut( uint8_t fxByte )
{
	m_effectDescription = "NCut \xd4";
	fxByte = lowNibble( fxByte );
	if( fxByte == m_module->state().tick ) {
		setActive( false );
		m_volume = 0;
	}
}

void ModChannel::efxFineVolSlideDown( uint8_t fxByte )
{
	m_effectDescription = "VSld \x19";
	if( m_module->state().tick != 0 ) {
		return;
	}
	fxByte = lowNibble( fxByte );
	m_physVolume = m_volume = std::max<int>( 0, m_volume - fxByte );
}

void ModChannel::efxFineVolSlideUp( uint8_t fxByte )
{
	m_effectDescription = "VSld \x18";
	if( m_module->state().tick != 0 ) {
		return;
	}
	fxByte = lowNibble( fxByte );
	m_physVolume = m_volume = std::min<int>( 0x40, m_volume + fxByte );
}

void ModChannel::efxSetFinetune( uint8_t fxByte )
{
	m_effectDescription = "FTune\xe6";
	m_finetune = lowNibble( fxByte );
	setCellPeriod();
}

void ModChannel::efxSetTremoloWaveform( uint8_t fxByte )
{
	m_effectDescription = "TWave\x9f";
	m_tremoloWaveform = fxByte & 0x7;
}

void ModChannel::efxSetVibWaveform( uint8_t fxByte )
{
	m_effectDescription = "VWave\x9f";
	m_vibratoWaveform = lowNibble( fxByte );
}

void ModChannel::efxGlissando( uint8_t fxByte )
{
	m_effectDescription = "Gliss\xcd";
	m_glissando = lowNibble( fxByte ) != 0;
}

void ModChannel::efxFineSlideDown( uint8_t fxByte )
{
	m_effectDescription = "Ptch \x1f";
	if( m_module->state().tick != 0 ) {
		return;
	}
	m_lowMask = 0x0f;
	fxPortaDown( fxByte );
	m_effectDescription = "Ptch \x1f";
}

void ModChannel::efxFineSlideUp( uint8_t fxByte )
{
	m_effectDescription = "Ptch \x1e";
	if( m_module->state().tick != 0 ) {
		return;
	}
	m_lowMask = 0x0f;
	fxPortaUp( fxByte );
	m_effectDescription = "Ptch \x1e";
}

void ModChannel::fxOffset( uint8_t fxByte )
{
	m_effectDescription = "Offs \xaa";
	reuseIfZero( m_lastOffsetFx, fxByte );
	if( m_module->state().tick != 0 || m_currentCell->period() == 0 ) {
		return;
	}
	if( currentSample() && currentSample()->length() > ( fxByte << 8 ) ) {
		setPosition( fxByte << 8 );
	}
}

void ModChannel::fxSetVolume( uint8_t fxByte )
{
	m_effectDescription = "StVol=";
	m_physVolume = m_volume = std::min<uint8_t>( 0x40, fxByte );
}

void ModChannel::fxVolSlide( uint8_t fxByte )
{
	if( highNibble( fxByte ) == 0 ) {
		// vol slide down
		m_effectDescription = "VSld \x1f";
		m_physVolume = m_volume = std::max<int>( 0x0, m_volume - lowNibble( fxByte ) );
	}
	else {
		m_effectDescription = "VSld \x1e";
		m_physVolume = m_volume = std::min( 0x40, m_volume + highNibble( fxByte ) );
	}
}

void ModChannel::fxVibVolSlide( uint8_t fxByte )
{
	fxVibrato( 0 );
	fxVolSlide( fxByte );
	m_effectDescription = "VibVo\xf7";
}

void ModChannel::fxPortaVolSlide( uint8_t fxByte )
{
	fxPorta( 0 );
	fxVolSlide( fxByte );
	m_effectDescription = "PrtVo\x12";
}

void ModChannel::fxPortaDown( uint8_t fxByte )
{
	m_effectDescription = "Ptch \x1f";
	m_period += fxByte & m_lowMask;
	m_lowMask = 0xff;
	if( m_period > 856 ) {
		m_period = 856;
	}
	m_physPeriod = m_portaTarget = m_period;
	applyGlissando();
}

void ModChannel::fxPortaUp( uint8_t fxByte )
{
	m_effectDescription = "Ptch \x1e";
	m_period -= fxByte & m_lowMask;
	m_lowMask = 0xff;
	if( m_period < 113 ) {
		m_period = 113;
	}
	m_physPeriod = m_portaTarget = m_period;
	applyGlissando();
}

void ModChannel::fxVibrato( uint8_t fxByte )
{
	m_effectDescription = "Vibr \xf7";
	reuseNibblesIfZero( m_lastVibratoFx, fxByte );
	int16_t vibVal = 0;
	switch( m_vibratoWaveform & 3 ) {
		case 0: // sine
			vibVal = WaveSine[ m_vibratoPhase & 0x1f ];
			break;
		case 1: // ramp
			if( ( m_vibratoPhase & 0x20 ) != 0 ) {
				vibVal = 255 - ( m_vibratoPhase << 3 );
			}
			else {
				vibVal = ( m_vibratoPhase << 3 );
			}
			break;
		default: // square
			vibVal = 255;
			break;
	}
	vibVal = ( lowNibble( fxByte ) * vibVal ) >> 7;
	if( ( m_vibratoPhase & 0x20 ) != 0 ) {
		vibVal = -vibVal;
	}
	m_physPeriod = m_period + vibVal;
	m_vibratoPhase += highNibble( fxByte );
	m_vibratoPhase &= 0x3f;
}

void ModChannel::fxPorta( uint8_t fxByte )
{
	m_effectDescription = "Porta\x12";
	reuseIfZero( m_portaSpeed, fxByte );
	if( m_portaTarget == 0 ) {
		setTonePortaTarget();
		return;
	}
	if( m_portaDirUp == 1 ) {
		m_period = std::max<int>( m_portaTarget, m_period - fxByte );
	}
	else {
		m_period = std::min<int>( m_portaTarget, m_period + fxByte );
	}
	if( m_period == m_portaTarget ) {
		m_portaTarget = 0;
	}
	applyGlissando();
}

void ModChannel::internal_updateStatus()
{
	if( !isActive() ) {
		setStatusString( "" );
		return;
	}
	std::string volStr = stringFmt( "%3d%%", clip<int>( m_volume, 0, 0x40 ) * 100 / 0x40 );
	setStatusString( stringFmt(
		"%02X: %s%s %s %s P:%s V:%s %s",
		int(m_sampleIndex),
		" ", //(m_noteChanged ? "*" : " "),
		noteName(),
		effectName(),
		m_effectDescription,
		"-----",
		volStr,
		currentSample() ? currentSample()->title() : ""
		)
	);
}

void ModChannel::fxArpeggio( uint8_t fxByte )
{
	if( fxByte == 0 ) {
		m_effectDescription = "      ";
		return;
	}
	m_effectDescription = "Arp  \xf0";
	if( ( m_module->state().tick % 3 ) == 0 ) {
		m_physPeriod = m_period;
		return;
	}
	uint8_t delta = 0;
	if( ( m_module->state().tick % 3 ) == 1 ) {
		delta = highNibble( fxByte );
	}
	else {
		delta = lowNibble( fxByte );
	}
	for( uint_fast8_t i = 0; i < fullPeriods.at( m_finetune ).size() - delta; i++ ) {
		if( fullPeriods.at( m_finetune )[ i ] <= m_period ) {
			m_physPeriod = fullPeriods.at( m_finetune )[ i + delta ];
			return;
		}
	}
	m_physPeriod = fullPeriods.at( m_finetune ).back();
}

void ModChannel::fxPatBreak( uint8_t )
{
	m_effectDescription = "PBrk \xf6";
	// implemented in ModModule
}

void ModChannel::fxPosJmp( uint8_t )
{
	m_effectDescription = "JmOrd\x1a";
	// implemented in ModModule
}

void ModChannel::fxSetFinePan( uint8_t fxByte )
{
	m_effectDescription = "StPan\x1d";
	logger()->warn( L4CXX_LOCATION, "Not implemented: Effect Fine Panning" );
	// TODO
}

void ModChannel::fxTremolo( uint8_t fxByte )
{
	m_effectDescription = "Tremo\xec";
	reuseNibblesIfZero( m_lastTremoloFx, fxByte );
	int16_t vibVal = 0;
	switch( m_tremoloWaveform & 3 ) {
		case 0: // sine
			vibVal = WaveSine[ m_tremoloPhase & 0x1f ];
			break;
		case 1: // ramp
			if( ( m_vibratoPhase & 0x20 ) != 0 ) {
				vibVal = 255 - ( m_tremoloPhase << 3 );
			}
			else {
				vibVal = ( m_tremoloPhase << 3 );
			}
			break;
		default: // square
			vibVal = 255;
			break;
	}
	vibVal = ( lowNibble( fxByte ) * vibVal ) >> 6;
	if( ( m_tremoloPhase & 0x20 ) != 0 ) {
		vibVal = -vibVal;
	}
	m_physVolume = clip<int>( m_volume + vibVal, 0, 0x40 );
	m_tremoloPhase += highNibble( fxByte );
	m_tremoloPhase &= 0x3f;
}

void ModChannel::efxPatLoop( uint8_t fxByte )
{
	m_effectDescription = "PLoop\xe8";
	//logger()->warn( L4CXX_LOCATION, "Not implemented: Effect Pattern Loop" );
	// TODO
}

void ModChannel::efxNoteDelay( uint8_t /*fxByte*/ )
{
	m_effectDescription = "Delay\xc2";
}

void ModChannel::efxSetPanning( uint8_t fxByte )
{
	m_effectDescription = "StPan\x1d";
	logger()->warn( L4CXX_LOCATION, "Not implemented: Effect Set Panning" );
	// TODO
}

void ModChannel::efxPatDelay( uint8_t fxByte )
{
	// TODO
	logger()->warn( L4CXX_LOCATION, "Not implemented: Effect Pattern Delay" );
}

void ModChannel::efxRetrigger( uint8_t fxByte )
{
	if( lowNibble( fxByte ) == 0 || m_currentCell->period() == 0 ) {
		return;
	}
	if( m_module->state().tick % lowNibble( fxByte ) != 0 ) {
		return;
	}
	setPosition( 0 );
}

void ModChannel::applyGlissando()
{
	if( !m_glissando ) {
		m_physPeriod = m_period;
		return;
	}
	for( auto val : fullPeriods.at( m_finetune ) ) {
		if( m_period >= val ) {
			m_physPeriod = val;
			return;
		}
	}
	m_physPeriod = fullPeriods.at( m_finetune ).back();
}

light4cxx::Logger::Ptr ModChannel::logger()
{
	return light4cxx::Logger::get( GenChannel::logger()->name() + ".mod" );
}

}
}
