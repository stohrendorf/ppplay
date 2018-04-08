#pragma once

#include "genmod/sample.h"
#include "stream/stream.h"

#include <cstdint>
#include <type_traits>

namespace ppp
{
namespace it
{
#pragma pack(push, 1)
struct ItSampleHeader
{
    char id[4] = {'I', 'M', 'P', 'S'};
    char filename[12] = "";
    uint8_t null = 0;
    uint8_t gvl = 0; //!< 0..64
    uint8_t flg = 0;
    uint8_t vol = 0; //!< 0..64
    char name[26] = "";
    uint8_t cvt = 0;
    uint8_t dfp = 0;
    uint32_t length = 0;
    uint32_t loopBeg = 0;
    uint32_t loopEnd = 0;
    uint32_t c5speed = 8363;
    uint32_t susLoopBeg = 0;
    uint32_t susLoopEnd = 0;
    uint32_t samplePointer = 0;
    uint8_t vis = 0;
    uint8_t vid = 0;
    uint8_t vir = 0;
    uint8_t vit = 0;

    static constexpr uint8_t FlgWithHeader = 0x01;
    static constexpr uint8_t Flg16Bit = 0x02;
    static constexpr uint8_t FlgStereo = 0x04;
    static constexpr uint8_t FlgCompressed = 0x08;
    static constexpr uint8_t FlgLooped = 0x10;
    static constexpr uint8_t FlgSusLooped = 0x20;
    static constexpr uint8_t FlgLoopPingPong = 0x40;
    static constexpr uint8_t FlgSusLoopPingPong = 0x80;
};
static_assert(sizeof(ItSampleHeader) == 0x50, "oooops");
#pragma pack(pop)

class ItSample final
    : public Sample
{
public:
    ItSampleHeader header{};

    void loadHeader(Stream& stream)
    {
        stream >> header;
    }

    void loadData(Stream& stream, uint16_t cmwt)
    {
        if( header.length == 0 )
        {
            return;
        }

        if( header.cvt & 0x02u )
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Not supported yet"));
        }
        if( header.cvt & 0x08u )
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Not supported yet"));
        }
        if( header.cvt & 0x10u )
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Not supported yet"));
        }
        if( header.cvt & 0x20u )
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Not supported yet"));
        }

        BOOST_ASSERT(header.length > 0);

        const bool _16Bit = (header.flg & ItSampleHeader::Flg16Bit) != 0;
        const bool mono = (header.flg & ItSampleHeader::FlgStereo) == 0;
        if( (header.flg & ItSampleHeader::FlgCompressed) != 0 )
        {
            bool fixedCompression = (header.cvt & 4u) != 0;

            // no compressed stereo
            std::vector<int16_t> data;
            if( _16Bit )
            {
                data = decompress<int16_t>(header.length, stream, fixedCompression);
            }
            else
            {
                data = decompress<int8_t>(header.length, stream, fixedCompression);
            }

            setData(interleave(data, data));
        }
        else
        {
            // signedness (or delta?)
            if( (header.cvt & 4u) != 0 )
            {
                // delta compression
                if( _16Bit )
                {
                    if( mono )
                    {
                        const auto data = decompressAdpcm<int16_t>(header.length, stream);
                        setData(interleave(data, data));
                    }
                    else
                    {
                        const auto l = decompressAdpcm<int16_t>(header.length, stream);
                        const auto r = decompressAdpcm<int16_t>(header.length, stream);
                        setData(interleave(l, r));
                    }
                }
                else
                {
                    if( mono )
                    {
                        const auto data = decompressAdpcm<int8_t>(header.length, stream);
                        setData(interleave(data, data));
                    }
                    else
                    {
                        const auto l = decompressAdpcm<int8_t>(header.length, stream);
                        const auto r = decompressAdpcm<int8_t>(header.length, stream);
                        setData(interleave(l, r));
                    }
                }
            }
            else
            {
                if( _16Bit )
                {
                    if( mono )
                    {
                        const auto data = readRaw<int16_t>(stream, header.length);
                        setData(interleave(data, data));
                    }
                    else
                    {
                        const auto l = readRaw<int16_t>(stream, header.length);
                        const auto r = readRaw<int16_t>(stream, header.length);
                        setData(interleave(l, r));
                    }
                }
                else
                {
                    if( mono )
                    {
                        const auto data = readRaw<int8_t>(stream, header.length);
                        setData(interleave(data, data));
                    }
                    else
                    {
                        const auto l = readRaw<int8_t>(stream, header.length);
                        const auto r = readRaw<int8_t>(stream, header.length);
                        setData(interleave(l, r));
                    }
                }

                if( (header.cvt & 1u) == 0 )
                {
                    swapSign();
                }
            }
        }

        BOOST_ASSERT(stream.good());
        BOOST_ASSERT(length() == header.length);
    }

