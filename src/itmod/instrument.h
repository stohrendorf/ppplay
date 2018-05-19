#pragma once

#include <cstdint>

namespace ppp
{
namespace it
{
#pragma pack(push, 1)

struct Envelope
{
    struct Point
    {
        int8_t y = 0;
        uint16_t tick = 0;
    };

    explicit Envelope(int8_t y)
        : points{{y, 0},
                 {y, 100 << 8},
                 {0, 0}}
    {}

    uint8_t flg = 0;
    uint8_t num = 2;
    uint8_t lpb = 0;
    uint8_t lpe = 0;
    uint8_t slb = 0;
    uint8_t sle = 0;
    Point points[25];
    uint8_t x = 0;

    static constexpr const uint8_t FlgOn = 0x01;
    static constexpr const uint8_t FlgLoop = 0x02;
    static constexpr const uint8_t FlgSusLoop = 0x04;
    static constexpr const uint8_t FlgFilter = 0x80;

    bool isEnabled() const noexcept
    {
        return (flg & FlgOn) != 0;
    }

    bool hasAnyLoop() const noexcept
    {
        return hasGlobalLoop() || hasSusLoop();
    }

    bool hasGlobalLoop() const noexcept
    {
        return (flg & FlgLoop) != 0;
    }

    bool hasSusLoop() const noexcept
    {
        return (flg & FlgSusLoop) != 0;
    }

    bool hasFilter() const noexcept
    {
        return (flg & FlgFilter) != 0;
    }
};

static_assert(sizeof(Envelope) == 82, "oooops");

constexpr uint8_t DCA_CUT = 0;
constexpr uint8_t DCA_NOTE_OFF = 1;
constexpr uint8_t DCA_NOTE_FADE = 2;

constexpr uint8_t DCT_OFF = 0;
constexpr uint8_t DCT_NOTE = 1;
constexpr uint8_t DCT_SAMPLE = 2;
constexpr uint8_t DCT_INSTRUMENT = 3;

constexpr uint8_t NNA_CUT = 0;
constexpr uint8_t NNA_CONTINUE = 1;
constexpr uint8_t NNA_NOTE_OFF = 2;
constexpr uint8_t NNA_NOTE_FADE = 3;

struct ItInstrument
{
    char id[4] = {'I', 'M', 'P', 'I'};
    char filename[12] = "";

    explicit ItInstrument()
        : volEnv{64}, panEnv{0}, pitchEnv{0}
    {
        for( int i = 0; i < 120; ++i )
        {
            keyboardTable[i].note = i;
        }
    };

    uint8_t null = 0;
    uint8_t nna = NNA_CUT;
    uint8_t dct = DCT_OFF;
    uint8_t dca = DCA_CUT;
    int16_t fadeout = 0;
    int8_t pitchPanSeparation = 0; //!< pps
    uint8_t pitchPanCenter = 60; //!< ppc
    uint8_t gbv = 128;
    uint8_t dfp = 32 + 128;
    uint8_t rv = 0;
    uint8_t rp = 0;
    uint16_t trkVers = 0;
    uint8_t nos = 0;
    uint8_t x = 0;

    char name[26] = "";
    uint8_t ifc = 0; //!< Initial filter cutoff
    uint8_t ifr = 0; //!< Initial filter resonance
    uint8_t mch = 0;
    uint8_t mpr = 0xff;
    uint16_t midibnk = 0xffff;

    struct MappingEntry
    {
        uint8_t note = 0;
        uint8_t sample = 0; //!< 1-based
    };

    // 0/0, 1/0, 2/0, ...
    MappingEntry keyboardTable[120];

    Envelope volEnv{64};
    Envelope panEnv{0};
    Envelope pitchEnv{0};
};

static_assert(sizeof(ItInstrument) == 550, "oooops");
#pragma pack(pop)
}
}