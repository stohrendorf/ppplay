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

#include "phaser.h"

/**
 * @file
 * @ingroup Common
 * @brief The phaser class (implementation)
 */

namespace ppp {
	Phaser::Phaser() throw() : m_lookup(), m_amplitude( 0 ), m_phase( 0 ) {
	}

	Phaser::Phaser( const int16_t table[], uint32_t length, int16_t amp, float multiplier ) throw() : m_lookup(), m_amplitude( amp ), m_phase( 0 ) {
		resetWave( table, length, amp, multiplier );
	}

	Phaser& Phaser::operator+=( int16_t delta ) throw( PppException ) {
		PPP_TEST( m_lookup.size() == 0 );
		m_phase += delta;
		m_phase %= m_lookup.size();
		return *this;
	}

	Phaser& Phaser::operator++() throw( PppException ) {
		PPP_TEST( m_lookup.size() == 0 );
		m_phase++;
		m_phase %= m_lookup.size();
		return *this;
	}

	int16_t& Phaser::operator[]( uint32_t index ) throw( PppException ) {
		//PPP_TEST( !m_lookup );
		PPP_TEST( m_lookup.size() == 0 );
		index %= m_lookup.size();
		return m_lookup[index];
	}

	uint32_t Phaser::getLength() const throw() {
		return m_lookup.size();
	}

	int16_t Phaser::getAmplitude() const throw() {
		return m_amplitude;
	}

	int32_t Phaser::getPhase() const throw() {
		return m_phase;
	}

	void Phaser::resetPhase() throw() {
		m_phase = 0;
	}

	void Phaser::resetWave( const int16_t table[], uint32_t length, int16_t amp, float multiplier ) throw( PppException ) {
		PPP_TEST( length == 0 );
		//m_lookup.reset( new int16_t[length] );
		m_lookup.resize( length );
		for( uint32_t i = 0; i < length; i++ ) {
			m_lookup[i] = static_cast<int16_t>( table[i] * multiplier );
		}
		//m_length = length;
		m_amplitude = amp;
	}

	int16_t Phaser::get() const throw( PppException ) {
		//PPP_TEST( !m_lookup );
		return m_lookup[m_phase];
	}

	float Phaser::getf() const throw( PppException ) {
		PPP_TEST( m_amplitude == 0 );
		return static_cast<float>( get() ) / m_amplitude;
	}

	IArchive& Phaser::serialize( IArchive* data ) {
		if( data->isSaving() ) {
			std::size_t s = m_lookup.size();
			*data& s& m_amplitude& m_phase;
			return data->array( &m_lookup.front(), m_lookup.size() );
		}
		else {
			std::size_t len;
			*data& len& m_amplitude& m_phase;
			m_lookup.resize( len );
			return data->array( &m_lookup.front(), m_lookup.size() );
		}
	}
}
