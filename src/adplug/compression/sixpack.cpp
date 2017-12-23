#include "sixpack.h"

#include <array>

namespace compression
{

namespace
{
const std::array<uint16_t, 6> copybits = {{4, 6, 8, 10, 12, 14}};

const std::array<uint16_t, 6> copymin = {{0, 16, 80, 336, 1360, 5456}};
}

std::vector<uint8_t> SixPack::decompress()
{
    uint16_t windowPos = 0;

    initTree();

    std::vector<uint8_t> window;
    window.resize(MAXBUF);

    std::vector<uint8_t> decompressed;

    for( auto c = decompressChar(); c != TERMINATE; c = decompressChar() )
    {
        if( c < 256 )
        {
            decompressed.emplace_back(static_cast<uint8_t>(c));
            window[windowPos++] = static_cast<uint8_t>(c);
            if( windowPos == MAXSIZE )
            {
                windowPos = 0;
            }
        }
        else
        {
            const auto t = c - FIRSTCODE;
            const auto index = t / CODESPERRANGE;
            const auto len = t + MINCOPY - index * CODESPERRANGE;
            const auto dist = readBits(copybits[index]) + len + copymin[index];

            auto j = windowPos;
            auto k = windowPos - dist;
            if( windowPos < dist )
            {
                k += MAXSIZE;
            }

            for( auto i = 0u; i <= len - 1; i++ )
            {
                decompressed.emplace_back(window[k]);
                window[j] = window[k];
                j++;
                k++;
                if( j == MAXSIZE )
                {
                    j = 0;
                }
                if( k == MAXSIZE )
                {
                    k = 0;
                }
            }

            windowPos += len;
            if( windowPos >= MAXSIZE )
            {
                windowPos -= MAXSIZE;
            }
        }
    }

    return decompressed;
}

void SixPack::initTree()
{
    for( uint16_t i = 2; i <= TWICEMAX; i++ )
    {
        m_dad[i] = i / 2u;
        m_freq[i] = 1;
    }

    for( uint16_t i = 1; i <= MAXCHAR; i++ )
    {
        m_leftc[i] = 2u * i;
        m_rightc[i] = 2u * i + 1u;
    }
}

void SixPack::updateFreq(std::size_t a, uint16_t b)
{
    do
    {
        m_freq[m_dad[a]] = m_freq[a] + m_freq[b];
        a = m_dad[a];
        if( a != ROOT )
        {
            if( m_leftc[m_dad[a]] == a )
            {
                b = m_rightc[m_dad[a]];
            }
            else
            {
                b = m_leftc[m_dad[a]];
            }
        }
    } while( a != ROOT );

    if( m_freq[ROOT] == MAXFREQ )
    {
        for( a = 1; a <= TWICEMAX; a++ )
        {
            m_freq[a] >>= 1;
        }
    }
}

void SixPack::updateModel(uint16_t code)
{
    std::size_t a = code + SUCCMAX;

    m_freq[a]++;
    if( m_dad[a] != ROOT )
    {
        auto code1 = m_dad[a];
        if( m_leftc[code1] == a )
        {
            updateFreq(a, m_rightc[code1]);
        }
        else
        {
            updateFreq(a, m_leftc[code1]);
        }

        do
        {
            const auto code2 = m_dad[code1];
            uint16_t b;
            if( m_leftc[code2] == code1 )
            {
                b = m_rightc[code2];
            }
            else
            {
                b = m_leftc[code2];
            }

            if( m_freq[a] > m_freq[b] )
            {
                if( m_leftc[code2] == code1 )
                {
                    m_rightc[code2] = a;
                }
                else
                {
                    m_leftc[code2] = a;
                }

                uint16_t c;
                if( m_leftc[code1] == a )
                {
                    m_leftc[code1] = b;
                    c = m_rightc[code1];
                }
                else
                {
                    m_rightc[code1] = b;
                    c = m_leftc[code1];
                }

                m_dad[b] = code1;
                m_dad[a] = code2;
                updateFreq(b, c);
                a = b;
            }

            a = m_dad[a];
            code1 = m_dad[a];
        } while( code1 != ROOT );
    }
}

uint16_t SixPack::readBits(uint8_t bits)
{
    uint16_t result = 0;

    for( int i = 0; i < bits; i++ )
    {
        if( m_bitcount == 0 )
        {
            if( m_bitcount == MAXBUF )
            {
                m_srcPosition = 0;
            }
            m_bitbuffer = m_srcData[m_srcPosition++];
            m_bitcount = 15;
        }
        else
        {
            m_bitcount--;
        }

        if( m_bitbuffer > 0x7fff )
        {
            result |= 1u << i;
        }
        m_bitbuffer <<= 1;
    }

    return result;
}

uint16_t SixPack::decompressChar()
{
    uint16_t a = 1;

    do
    {
        if( readBits(1) != 0 )
        {
            a = m_rightc[a];
        }
        else
        {
            a = m_leftc[a];
        }
    } while( a <= MAXCHAR );

    a -= SUCCMAX;
    updateModel(a);
    return a;
}

SixPack::SixPack(const std::vector<uint16_t>& srcData)
    : m_srcData(srcData)
{
    m_decompressed = decompress();
}

const std::vector<uint8_t>& SixPack::get() const
{
    return m_decompressed;
}
}