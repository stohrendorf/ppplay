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

#include "s3mbase.h"
#include "s3msample.h"
#include <algorithm>

using namespace ppp;
using namespace ppp::s3m;

/**
* @file
* @brief S3M Sample class
* @ingroup S3mMod
*/

/**
* @brief Flags for s3mSampleHeader::flags
* @ingroup S3mMod
*/
enum : uint8_t {
	s3mFlagSmpLooped    = 0x01, //!< @brief Sample is looped
	s3mFlagSmpStereo    = 0x02, //!< @brief Sample is stereo
	s3mFlagSmp16bit     = 0x04  //!< @brief Sample has 16-bit samples
};

#pragma pack(push,1)
/**
 * @typedef S3mSampleHeader
 * @ingroup S3mMod
 * @brief S3M Sample Header
 */
struct S3mSampleHeader {
	uint8_t type;         //!< @brief Sample type, only type 1 supported
	char filename[12];    //!< @brief DOS filename, no ending @c NUL character
	uint8_t memSeg[3];    //!< @brief Parapointer to sample data
	uint16_t length;      //!< @brief Sample length in bytes
	uint16_t hiLength;    //!< @brief Sample length in bytes, high nibble
	uint16_t loopStart;   //!< @brief Loop start offset in bytes
	uint16_t hiLoopStart; //!< @brief Loop start offset in bytes, high nibble
	uint16_t loopEnd;     //!< @brief Loop end offset in bytes
	uint16_t hiLoopEnd;   //!< @brief Loop end offset in bytes, high nibble
	uint8_t volume;       //!< @brief Sample volume
	uint8_t rsvd1;        //!< @brief Reserved
	uint8_t pack;         //!< @brief 1 if DP30ADPCM packed @warning Not supported
	uint8_t flags;        //!< @brief Sample flags
	uint16_t c2spd;       //!< @brief Base frequency
	uint16_t hiC2spd;     //!< @brief Base frequency, high nibble
	uint8_t rsvd2[12];    //!< @brief Reserved
	char sampleName[28];  //!< @brief Sample title, including @c NUL character
	char ID[4];           //!< @brief @c 'SCRS'
};
#pragma pack(pop)

S3mSample::S3mSample() throw() : GenSample(), m_highQuality( false ) {
}

S3mSample::~S3mSample() throw() {
}

