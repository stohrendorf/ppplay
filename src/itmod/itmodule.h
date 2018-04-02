#pragma once

#include "genmod/abstractmodule.h"

#include "slavechannel.h"
#include "hostchannel.h"
#include "sample.h"
#include "instrument.h"

namespace ppp
{
namespace it
{
using ItPattern = std::vector<uint8_t>;

#pragma pack(push, 1)
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
struct ITHeader
{
    char id[4]; // 'IMPM'
    char name[26];
    uint16_t patternHighlight;
    uint16_t ordNum;
    uint16_t insNum;
    uint16_t smpNum;
    uint16_t patNum;
    uint16_t cwtV;
    uint16_t cmwt; // >= 0x0200 !!!
    uint16_t flags;
    uint16_t special;
    uint8_t gv;
    uint8_t mv;
    uint8_t is;
    uint8_t it;
    uint8_t sep;
    uint8_t pwd; //!< pitch wheel depth
    uint16_t msgLen;
    uint32_t messageOffset;
    uint32_t reserved;
    uint8_t chnPan[64];
    uint8_t chnVol[64];

    static constexpr uint16_t FlgStereo = 0x01;
    static constexpr uint16_t FlgInstrumentMode = 0x04;
    static constexpr uint16_t FlgLinear = 0x08;
    static constexpr uint16_t FlgOldEffects = 0x10;
    static constexpr uint16_t FlgLinkedEffects = 0x20;
    static constexpr uint16_t FlgMidiPitch = 0x40;
    static constexpr uint16_t FlgMidiEmbedded = 0x80;