private:
    static constexpr int16_t extend(const int8_t& value)
    {
        return static_cast<int16_t>(value * 256);
    }

    static constexpr int16_t extend(const int16_t& value)
    {
        return value;
    }

    template<typename T>
    static std::vector<int16_t> decompress(std::size_t len, Stream& stream, bool it215)
    {
        static_assert(std::is_signed<T>::value, "T must be signed");

        std::vector<int16_t> result;

        // now unpack data till the dest buffer is full
        while( len )
        {
            if( !stream )
            {
                return result;
            }

            stream.seekrel(2);

            // state for readBits
            uint32_t bitbuf = 0;
            uint32_t bitnum = 0;

            // length of compressed data block in samples
            std::size_t blklen = std::min(std::size_t(0x8000) / sizeof(T),
                                          len); // 0x4000/0x8000 samples => 0x8000 bytes again
            uint16_t blkpos = 0; // position in block

            uint8_t width = 1 + sizeof(T) * 8; // start with width of 9/17 bits
            T d1 = 0;
            T d2 = 0; // reset integrator buffers

            // now uncompress the data block
            while( blkpos < blklen )
            {
                if( width > 1 + sizeof(T) * 8 )
                {
                    // illegal width, abort
                    std::cerr << "Illegal bit width " << int(width) << " for " << (sizeof(T) * 8) << "-bit sample";
                    return result;
                }
                uint32_t value = readBits(width, bitbuf, bitnum, stream);

                if( width <= 6 )
                {
                    // method 1 (1-6 bits)
                    // check for "100..."
                    if( value == 1u << (width - 1) )
                    {
                        // yes!
                        value = readBits(sizeof(T) + 2, bitbuf, bitnum, stream) + 1; // read new width
                        width = (value < width) ? value : value + 1u; // and expand it
                        continue; // ... next value
                    }
                }
                else if( width <= sizeof(T) * 8 )
                {
                    // method 2 (7-8/16 bits)
                    std::make_unsigned_t<T> border = (std::make_unsigned_t<T>(-1) >> (sizeof(T) * 8 + 1 - width)) -
                                                     sizeof(T) * 4; // lower border for width chg
                    if( value > border && value <= uint32_t(border + sizeof(T) * 8) )
                    {
                        value -= border; // convert width to 1-8
                        width = (value < width) ? value : value + 1; // and expand it
                        continue; // ... next value
                    }
                }
                else
                {
                    // method 3 (17 bits)
                    // bit 16 set?
                    if( value & (1u << (sizeof(T) * 8)) )
                    {
                        width = (value + 1u) & 0xffu; // new width...
                        continue; // ... and next value
                    }
                }

                // now expand value to signed word
                T v;
                if( width < sizeof(T) * 8 )
                {
                    uint8_t shift = sizeof(T) * 8 - width;
                    v = T(value << shift) / (1 << shift);
                }
                else
                {
                    v = T(value);
                }

                // integrate upon the sample values
                d1 += v;
                d2 += d1;

                // .. and store it into the buffer
                result.emplace_back(extend(it215 ? d2 : d1));
                blkpos++;
            }

            // now subtract block length from total length and go on
            len -= blklen;
        }

        return result;
    }

    template<typename T>
    static std::vector<int16_t> decompressAdpcm(std::size_t len, Stream& stream)
    {
        std::vector<int16_t> result;

        T delta = 0;
        while( len-- )
        {
            T tmp;
            stream.read(reinterpret_cast<char*>(&tmp), sizeof(T));
            delta += tmp;
            result.emplace_back(extend(delta));
        }

        return result;
    }

    static uint32_t readBits(const uint8_t n, uint32_t& bitBuffer, uint32_t& bitOffset, Stream& stream)
    {
        BOOST_ASSERT(n <= 32);

        uint32_t value = 0;
        uint8_t i = n;

        // this could be better
        while( i-- )
        {
            if( bitOffset == 0 )
            {
                uint8_t tmp;
                stream >> tmp;
                bitBuffer = tmp;
                bitOffset = 8;
            }
            value >>= 1;
            value |= bitBuffer << 31u;
            bitBuffer >>= 1;
            --bitOffset;
        }
        return value >> (32u - n);
    }

    template<typename T>
    static std::vector<int16_t> readRaw(Stream& stream, std::size_t len)
    {
        std::vector<int16_t> result;
        result.reserve(len);
        while( len-- )
        {
            T tmp;
            stream >> tmp;
            result.emplace_back(extend(tmp));
        }
        return result;
    }

    static AudioFrameBuffer interleave(const std::vector<int16_t>& l, const std::vector<int16_t>& r)
    {
        BOOST_ASSERT(l.size() == r.size());

        AudioFrameBuffer result;
        result.reserve(l.size());
        for( std::size_t i = 0; i < l.size(); ++i )
        {
            result.emplace_back(l[i], r[i]);
        }
        return result;
    }

    void swapSign()
    {
        for( auto it = beginIterator(); it != endIterator(); ++it )
        {
            it->left ^= 0x8000;
            it->right ^= 0x8000;
        }
    }
};
}
}