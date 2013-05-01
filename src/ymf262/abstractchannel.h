/*
 * PeePeePlayer - an old-fashioned module player
 * Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Original Java Code: Copyright (C) 2008 Robson Cozendey <robson@cozendey.com>
 * 
 * Some code based on forum posts in: http://forums.submarine.org.uk/phpBB/viewforum.php?f=9,
 * Copyright (C) 2010-2013 by carbon14 and opl3
 */

#ifndef PPP_OPL_ABSTRACTCHANNEL_H
#define PPP_OPL_ABSTRACTCHANNEL_H

#include "stuff/utils.h"

#include <light4cxx/logger.h>
#include <stream/iserializable.h>
#include <vector>
#include <cstdint>
#include <memory>
#include <array>

namespace opl
{

class Opl3;

class AbstractChannel : public ISerializable
{
	DISABLE_COPY( AbstractChannel )
public:
	typedef std::shared_ptr<AbstractChannel> Ptr;
	
	static constexpr int _2_KON1_BLOCK3_FNUMH2_Offset = 0xB0;
	static constexpr int  FNUML8_Offset = 0xA0;
	static constexpr int  CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset = 0xC0;
private:
	Opl3* m_opl;
	int m_channelBaseAddress;

	//! @brief Frequency Number, 0..1023
	uint16_t m_fnum;
	//! @brief Key On. If changed, calls Channel.keyOn() / keyOff().
	bool m_kon;
	//! @brief Block/octave (0..7)
	uint8_t m_block;
	bool m_cha;
	bool m_chb;
	bool m_chc;
	bool m_chd;
	uint8_t m_fb;
	int16_t m_feedback[2];
	bool m_cnt;
	
	static light4cxx::Logger* logger();

public:
	bool cnt() const {
		return m_cnt;
	}
	Opl3* opl() const {
		return m_opl;
	}
	uint16_t fnum() const {
		return m_fnum;
	}
	uint8_t block() const {
		return m_block;
	}
	/**
	 * @brief Calculate adjusted phase feedback
	 * @return Adjusted phase feedback using m_fb (11 bit)
	 */
	int16_t avgFeedback() const {
		if( m_fb == 0) {
			return 0;
		}
		return (( m_feedback[0] + m_feedback[1] ) << m_fb)>>9;
	}
	void clearFeedback() {
		m_feedback[0] = m_feedback[1] = 0;
	}
	/**
	 * @brief Push feedback into the queue
	 * @param[in] fb 13 bit feedback from channel output
	 */
	void pushFeedback( int16_t fb ) {
		m_feedback[0] = m_feedback[1];
		m_feedback[1] = fb;
	}

	AbstractChannel( Opl3* opl, int baseAddress );
	virtual ~AbstractChannel() {}

	/**
	 * @post m_fnumh<8 && m_block<8
	 */
	void update_2_KON1_BLOCK3_FNUMH2();

	void update_FNUML8();

	/**
	 * @post m_fb<8
	 */
	void update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1();

	void updateChannel();

	void getInFourChannels(std::array<int16_t,4>* dest, int16_t channelOutput );

	virtual void nextSample(std::array<int16_t,4>* dest) = 0;
	virtual void keyOn() = 0;
	virtual void keyOff() = 0;
	virtual void updateOperators() = 0;

	int baseAddress() const {
		return m_channelBaseAddress;
	}
	
    virtual AbstractArchive& serialize(AbstractArchive* archive) = 0;
};
}

#endif