bool S3mSample::load( BinStream& str, const std::size_t pos, bool imagoLoopEnd ) throw( PppException ) {
	try {
		PPP_TEST( getDataL() != NULL || getDataR() != NULL );
		str.seek( pos );
		S3mSampleHeader smpHdr;
		str.read( reinterpret_cast<char*>( &smpHdr ), sizeof( smpHdr ) );
// 		str.read((char*)&smpHdr, sizeof(s3mSampleHeader));
		if( ( smpHdr.length == 0 ) || ( ( smpHdr.memSeg[0] == 0 ) && ( smpHdr.memSeg[1] == 0 ) && ( smpHdr.memSeg[2] == 0 ) ) )
			return true;
		//if (memcmp(smpHdr.ID, "SCRS", 4)) {
		if( !std::equal( smpHdr.ID, smpHdr.ID + 4, "SCRS" ) ) {
			LOG_WARNING( "Sample ID not 'SCRS', assuming empty." );
			return true;
		}
		if( smpHdr.pack != 0 ) {
			LOG_ERROR( "Packed sample, not supported." );
			return false;
		}
		if( smpHdr.type != 1 ) {
			LOG_WARNING( "Sample Type not 0x01 (is 0x%.2x), assuming empty.", smpHdr.type );
			return true;
		}
		/// @warning This could be a much too high value...
		setLength( ( smpHdr.hiLength << 16 ) | smpHdr.length );
		//	aLength = (aLength>64000) ? 64000 : aLength;
		setLoopStart( ( smpHdr.hiLoopStart << 16 ) | smpHdr.loopStart );
		//	aLoopStart = (aLoopStart>64000) ? 64000 : aLoopStart;
		if( !imagoLoopEnd )
			setLoopEnd( ( smpHdr.hiLoopEnd << 16 ) | smpHdr.loopEnd );
		else
			setLoopEnd( (( smpHdr.hiLoopEnd << 16 ) | smpHdr.loopEnd) + 1 );
		//	aLoopEnd = (aLoopEnd>64000) ? 64000 : aLoopEnd;
		setVolume( smpHdr.volume );
		setBaseFrq( smpHdr.c2spd );
		bool loadStereo = ( smpHdr.flags & s3mFlagSmpStereo ) != 0;
		setLoopType( ( smpHdr.flags & s3mFlagSmpLooped ) == 0 ? GenSample::LoopType::None : GenSample::LoopType::Forward );
		setTitle( stringncpy( smpHdr.sampleName, 28 ) );
		setFilename( stringncpy( smpHdr.filename, 12 ) );
		// ok, header loaded, now load the sample data
		str.seek( ( ( smpHdr.memSeg[0] << 16 ) | ( smpHdr.memSeg[2] << 8 ) | smpHdr.memSeg[1] ) * 16 );
		PPP_TEST( getLength() == 0 );
		if( str.fail() ) {
			setBaseFrq( 0 );
			LOG_WARNING( "Seek failed or length is zero, assuming empty." );
			return true;
		}
		if( loadStereo ) {
			setDataL( new int16_t[getLength()] );
			std::fill_n( getNonConstDataL(), getLength(), 0 );
			setDataR( new int16_t[getLength()] );
			std::fill_n( getNonConstDataR(), getLength(), 0 );
		}
		else {
			setDataMono( new int16_t[getLength()] );
			std::fill_n( getNonConstDataR(), getLength(), 0 );
		}
		if( smpHdr.flags & s3mFlagSmp16bit ) {
			LOG_MESSAGE( "Loading 16-bit sample" );
			m_highQuality = true;
			uint16_t smp16;
			BasicSample* smpPtr = getNonConstDataL();
			for( uint32_t i = 0; i < getLength(); i++ ) {
				str.read( &smp16 );
				if( str.fail() ) {
					LOG_WARNING( "EOF reached before Sample Data read completely, assuming zeroes." );
					return true;
				}
				*( smpPtr++ ) = clip( smp16 - 32768, -32767, 32767 ); // negating -32768 fails otherwise in surround mode
			}
			if( loadStereo ) {
				LOG_MESSAGE( "Loading Stereo..." );
				smpPtr = getNonConstDataR();
				for( uint32_t i = 0; i < getLength(); i++ ) {
					str.read( &smp16 );
					if( str.fail() ) {
						LOG_WARNING( "EOF reached before Sample Data read completely, assuming zeroes." );
						return true;
					}
					*( smpPtr++ ) = clip( smp16 - 32768, -32767, 32767 ); // negating -32768 fails otherwise in surround mode
				}
			}
		}
		else { // convert 8-bit samples to 16-bit ones
			LOG_MESSAGE( "Loading 8-bit sample" );
			uint8_t smp8;
			BasicSample* smpPtr = getNonConstDataL();
			for( uint32_t i = 0; i < getLength(); i++ ) {
				str.read( &smp8 );
				if( str.fail() ) {
					LOG_WARNING( "EOF reached before Sample Data read completely, assuming zeroes." );
					return true;
				}
				*( smpPtr++ ) = clip( ( smp8 - 128 ) << 8, -32767, 32767 ); // negating -32768 fails otherwise in surround mode
			}
			if( loadStereo ) {
				LOG_MESSAGE( "Loading Stereo..." );
				smpPtr = getNonConstDataR();
				for( uint32_t i = 0; i < getLength(); i++ ) {
					str.read( &smp8 );
					if( str.fail() ) {
						LOG_WARNING( "EOF reached before Sample Data read completely, assuming zeroes." );
						return true;
					}
					*( smpPtr++ ) = clip( ( smp8 - 128 ) << 8, -32767, 32767 ); // negating -32768 fails otherwise in surround mode
				}
			}
		}
		return true;
	}
	PPP_CATCH_ALL();
}