    static constexpr uint16_t SFlgSngMessage = 0x01;
    static constexpr uint16_t SFlgMidiEmbedded = 0x80;
};
static_assert(sizeof(ITHeader) == 0xc0, "Ooops");
#pragma clang diagnostic pop
#pragma pack(pop)

constexpr uint8_t PATTERN_MAX_NOTE = 12 * 10 - 1;
constexpr uint8_t PATTERN_NOTE_OFF = 255;
constexpr uint8_t PATTERN_NOTE_CUT = 254;

constexpr uint8_t VFX_FINE_VOL_UP = 0;
constexpr uint8_t VFX_FINE_VOL_DOWN = 1;
constexpr uint8_t VFX_VOL_UP = 2;
constexpr uint8_t VFX_VOL_DOWN = 3;
constexpr uint8_t VFX_PITCH_DOWN = 4;
constexpr uint8_t VFX_PITCH_UP = 5;
constexpr uint8_t VFX_PORTA = 6;
constexpr uint8_t VFX_VIBRATO = 7;

class ItModule final
    : public AbstractModule
{
public:
    DISABLE_COPY(ItModule)

    static AbstractModule* factory(Stream* stream, uint32_t frequency, int maxRpt, Sample::Interpolation inter);

private:
    ITHeader m_header{};

    std::vector<ItInstrument> m_instruments{};
    std::vector<ItPattern> m_patterns{};
    std::array<HostChannel, 64> m_hosts{};
    std::array<SlaveChannel, 256> m_slaves{};

    int16_t m_nextRow = -2;

    int16_t m_nextOrder = -1;

    uint16_t m_tickCountdown = 1;

    int16_t m_currentDecodingPattern = -2;
    int16_t m_currentDecodingRow = -2;
    uint8_t m_rowDelay = 1;
    bool m_rowDelayActive = false;

    uint16_t m_breakRow = 0;
    bool m_patternLooping = false;

    const uint8_t* m_patternDataPtr = nullptr;
    uint16_t m_patternRows = 0;

    SlaveChannel* m_lastSlaveChannel = nullptr;

protected:
    AbstractArchive& serialize(AbstractArchive* data) override
    {
        AbstractModule::serialize(data)
        % m_nextRow
        % m_nextOrder
        % m_tickCountdown
        % m_currentDecodingPattern
        % m_currentDecodingRow
        % m_rowDelay
        % m_rowDelayActive
        % m_breakRow
        % m_patternLooping
        % reinterpret_cast<uintptr_t&>(m_patternDataPtr)
        % m_patternRows
        % reinterpret_cast<uintptr_t&>(m_lastSlaveChannel);

        for( auto& channel : m_slaves )
        {
            *data % channel;
        }
        for( auto& channel : m_hosts )
        {
            *data % channel;
        }

        return *data;
    }

public:
    ItModule(int maxRpt, Sample::Interpolation inter)
        : AbstractModule{maxRpt, inter}
    {
    }

    ~ItModule() override = default;

private:
    size_t internal_buildTick(const AudioFrameBufferPtr& buffer) override;

    ChannelState internal_channelStatus(size_t idx) const override;

    int internal_channelCount() const override;

    static light4cxx::Logger* logger();

    bool update();

    bool updateData();

    void loadRow();

    void goToProcessRow();

    void onCellLoaded(HostChannel& host);

    void updateSamples();

    void updateVibrato(SlaveChannel& slave);

    void updateInstruments();

    void updateMidi();

    void midiTranslate(HostChannel* host, SlaveChannel* slave, uint16_t cmd);

    void midiTranslateParametrized(HostChannel* host, SlaveChannel* slave, uint16_t cmd);

    void M32MixHandler(MixerFrameBuffer& buffer);

public:
    std::vector<std::unique_ptr<ItSample>> m_samples{};

    void initNoCommand(HostChannel& host);

    void initCommandA(HostChannel& host);

    void initCommandB(HostChannel& host);

    void initCommandC(HostChannel& host);

    void initCommandD(HostChannel& host);

    void initCommandDKL(HostChannel& host);

    void initCommandE(HostChannel& host);

    void initCommandF(HostChannel& host);

    void initCommandG(HostChannel& host);

    void initCommandGL(HostChannel& host);

    void initCommandH(HostChannel& host);

    void initCommandI(HostChannel& host);

    void initCommandJ(HostChannel& host);

    void initCommandK(HostChannel& host);

    void initCommandL(HostChannel& host);

    void initCommandM(HostChannel& host);

    void initCommandN(HostChannel& host);

    void initCommandO(HostChannel& host);

    void initCommandP(HostChannel& host);

    void initCommandQ(HostChannel& host);

    void initCommandR(HostChannel& host);

    void initCommandS(HostChannel& host);

    void initCommandT(HostChannel& host);

    void initCommandU(HostChannel& host);

    void initCommandV(HostChannel& host);

    void initCommandW(HostChannel& host);

    void initCommandX(HostChannel& host);

    void initCommandY(HostChannel& host);

    void initCommandZ(HostChannel& host);

    void commandD(HostChannel& host);

    void commandE(HostChannel& host);

    void commandF(HostChannel& host);

    void commandG(HostChannel& host);

    void commandH(HostChannel& host);

    void commandI(HostChannel& host);

    void commandJ(HostChannel& host);

    void commandK(HostChannel& host);

    void commandL(HostChannel& host);

    void commandN(HostChannel& host);

    void commandP(HostChannel& host);

    void commandQ(HostChannel& host);

    void commandR(HostChannel& host);

    void commandS(HostChannel& host);

    void commandT(HostChannel& host);

    void commandW(HostChannel& host);

    void commandY(HostChannel& host);

    void pitchSlideDown(SlaveChannel& slave, uint16_t val);

    void pitchSlideUp(HostChannel& host, uint16_t val);

    void initVibrato(HostChannel& host);

    void setVibrato(HostChannel& host);

    void initVolumeEffect(HostChannel& host);

    void volumeCommandC(HostChannel& host);

    void volumeCommandD(HostChannel& host);

    void volumeCommandE(HostChannel& host);

    void volumeCommandF(HostChannel& host);

    void volumeCommandG(HostChannel& host);

    bool applySample(HostChannel& host, SlaveChannel* slave);

    void recalculateAllVolumes()
    {
        for( auto& slave : m_slaves )
        {
            slave.flags |= SCFLG_RECALC_VOL | SCFLG_RECALC_PAN;
        }
    }

    SlaveChannel* allocateChannel(HostChannel& host);

    SlaveChannel* allocateSampleChannel(HostChannel& host);

    SlaveChannel* allocateSampleSearch(HostChannel& host, const ItInstrument& instrument);

    SlaveChannel* allocateChannelInstrument(SlaveChannel* slave, HostChannel& host, const ItInstrument& instrument);

    SlaveChannel* searchUsedChannel(HostChannel& host);
};
}
}