#pragma once

#include <cstdint>
#include <vector>

namespace compression
{
class SixPack
{
    static constexpr std::size_t COPYRANGES = 6;
    static constexpr std::size_t FIRSTCODE = 257;
    static constexpr std::size_t MINCOPY = 3;
    static constexpr std::size_t MAXCOPY = 255;
    static constexpr std::size_t CODESPERRANGE = (MAXCOPY - MINCOPY + 1);
    static constexpr std::size_t MAXCHAR = (FIRSTCODE + COPYRANGES * CODESPERRANGE - 1);
    static constexpr std::size_t TWICEMAX = (2 * MAXCHAR + 1);

    static constexpr std::size_t MAXFREQ = 2000;
    static constexpr std::size_t TERMINATE = 256;
    static constexpr std::size_t SUCCMAX = MAXCHAR + 1;
    static constexpr std::size_t ROOT = 1;
    static constexpr std::size_t MAXBUF = 42 * 1024;
    static constexpr std::size_t MAXDISTANCE = 21389;
    static constexpr std::size_t MAXSIZE = MAXDISTANCE + MAXCOPY;

public:
    explicit SixPack(const std::vector<uint16_t>& srcData);

    const std::vector<uint8_t>& get() const;

private:
    void initTree();

    void updateFreq(std::size_t a, uint16_t b);

    void updateModel(uint16_t code);

    uint16_t readBits(uint8_t bits);

    uint16_t decompressChar();

    std::vector<uint8_t> decompress();

    uint16_t m_leftc[MAXCHAR + 1]{};
    uint16_t m_rightc[MAXCHAR + 1]{};
    uint16_t m_dad[TWICEMAX + 1]{};
    uint16_t m_freq[TWICEMAX + 1]{};

    uint16_t m_bitcount = 0;
    uint16_t m_bitbuffer = 0;

    const std::vector<uint16_t>& m_srcData;
    std::size_t m_srcPosition = 0;
    std::vector<uint8_t> m_decompressed{};
};
}