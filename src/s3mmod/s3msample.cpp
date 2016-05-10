/*
    PPPlay - an old-fashioned module player
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

/**
 * @ingroup S3mMod
 * @{
 */

#include <boost/exception/all.hpp>

#include "s3msample.h"
#include "stream/stream.h"

namespace ppp
{
namespace s3m
{
#ifndef DOXYGEN_SHOULD_SKIP_THIS
enum : uint8_t
{
    s3mFlagSmpLooped = 0x01, //!< @brief Sample is looped
    s3mFlagSmpStereo = 0x02, //!< @brief Sample is stereo
    s3mFlagSmp16bit = 0x04  //!< @brief Sample has 16-bit samples
};

#pragma pack(push,1)
struct S3mSampleHeader
{
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
#endif

S3mSample::S3mSample() : Sample(), m_highQuality(false)
{
}

bool S3mSample::load(Stream* str, size_t pos, bool imagoLoopEnd)
{
    try
    {
        str->seek(pos);
        S3mSampleHeader smpHdr;
        *str >> smpHdr;
        if((smpHdr.length == 0) || ((smpHdr.memSeg[0] == 0) && (smpHdr.memSeg[1] == 0) && (smpHdr.memSeg[2] == 0)))
            return true;
        if(!std::equal(smpHdr.ID, smpHdr.ID + 4, "SCRS"))
        {
            logger()->warn(L4CXX_LOCATION, "Sample ID not 'SCRS', assuming empty.");
            return true;
        }
        if(smpHdr.pack != 0)
        {
            logger()->error(L4CXX_LOCATION, "DP30ADPCM packed sample, not supported.");
            return false;
        }
        if(smpHdr.type != 1)
        {
            logger()->warn(L4CXX_LOCATION, "Sample Type not 0x01 (is %#.2x), assuming empty.", int(smpHdr.type));
            return true;
        }
        /// @warning This could be a much too high value...
        resizeData((smpHdr.hiLength << 16) | smpHdr.length);
        //	aLength = (aLength>64000) ? 64000 : aLength;
        setLoopStart((smpHdr.hiLoopStart << 16) | smpHdr.loopStart);
        //	aLoopStart = (aLoopStart>64000) ? 64000 : aLoopStart;
        if(!imagoLoopEnd)
            setLoopEnd((smpHdr.hiLoopEnd << 16) | smpHdr.loopEnd);
        else
            setLoopEnd(((smpHdr.hiLoopEnd << 16) | smpHdr.loopEnd) + 1);
        //	aLoopEnd = (aLoopEnd>64000) ? 64000 : aLoopEnd;
        setVolume(smpHdr.volume);
        setFrequency(smpHdr.c2spd);
        bool loadStereo = (smpHdr.flags & static_cast<uint8_t>(s3mFlagSmpStereo)) != 0;
        if(smpHdr.hiLoopStart == smpHdr.hiLoopEnd && smpHdr.loopStart == smpHdr.loopEnd)
        {
            setLoopType(Sample::LoopType::None);
        }
        else
        {
            setLoopType((smpHdr.flags & static_cast<uint8_t>(s3mFlagSmpLooped)) == 0 ? Sample::LoopType::None : Sample::LoopType::Forward);
        }
        setTitle(stringncpy(smpHdr.sampleName, 28));
        setFilename(stringncpy(smpHdr.filename, 12));
        // ok, header loaded, now load the sample data
        str->seek(((smpHdr.memSeg[0] << 16) | (smpHdr.memSeg[2] << 8) | smpHdr.memSeg[1]) * 16);
        BOOST_ASSERT(length() != 0);
        if(!str->good())
        {
            setFrequency(0);
            logger()->warn(L4CXX_LOCATION, "Seek failed or length is zero, assuming empty.");
            return true;
        }
        if(smpHdr.flags & static_cast<uint8_t>(s3mFlagSmp16bit))
        {
            logger()->info(L4CXX_LOCATION, "Loading 16-bit sample");
            m_highQuality = true;
            uint16_t smp16;
            auto smpPtr = beginIterator();
            for(std::streamoff i = 0; i < length(); i++)
            {
                if(!(*str >> smp16))
                {
                    logger()->warn(L4CXX_LOCATION, "EOF reached before Sample Data read completely, assuming zeroes.");
                    return true;
                }
                smpPtr->left = smpPtr->right = clip(smp16 - 32768, -32767, 32767);   // negating -32768 fails otherwise in surround mode
                ++smpPtr;
            }
            if(loadStereo)
            {
                logger()->info(L4CXX_LOCATION, "Loading Stereo...");
                smpPtr = beginIterator();
                for(std::streamoff i = 0; i < length(); i++)
                {
                    if(!(*str >> smp16))
                    {
                        logger()->warn(L4CXX_LOCATION, "EOF reached before Sample Data read completely, assuming zeroes.");
                        return true;
                    }
                    smpPtr->right = clip(smp16 - 32768, -32767, 32767);   // negating -32768 fails otherwise in surround mode
                    ++smpPtr;
                }
            }
        }
        else
        { // convert 8-bit samples to 16-bit ones
            logger()->info(L4CXX_LOCATION, "Loading 8-bit sample");
            uint8_t smp8;
            auto smpPtr = beginIterator();
            for(std::streamoff i = 0; i < length(); i++)
            {
                if(!(*str >> smp8))
                {
                    logger()->warn(L4CXX_LOCATION, "EOF reached before Sample Data read completely, assuming zeroes.");
                    return true;
                }
                smpPtr->left = smpPtr->right = clip((smp8 - 128) << 8, -32767, 32767);   // negating -32768 fails otherwise in surround mode
                ++smpPtr;
            }
            if(loadStereo)
            {
                logger()->info(L4CXX_LOCATION, "Loading Stereo...");
                smpPtr = beginIterator();
                for(std::streamoff i = 0; i < length(); i++)
                {
                    if(!(*str >> smp8))
                    {
                        logger()->warn(L4CXX_LOCATION, "EOF reached before Sample Data read completely, assuming zeroes.");
                        return true;
                    }
                    smpPtr->right = clip((smp8 - 128) << 8, -32767, 32767);   // negating -32768 fails otherwise in surround mode
                    ++smpPtr;
                }
            }
        }
        return true;
    }
    catch(...)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(boost::current_exception_diagnostic_information()));
    }
}

bool S3mSample::isHighQuality() const
{
    return m_highQuality;
}

light4cxx::Logger* S3mSample::logger()
{
    return light4cxx::Logger::get(Sample::logger()->name() + ".s3m");
}
}
}

/**
 * @}
 */