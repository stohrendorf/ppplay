#include <cmath>
#include <map>
#include <genmod/standardfxdesc.h>
#include <genmod/genbase.h>
#include <cstring>
#include "itmodule.h"

#include "genmod/orderentry.h"
#include "genmod/channelstate.h"
#include "filters.h"

namespace ppp
{
namespace it
{
class ItOrder
    : public OrderEntry
{
public:
    DISABLE_COPY(ItOrder)

    ItOrder() = delete;

    explicit ItOrder(uint8_t idx)
        : OrderEntry(idx)
    {
    }

    bool isUnplayed() const override
    {
        return OrderEntry::isUnplayed() && index() < 254;
    }
};

typedef void (ItModule::*InitCommand)(HostChannel&);

typedef void (ItModule::*Command)(HostChannel&);

std::array<InitCommand, 32> initCommands{
    &ItModule::initNoCommand,
    &ItModule::initCommandA,
    &ItModule::initCommandB,
    &ItModule::initCommandC,
    &ItModule::initCommandD,
    &ItModule::initCommandE,
    &ItModule::initCommandF,
    &ItModule::initCommandG,
    &ItModule::initCommandH,
    &ItModule::initCommandI,
    &ItModule::initCommandJ,
    &ItModule::initCommandK,
    &ItModule::initCommandL,
    &ItModule::initCommandM,
    &ItModule::initCommandN,
    &ItModule::initCommandO,
    &ItModule::initCommandP,
    &ItModule::initCommandQ,
    &ItModule::initCommandR,
    &ItModule::initCommandS,
    &ItModule::initCommandT,
    &ItModule::initCommandU,
    &ItModule::initCommandV,
    &ItModule::initCommandW,
    &ItModule::initCommandX,
    &ItModule::initCommandY,
    &ItModule::initCommandZ,
    &ItModule::initNoCommand,
    &ItModule::initNoCommand,
    &ItModule::initNoCommand,
    &ItModule::initNoCommand,
    &ItModule::initNoCommand
};

std::array<Command, 8> volCommands{
    nullptr,
    nullptr,
    &ItModule::volumeCommandC,
    &ItModule::volumeCommandD,
    &ItModule::volumeCommandE,
    &ItModule::volumeCommandF,
    &ItModule::volumeCommandG,
    &ItModule::commandH
};

std::array<Command, 32> commands{
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &ItModule::commandD,
    &ItModule::commandE,
    &ItModule::commandF,
    &ItModule::commandG,
    &ItModule::commandH,
    &ItModule::commandI,
    &ItModule::commandJ,
    &ItModule::commandK,
    &ItModule::commandL,
    nullptr,
    &ItModule::commandN,
    nullptr,
    &ItModule::commandP,
    &ItModule::commandQ,
    &ItModule::commandR,
    &ItModule::commandS,
    &ItModule::commandT,
    &ItModule::commandH,
    nullptr,
    &ItModule::commandW,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

const uint8_t slideTable[] = {1, 4, 8, 16, 32, 64, 96, 128, 255};

const std::array<int8_t, 256> fineSineData = {
    0, 2, 3, 5, 6, 8, 9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
    24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
    59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
    59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
    45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
    24, 23, 22, 20, 19, 17, 16, 14, 12, 11, 9, 8, 6, 5, 3, 2,
    0, -2, -3, -5, -6, -8, -9, -11, -12, -14, -16, -17, -19, -20, -22, -23,
    -24, -26, -27, -29, -30, -32, -33, -34, -36, -37, -38, -39, -41, -42, -43, -44,
    -45, -46, -47, -48, -49, -50, -51, -52, -53, -54, -55, -56, -56, -57, -58, -59,
    -59, -60, -60, -61, -61, -62, -62, -62, -63, -63, -63, -64, -64, -64, -64, -64,
    -64, -64, -64, -64, -64, -64, -63, -63, -63, -62, -62, -62, -61, -61, -60, -60,
    -59, -59, -58, -57, -56, -56, -55, -54, -53, -52, -51, -50, -49, -48, -47, -46,
    -45, -44, -43, -42, -41, -39, -38, -37, -36, -34, -33, -32, -30, -29, -27, -26,
    -24, -23, -22, -20, -19, -17, -16, -14, -12, -11, -9, -8, -6, -5, -3, -2
};

const std::array<int8_t, 256> fineRampDownData = {
    64, 63, 63, 62, 62, 61, 61, 60, 60, 59, 59, 58, 58, 57, 57, 56,
    56, 55, 55, 54, 54, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48,
    48, 47, 47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42, 41, 41, 40,
    40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32,
    32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24,
    24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
    16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10, 9, 9, 8,
    8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0,
    0, -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6, -7, -7, -8,
    -8, -9, -9, -10, -10, -11, -11, -12, -12, -13, -13, -14, -14, -15, -15, -16,
    -16, -17, -17, -18, -18, -19, -19, -20, -20, -21, -21, -22, -22, -23, -23, -24,
    -24, -25, -25, -26, -26, -27, -27, -28, -28, -29, -29, -30, -30, -31, -31, -32,
    -32, -33, -33, -34, -34, -35, -35, -36, -36, -37, -37, -38, -38, -39, -39, -40,
    -40, -41, -41, -42, -42, -43, -43, -44, -44, -45, -45, -46, -46, -47, -47, -48,
    -48, -49, -49, -50, -50, -51, -51, -52, -52, -53, -53, -54, -54, -55, -55, -56,
    -56, -57, -57, -58, -58, -59, -59, -60, -60, -61, -61, -62, -62, -63, -63, -64
};

const std::array<int8_t, 256> fineSquareWave = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

bool updateEnvelope(SEnvelope& slaveEnvelope, const Envelope& insEnvelope, bool noteOff)
{
    if( slaveEnvelope.tick < slaveEnvelope.nextPointTick )
    {
        ++slaveEnvelope.tick;
        slaveEnvelope.value.next();
        return true;
    }

    // Procedure:
    // 1) Get current pos' value
    // 2) Figure out next pos (inc/loop)
    // 3) Figure out delta to next pos
    // 4) Terminate if no loop (with carry)
    //      or place new check in [SI+0Ch]

    BOOST_ASSERT(slaveEnvelope.nextPointIndex < 25);

    slaveEnvelope.value = insEnvelope.points[slaveEnvelope.nextPointIndex].y;

    ++slaveEnvelope.nextPointIndex;

    if( insEnvelope.hasAnyLoop() )
    {
        uint8_t lpb = insEnvelope.lpb;
        uint8_t lpe = insEnvelope.lpe;

        bool isLooped = false;
        if( insEnvelope.hasSusLoop() && !noteOff )
        {
            lpb = insEnvelope.slb;
            lpe = insEnvelope.sle;
            isLooped = true;
        }
        else if( insEnvelope.hasGlobalLoop() )
        {
            isLooped = true;
        }

        if( isLooped )
        {
            BOOST_ASSERT(lpb < 25);
            BOOST_ASSERT(lpe < 25);
            BOOST_ASSERT(lpb <= insEnvelope.num);
            BOOST_ASSERT(lpe <= insEnvelope.num);

            // UpdateEnvelope3
            if( slaveEnvelope.nextPointIndex >= lpe )
            {
                slaveEnvelope.nextPointIndex = lpb;
                slaveEnvelope.tick = insEnvelope.points[lpb].tick;
                slaveEnvelope.nextPointTick = insEnvelope.points[lpb].tick;
                return true;
            }
        }
    }

    if( slaveEnvelope.nextPointIndex >= insEnvelope.num )
    {
        return false;
    }

    BOOST_ASSERT(slaveEnvelope.nextPointIndex < 25);
    slaveEnvelope.nextPointTick = insEnvelope.points[slaveEnvelope.nextPointIndex].tick;
    slaveEnvelope.tick = insEnvelope.points[slaveEnvelope.nextPointIndex - 1].tick + 1u;

    const auto dt = int(insEnvelope.points[slaveEnvelope.nextPointIndex].tick) - insEnvelope.points[slaveEnvelope.nextPointIndex - 1].tick;
    BOOST_ASSERT(dt >= 0);
    const auto dy = int(insEnvelope.points[slaveEnvelope.nextPointIndex].y) - insEnvelope.points[slaveEnvelope.nextPointIndex - 1].y;
    if( dt == 0 )
    {
        slaveEnvelope.value.setStepSize(1, dy);
    }
    else
    {
        slaveEnvelope.value.setStepSize(dt, dy);
    }
    return true;
}

void pitchSlideDownLinear(SlaveChannel& slave, uint16_t val)
{
    slave.flags |= SCFLG_FREQ_CHANGE;

    slave.frequency = std::lround(slave.frequency / std::pow(2, val / 768.0));
}

void pitchSlideUpLinear(SlaveChannel& slave, uint16_t val)
{
    slave.flags |= SCFLG_FREQ_CHANGE;

    slave.frequency = std::lround(slave.frequency * std::pow(2, val / 768.0f));
}

void pitchSlideDownAmiga(SlaveChannel& slave, uint16_t val)
{
    slave.flags |= SCFLG_FREQ_CHANGE;

    static constexpr auto F = 1712ul * 8363ul;

    auto sf = uint64_t(slave.frequency) * val;

    slave.frequency = slave.frequency * F / (F + sf);
}

void ItModule::pitchSlideDown(SlaveChannel& slave, uint16_t val)
{
    if( (m_header.flags & ITHeader::FlgLinear) == 0 )
    {
        return pitchSlideDownAmiga(slave, val);
    }
    else
    {
        return pitchSlideDownLinear(slave, val);
    }
}

void pitchSlideUpAmiga(HostChannel& host, uint16_t val)
{
    host.getSlave()->flags |= SCFLG_FREQ_CHANGE;

    static constexpr auto F = 1712ul * 8363ul;

    auto sf = uint64_t(host.getSlave()->frequency) * val;
    if( sf >= F )
    {
        host.getSlave()->flags |= SCFLG_NOTE_CUT;
        host.disable();
        return;
    }

    host.getSlave()->frequency = host.getSlave()->frequency * F / (F - sf);
}

void ItModule::pitchSlideUp(HostChannel& host, uint16_t val)
{
    if( (m_header.flags & ITHeader::FlgLinear) == 0 )
    {
        return pitchSlideUpAmiga(host, val);
    }
    else
    {
        return pitchSlideUpLinear(*host.getSlave(), val);
    }
}

void setVolume(HostChannel& host, uint8_t volume)
{
    BOOST_ASSERT(volume <= 64);
    host.getSlave()->flags |= SCFLG_RECALC_VOL;
    host.getSlave()->effectiveBaseVolume = host.getSlave()->effectiveVolume = volume;
    host.vse = volume;
}

void setPan(HostChannel& host, uint8_t pan)
{
    BOOST_ASSERT(pan <= 64 || pan == SurroundPan);

    if( host.isEnabled() )
    {
        host.getSlave()->pan = pan;
        host.getSlave()->ps = pan;
        host.getSlave()->flags |= SCFLG_RECALC_FINAL_VOL | SCFLG_RECALC_PAN;
    }
    host.cp = pan;
}

void applyRandomValues(HostChannel& host)
{
    host.flags &= ~HCFLG_RANDOM;

    if( host.getSlave()->insOffs->rv != 0 )
    {
        auto vol = host.getSlave()->sampleVolume +
                   (host.getSlave()->insOffs->rv * ((std::rand() & 0xff) - 128) / 64 + 1) * host.getSlave()->sampleVolume / 199;
        if( vol < 0 )
        {
            host.getSlave()->sampleVolume = 0;
        }
        else if( vol > 128 )
        {
            host.getSlave()->sampleVolume = 128;
        }
        else
        {
            host.getSlave()->sampleVolume = static_cast<uint8_t>(vol);
        }
    }

    if( host.getSlave()->insOffs->rp != 0 && host.getSlave()->pan != SurroundPan )
    {
        auto pan = host.getSlave()->pan + host.getSlave()->insOffs->rp * ((std::rand() & 0xff) - 128) / 128;
        if( pan < 0 )
        {
            host.getSlave()->pan = 0;
            host.getSlave()->ps = 0;
        }
        else if( pan > 64 )
        {
            host.getSlave()->pan = 64;
            host.getSlave()->ps = 64;
        }
        else
        {
            host.getSlave()->pan = static_cast<uint8_t>(pan);
            host.getSlave()->ps = static_cast<uint8_t>(pan);
            BOOST_ASSERT(host.getSlave()->pan <= 64 || host.getSlave()->pan == SurroundPan);
        }
    }
}

void initTremolo(HostChannel& host, int8_t value)
{
    host.ltr = value;
    int vol = (value * host.tremoloDepth * 4 + 128) / 256 + host.getSlave()->effectiveVolume;
    if( vol < 0 )
    {
        vol = 0;
    }
    else if( vol > 64 )
    {
        vol = 64;
    }
    host.getSlave()->effectiveVolume = static_cast<uint8_t>(vol);
}

bool ItModule::applySample(HostChannel& host, SlaveChannel* slave)
{
    // slave->flags &= 0x00ff;
    slave->flags |= SCFLG_NEW_NOTE;

    BOOST_ASSERT(host.sampleIndex >= 1 && host.sampleIndex <= m_samples.size());

    BOOST_ASSERT(slave->smp >= 0 && slave->smp < m_samples.size());
    slave->smp = host.sampleIndex - 1;
    slave->smpOffs = m_samples[slave->smp].get();

    slave->viDepth = 0;
    slave->sampleOffset = 0;
    slave->loopDirBackward = false;

    slave->sampleVolume = slave->smpOffs->header.gvl * 2;
    BOOST_ASSERT(slave->sampleVolume <= 128);

    if( (slave->smpOffs->header.flg & ItSampleHeader::FlgWithHeader) == 0 )
    {
        slave->flags = SCFLG_NOTE_CUT;
        host.disable();
        return false;
    }

    slave->vip = 0;

    slave->applySampleLoop();
    return true;
}

void ItModule::setVibrato(HostChannel& host)
{
    auto delta = ((host.lvi * host.vdp) * 4 + 0x80) / 256;

    if( (m_header.flags & ITHeader::FlgOldEffects) != 0 )
    {
        delta = -delta;
    }

    if( delta < 0 )
    {
        pitchSlideDown(*host.getSlave(), -delta);
    }
    else if( delta > 0 )
    {
        pitchSlideUp(host, delta);
    }
}

void ItModule::initVolumeEffect(HostChannel& host)
{
    if( (host.cellMask & HCFLG_MSK_VOL) == 0 )
    {
        return;
    }

    auto patternVolume = host.patternVolume & 0x7f;

    if( patternVolume <= 64 )
    { // raw volume or pan
        return;
    }

    patternVolume -= 65;
    if( (host.patternVolume & 0x80) != 0 )
    {
        patternVolume += 60;
    } // remove pan values 128..192

    const uint8_t param = patternVolume % 10u;// AH = effect parameter
    host.volumeFx = patternVolume / 10u;// AL = effect number

    if( param != 0 )
    {
        if( host.volumeFx < VFX_PITCH_DOWN ) // vol slide
        {
            host.volumeFxParam = param;
        }
        else if( host.volumeFx < VFX_PORTA ) // pitch slide
        {
            host.efg = param << 2;
        }
        else if( host.volumeFx == VFX_PORTA ) // porta
        {
            if( (m_header.flags & ITHeader::FlgLinkedEffects) == 0 )
            {
                host.linkedPortaFxValue = slideTable[param - 1];
            }
            else
            {
                host.efg = slideTable[param - 1];
            }
        }
    }

    if( !host.isEnabled() )
    {
        if( host.volumeFx != VFX_VIBRATO )
        { // vibrato
            return;
        }

        if( param != 0 )
        {
            host.vdp = param << 2;
        }

        if( !host.isEnabled() )
        {
            return;
        }

        initVibrato(host);
        return;
    }

    if( host.volumeFx > VFX_FINE_VOL_DOWN )
    {
        host.flags |= HCFLG_UPD_VOL_IF_ON;

        if( host.volumeFx >= VFX_VIBRATO )
        {
            if( param != 0 )
            {
                host.vdp = param * 4u;
            }

            if( !host.isEnabled() )
            {
                return;
            }

            initVibrato(host);
        }
        else if( host.volumeFx == VFX_PORTA )
        {
            initPorta(host);
        }
    }
    else if( host.volumeFx == VFX_FINE_VOL_DOWN )
    {
        auto tmp = host.getSlave()->effectiveBaseVolume - host.volumeFxParam;
        if( tmp < 0 )
        {
            tmp = 0;
        }
        setVolume(host, tmp);
    }
    else
    {
        // Fine volume slide up.
        auto tmp = host.getSlave()->effectiveBaseVolume + host.volumeFxParam;
        if( tmp > 64 )
        {
            tmp = 64;
        }
        setVolume(host, tmp);
    }
}

void ItModule::initVibrato(HostChannel& host)
{
    if( (m_header.flags & ITHeader::FlgOldEffects) == 0 )
    {
        commandH(host);
        return;
    }

    host.getSlave()->flags |= SCFLG_FREQ_CHANGE;
    setVibrato(host);
}

AbstractModule* ItModule::factory(Stream* stream, uint32_t frequency, int maxRpt, ppp::Sample::Interpolation inter)
{
    BOOST_ASSERT(stream != nullptr);
    stream->seek(0);

    auto result = std::make_unique<ItModule>(maxRpt, inter);
    *stream >> result->m_header;

    if( std::strncmp(result->m_header.id, "IMPM", 4) != 0 )
    {
        logger()->warn(L4CXX_LOCATION, "Header ID mismatch");
        return nullptr;
    }

    if( result->m_header.flags & ITHeader::FlgInstrumentMode )
    {
        logger()->info(L4CXX_LOCATION, "Instrument mode");
    }
    else
    {
        logger()->info(L4CXX_LOCATION, "Sample mode");
    }

    auto orders = stream->readVector<uint8_t>(result->m_header.ordNum);
    orders.resize(256, 0xff);

    const auto instrumentOffsets = stream->readVector<uint32_t>(result->m_header.insNum);
    const auto sampleOffsets = stream->readVector<uint32_t>(result->m_header.smpNum);
    const auto patternOffsets = stream->readVector<uint32_t>(result->m_header.patNum);

    if( (result->m_header.special & ITHeader::SFlgMidiEmbedded) != 0 )
    {
        *stream >> result->m_midiDataArea;
    }

    for( const auto& o : orders )
    {
        result->addOrder(std::make_unique<ItOrder>(o));
    }

    result->m_patterns.resize(result->m_header.patNum);

    result->m_instruments.resize(std::max(uint16_t(99), result->m_header.insNum));

    for( int i = 0; i < result->m_header.insNum; ++i )
    {
        stream->seek(instrumentOffsets[i]);
        *stream >> result->m_instruments[i];

        if( std::strncmp(result->m_instruments[i].id, "IMPI", 4) != 0 )
        {
            logger()->warn(L4CXX_LOCATION, "Instrument Header ID mismatch");
            return nullptr;
        }

        if( result->m_header.cmwt < 0x0200 )
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("old instruments not yet supported"));
            // convertOldInstrument(i);
        }
    }

    result->m_samples.resize(256);
    for( auto& smp : result->m_samples )
    {
        smp = std::make_unique<ItSample>();
    }

    for( int i = 0; i < result->m_header.smpNum; ++i )
    {
        stream->seek(sampleOffsets[i]);
        result->m_samples[i]->loadHeader(*stream);
        if( std::strncmp(result->m_samples[i]->header.id, "IMPS", 4) != 0 )
        {
            logger()->warn(L4CXX_LOCATION, "Sample Header ID mismatch");
            return nullptr;
        }
    }

    for( int i = 0; i < result->m_header.smpNum; ++i )
    {
        if( (result->m_samples[i]->header.flg & ItSampleHeader::FlgWithHeader) != 0 && result->m_samples[i]->header.samplePointer != 0 )
        {
            stream->seek(result->m_samples[i]->header.samplePointer);
            result->m_samples[i]->loadData(*stream);
        }
    }

    for( int i = 0; i < result->m_header.patNum; ++i )
    {
        if( patternOffsets[i] == 0 )
        {
            continue;
        }

        stream->seek(patternOffsets[i]);

        uint16_t patLen;
        *stream >> patLen;
        result->m_patterns[i] = stream->readVector<uint8_t>(patLen + 6);
        BOOST_ASSERT(stream->good());
    }

    if( (result->m_header.flags & ITHeader::FlgInstrumentMode) == 0 )
    {
        // pre-bind channels
        for( int i = 0; i < 64; ++i )
        {
            result->m_hosts[i].setSlave(&result->m_slaves[i]);
            result->m_slaves[i].setHost(&result->m_hosts[i]);
        }
    }

    result->setSpeed(result->m_header.is);
    result->setTempo(result->m_header.it);

    result->state().globalVolumeLimit = 0x80;
    result->state().globalVolume = result->m_header.gv;

    for( size_t i = 0; i < result->m_hosts.size(); ++i )
    {
        result->m_hosts[i].cp = result->m_header.chnPan[i] & 0x7f;
        result->m_hosts[i].channelVolume = result->m_header.chnVol[i];
        result->m_hosts[i].setSlave(&result->m_slaves[i]);
        result->m_slaves[i].setHost(&result->m_hosts[i]);
    }

    const uint16_t cwtV = result->m_header.cwtV;
    switch( cwtV & 0xf000u )
    {
        case 0x0000:
            result->noConstMetaInfo().trackerInfo = stringFmt("Impulse Tracker %X.%02X", (cwtV >> 8) & 0xf, cwtV & 0xff);
            break;
        case 0x1000:
        {
            auto schismVersion = cwtV & 0xfff;
            if( schismVersion <= 0x50 )
            {
                result->noConstMetaInfo().trackerInfo = stringFmt("Schism Tracker %X.%02X", (cwtV >> 8) & 0xf, cwtV & 0xff);
            }
            else
            {
                // > 0x50: the number of days since 2009-10-31

                tm epoch{};
                epoch.tm_year = 109;
                epoch.tm_mon = 9;
                epoch.tm_mday = 31;
                auto epochSec = mktime(&epoch);

                auto versionSec = ((schismVersion - 0x50) * 86400) + epochSec;
                tm version{};
                if( localtime_r(&versionSec, &version) )
                {
                    result->noConstMetaInfo().trackerInfo = stringFmt("Schism Tracker %04d-%02d-%02d", version.tm_year + 1900, version.tm_mon + 1,
                                                                      version.tm_mday);
                }
                else
                {
                    result->noConstMetaInfo().trackerInfo = stringFmt("Schism Tracker 0x%03x", schismVersion);
                }
            }
        }
            break;
        case 0x4000:
            result->noConstMetaInfo().trackerInfo = stringFmt("pyIT %X.%02X", (cwtV >> 8) & 0xf, cwtV & 0xff);
            break;
        case 0x5000:
            result->noConstMetaInfo().trackerInfo = stringFmt("OpenMPT %X.%02X", (cwtV >> 8) & 0xf, cwtV & 0xff);
            break;
        case 0x6000:
            result->noConstMetaInfo().trackerInfo = stringFmt("BeRoTracker %X.%02X", (cwtV >> 8) & 0xf, cwtV & 0xff);
            break;
        case 0x7000:
            if( cwtV == 0x7fff )
            {
                result->noConstMetaInfo().trackerInfo = "munch.py";
            }
            else
            {
                result->noConstMetaInfo().trackerInfo = stringFmt("ITMCK %d.%d.%d", (cwtV >> 8) & 0xf, (cwtV >> 4) & 0xf, cwtV & 0xf);
            }
            break;
        case 0x8000:
            result->noConstMetaInfo().trackerInfo = stringFmt("Tralala %X.%02X", (cwtV >> 8) & 0xf, cwtV & 0xff);
            break;
        case 0xc000:
            result->noConstMetaInfo().trackerInfo = stringFmt("ChickDune ChipTune Tracker %X.%02X", (cwtV >> 8) & 0xf, cwtV & 0xff);
            break;
        case 0xd000:
            if( cwtV == 0xdaeb )
            {
                result->noConstMetaInfo().trackerInfo = "spc2it";
            }
            else
            {
                result->noConstMetaInfo().trackerInfo = stringFmt("Unknown (0x%04x)", cwtV);
            }
            break;
        default:
            result->noConstMetaInfo().trackerInfo = stringFmt("Unknown (0x%04x)", cwtV);
    }
    result->noConstMetaInfo().filename = stream->name();
    result->noConstMetaInfo().title = stringncpy(result->m_header.name, 26);

    if( !result->initialize(frequency) )
    {
        return nullptr;
    }

    return result.release();
}

size_t ItModule::internal_buildTick(const AudioFrameBufferPtr& buffer)
{
    if( !update() )
    {
        logger()->info(L4CXX_LOCATION, "Update failed");
        setOrder(orderCount());
        return 0;
    }

    if( state().order >= orderCount() )
    {
        return 0;
    }

    if( orderAt(state().order)->playbackCount() >= maxRepeat() )
    {
        logger()->info(L4CXX_LOCATION, "Song end reached: Maximum repeat count reached");
        return 0;
    }

    if( !orderAt(state().order)->increaseRowPlayback(state().row) )
    {
        logger()->info(L4CXX_LOCATION, "Row playback counter reached limit");
        setOrder(orderCount());
        return 0;
    }

    MixerFrameBuffer mixBuffer;
    mixBuffer.resize(tickBufferLength());
    M32MixHandler(mixBuffer, buffer == nullptr);
    BOOST_ASSERT(mixBuffer.size() == tickBufferLength());

    if( buffer != nullptr )
    {
        buffer->clear();

        for( const auto& f : mixBuffer )
        {
            buffer->emplace_back(f.rightShiftClip(15));
        }
    }

    state().playedFrames += tickBufferLength();
    return tickBufferLength();
}

bool ItModule::update()
{
    for( auto& slave : m_slaves )
    {
        if( (slave.flags & SCFLG_ON) == 0 )
        {
            continue;
        }

        if( slave.effectiveVolume != slave.effectiveBaseVolume )
        {
            BOOST_ASSERT(slave.effectiveBaseVolume >= 0 && slave.effectiveBaseVolume <= 64);
            slave.effectiveVolume = slave.effectiveBaseVolume;
            slave.flags |= SCFLG_RECALC_VOL;
        }

        if( slave.frequency != slave.frequencySet )
        {
            slave.frequency = slave.frequencySet;
            slave.flags |= SCFLG_FREQ_CHANGE;
        }
    }

    if( !updateData() )
    {
        return false;
    }

    if( state().pattern >= m_patterns.size() )
    {
        return false;
    }

    if( (m_header.flags & ITHeader::FlgInstrumentMode) == 0 )
    {
        updateSamples();
    }
    else
    {
        updateInstruments();
    }

    return true;
}

bool ItModule::updateData()
{
    --m_tickCountdown;
    ++state().tick;
    if( m_tickCountdown != 0 )
    {
        for( auto& host : m_hosts )
        {
            if( host.isEnabled() && (host.flags & HCFLG_UPD_VOL_IF_ON) != 0 )
            {
                if( auto handler = volCommands[host.volumeFx & 7u] )
                {
                    (this->*handler)(host);
                }
            }

            if( (host.flags & HCFLG_UPD_MODE) == HCFLG_UPD_NEVER )
            {
                continue;
            }

            if( host.isEnabled() || (host.flags & HCFLG_UPD_ALWAYS) != 0 )
            {
                if( auto handler = commands[host.patternFx & 0x1fu] )
                {
                    (this->*handler)(host);
                }
            }
        }
        return true;
    }

    if( state().order >= orderCount() || state().pattern >= m_patterns.size() )
    {
        return false;
    }

    m_tickCountdown = state().speed;
    state().tick = 0;

    --m_rowDelay;
    if( m_rowDelay != 0 )
    {
        for( auto& host : m_hosts )
        {
            if( (host.flags & HCFLG_ROW_UPDATED) == 0 )
            {
                continue;
            }

            if( (host.cellMask & HCFLG_MSK_CMD) == 0 )
            {
                continue;
            }

            host.cellMask &= HCFLG_MSK_CMD;

            const auto msk = host.cellMask;
            if( auto handler = initCommands[host.patternFx & 0x1fu] )
            {
                (this->*handler)(host);
            }
            host.cellMask = msk;
        }
        return true;
    }

    m_rowDelay = 1;
    m_rowDelayActive = false;

    if( m_nextRow + 1u < m_patternRows )
    {
        m_nextRow = m_nextRow + 1u;
        setRow(m_nextRow);
        loadRow();
        return true;
    }

    uint16_t nextOrder = m_nextOrder + 1u;
    do
    {
        if( orderAt(nextOrder)->index() < 200 )
        {
            m_nextOrder = nextOrder;
            setOrder(nextOrder);
            m_nextRow = std::exchange(m_breakRow, 0);
            setRow(m_nextRow);
            if( state().pattern >= m_patterns.size() )
            {
                return false;
            }

            loadRow();
            return true;
        }

        ++nextOrder;

        if( nextOrder >= orderCount() || orderAt(nextOrder)->index() == 0xff )
        {
            return false;
        }
        else if( orderAt(nextOrder)->index() == 0xfe )
        {
            continue;
        }
    } while( nextOrder < orderCount() );

    return false;
}

void ItModule::loadRow()
{
    m_patternLooping = false;
    if( state().pattern != m_currentDecodingPattern )
    {
        goToProcessRow();
    }
    else
    {
        ++m_currentDecodingRow;
        if( state().row != m_currentDecodingRow )
        {
            goToProcessRow();
        }
    }

    for( auto& host : m_hosts )
    {
        host.flags &= ~(HCFLG_UPD_MODE | HCFLG_ROW_UPDATED | HCFLG_UPD_VOL_IF_ON);
    }

    // first clear all states for the case there's no pattern data
    for( auto& host : m_hosts )
    {
        host.channelState.noteTriggered = false;
        host.channelState.fx = 0;
        host.channelState.fxDesc = ppp::fxdesc::NullFx;

        host.channelState.cell = "... .. .. ...";
    }

    BOOST_ASSERT(m_patternDataPtr != nullptr);
    while( uint8_t cellHeader = *m_patternDataPtr++ )
    {
        BOOST_ASSERT((cellHeader & 0x7fu) > 0 && (cellHeader & 0x7fu) <= m_hosts.size());
        auto host = &m_hosts[(cellHeader & 0x7fu) - 1];

        host->channelState.cell.clear();

        if( (cellHeader & 0x80u) != 0 )
        {
            host->cellMask = *m_patternDataPtr++;
        }

        if( (host->cellMask & HCFLG_MSK_NOTE_1) != 0 )
        {
            host->patternNote = *m_patternDataPtr++;
        }

        if( (host->cellMask & HCFLG_MSK_NOTE) == 0 )
        {
            host->channelState.noteTriggered = false;
            host->channelState.cell += "... ";
        }
        else
        {
            if( host->patternNote == PATTERN_NOTE_CUT )
            {
                host->channelState.noteTriggered = true;
                host->channelState.cell += "^^^ ";
                host->channelState.note = ChannelState::NoteCut;
            }
            else if( host->patternNote > PATTERN_MAX_NOTE )
            {
                host->channelState.noteTriggered = true;
                host->channelState.cell += "=== ";
                host->channelState.note = ChannelState::KeyOff;
            }
            else if( host->getSlave() != nullptr && (host->getSlave()->flags & SCFLG_ON) != 0 )
            {
                host->channelState.noteTriggered = true;
                host->channelState.cell += stringFmt("%s%d ", ppp::NoteNames[host->patternNote % 12], host->patternNote / 12 + 0);
            }
            else
            {
                host->channelState.noteTriggered = false;
                host->channelState.cell += "... ";
            }
        }

        if( (host->cellMask & HCFLG_MSK_INS_1) != 0 )
        {
            host->patternInstrument = *m_patternDataPtr++;
        }

        if( (host->cellMask & HCFLG_MSK_INS) == 0 )
        {
            host->channelState.cell += ".. ";
        }
        else
        {
            host->channelState.cell += stringFmt("%02d ", int(host->patternInstrument));
        }

        if( (host->cellMask & HCFLG_MSK_VOL_1) != 0 )
        {
            host->patternVolume = *m_patternDataPtr++;
        }

        if( (host->cellMask & HCFLG_MSK_VOL) == 0 )
        {
            host->channelState.cell += ".. ";
        }
        else
        {
            host->channelState.cell += stringFmt("%02X ", int(host->patternVolume));
        }

        if( (host->cellMask & HCFLG_MSK_CMD_1) != 0 )
        {
            host->patternFx = host->lastPatternFx = *m_patternDataPtr++;
            host->patternFxParam = host->lastPatternFxParam = *m_patternDataPtr++;
        }
        else if( (host->cellMask & HCFLG_MSK_CMD_2) != 0 )
        {
            host->patternFx = host->lastPatternFx;
            host->patternFxParam = host->lastPatternFxParam;
        }
        else
        {
            host->patternFx = 0;
            host->patternFxParam = 0;
        }

        if( (host->cellMask & HCFLG_MSK_CMD) == 0 || host->patternFx == 0 )
        {
            host->channelState.cell += "...";
        }
        else
        {
            host->channelState.cell += stringFmt("%c%02X", char('A' + (host->patternFx & 0x1f) - 1), int(host->patternFxParam));
        }

        onCellLoaded(*host);
    }

    for( auto& host : m_hosts )
    {
        host.channelState.active = (host.flags & HCFLG_ON) != 0;
        if( host.getSlave() == nullptr || !host.channelState.active )
        {
            host.channelState.note = ChannelState::NoNote;
            continue;
        }

        auto slave = host.getSlave();
        auto smp = slave->smpOffs;
        BOOST_ASSERT(smp != nullptr);

        auto exp = std::log2(double(host.getSlave()->frequency) / smp->header.c5speed);
        auto note = std::lround((exp * 12) + 5 * 12);
        if( note < 0 )
        {
            host.channelState.note = ChannelState::TooLow;
        }
        else if( note > PATTERN_MAX_NOTE )
        {
            host.channelState.note = ChannelState::TooHigh;
        }
        else
        {
            host.channelState.note = static_cast<uint8_t>(note);
        }

        if( (m_header.flags & ITHeader::FlgInstrumentMode) == 0 )
        {
            host.channelState.instrument = slave->smp;
            host.channelState.instrumentName = smp->title();
        }
        else if( (m_header.flags & ITHeader::FlgInstrumentMode) != 0 && slave->insOffs != nullptr )
        {
            host.channelState.instrument = slave->ins;
            host.channelState.instrumentName = stringncpy(slave->insOffs->name, 26);
        }
        else
        {
            host.channelState.instrumentName.clear();
        }

        host.channelState.volume = slave->_16bVol * 100 / 32768;
        if( slave->finalPan != SurroundPan )
        {
            host.channelState.panning = (slave->finalPan - 32) * 100 / 32;
        }
        else
        {
            host.channelState.panning = ChannelState::Surround;
        }
    }
}

void ItModule::goToProcessRow()
{
    static const std::vector<uint8_t> emptyPattern{
        64, 0, 64, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    m_currentDecodingPattern = state().pattern;

    const auto& pattern = m_patterns.at(state().pattern);

    const auto& patternData = pattern.empty() ? emptyPattern : pattern;

    m_patternRows = *reinterpret_cast<const uint16_t*>(patternData.data());

    if( m_patternRows <= m_nextRow )
    {
        m_nextRow = 0;
    }

    BOOST_ASSERT(m_nextRow >= 0);
    setRow(m_nextRow);
    m_currentDecodingRow = m_nextRow;

    m_patternDataPtr = patternData.data() + 6; // skip rowcount + dummy data

    for( int i = 0; i < m_nextRow; ++i )
    {
        while( const auto chn = *m_patternDataPtr++ )
        {
            auto host = &m_hosts[(chn & 0x7fu) - 1];

            if( (chn & 0x80u) != 0 )
            {
                host->cellMask = *m_patternDataPtr++;
            }

            if( (host->cellMask & HCFLG_MSK_NOTE_1) != 0 )
            {
                host->patternNote = *m_patternDataPtr++;
            }

            if( (host->cellMask & HCFLG_MSK_INS_1) != 0 )
            {
                host->patternInstrument = *m_patternDataPtr++;
            }

            if( (host->cellMask & HCFLG_MSK_VOL_1) != 0 )
            {
                host->patternVolume = *m_patternDataPtr++;
            }

            if( (host->cellMask & HCFLG_MSK_CMD_1) != 0 )
            {
                host->lastPatternFx = *m_patternDataPtr++;
                host->lastPatternFxParam = *m_patternDataPtr++;
            }
        }
    }
    BOOST_ASSERT(m_patternDataPtr <= &patternData[patternData.size()]);
}

void ItModule::onCellLoaded(HostChannel& host)
{
    if( (host.cellMask & (HCFLG_MSK_NOTE | HCFLG_MSK_INS)) != 0 )
    {
        if( (m_header.flags & ITHeader::FlgInstrumentMode) == 0 )
        {
            host.effectiveNote = host.patternNote;
            host.sampleIndex = host.patternInstrument;
        }
        else if( host.patternNote > PATTERN_MAX_NOTE || host.patternInstrument == 0xff )
        {
            host.effectiveNote = host.patternNote;
            host.sampleIndex = host.patternInstrument;
        }
        else
        {
            BOOST_ASSERT(host.patternInstrument > 0 && host.patternInstrument <= m_instruments.size());
            const auto& instrument = m_instruments[host.patternInstrument - 1];

            if( instrument.mch != 0 )
            {
                if( instrument.mch == 17 )
                {
                    host.midiChannel = (std::distance(&m_hosts[0], &host) & 0x0fu) + 1u;
                }
                else
                {
                    host.midiChannel = instrument.mch;
                }
                host.midiProgram = instrument.mpr;
            }

            host.sampleIndex = instrument.keyboardTable[host.patternNote].sample;
            host.effectiveNote = instrument.keyboardTable[host.patternNote].note;

            if( host.sampleIndex == 0 )
            {
                return;
            }
        }
    }

    if( auto handler = initCommands[host.patternFx & 0x1fu] )
    {
        (this->*handler)(host);
    }

    host.flags |= HCFLG_ROW_UPDATED;
    if( (m_header.chnPan[std::distance(&m_hosts[0], &host)] & 0x80u) == 0 )
    {
        return;
    }

    if( !host.isEnabled() )
    {
        return;
    }

    host.getSlave()->flags |= SCFLG_MUTED;
}

void ItModule::updateSamples()
{
    for( auto& slave : m_slaves )
    {
        if( (slave.flags & SCFLG_ON) == 0 )
        {
            continue;
        }

        if( (slave.flags & SCFLG_RECALC_VOL) != 0 )
        {
            slave.flags &= ~SCFLG_RECALC_VOL;
            slave.flags |= SCFLG_RECALC_FINAL_VOL;

            auto totalVolume =
                uint32_t(slave.effectiveVolume) * slave.channelVolume * slave.sampleVolume / 16u * state().globalVolume / 128u;
            BOOST_ASSERT(totalVolume <= 32768);
            slave._16bVol = static_cast<uint16_t>(totalVolume);
        }

        if( (slave.flags & SCFLG_RECALC_PAN) != 0 )
        {
            slave.flags &= ~SCFLG_RECALC_PAN;
            slave.flags |= SCFLG_PAN_CHANGE;

            slave.midiFinalPan = slave.pan;
            if( slave.pan != SurroundPan )
            {
                int tmp = (slave.pan - 32) * m_header.sep / 128 + 32;
                slave.finalPan = static_cast<uint8_t>(tmp);
                BOOST_ASSERT(slave.finalPan <= 64);
            }
            else
            {
                slave.finalPan = SurroundPan;
            }
        }

        updateVibrato(slave);
    }
}

void ItModule::updateVibrato(SlaveChannel& slave)
{
    if( slave.smpOffs->header.vid == 0 || slave.smpOffs->header.vis == 0 )
    {
        return;
    }

    slave.vip += slave.smpOffs->header.vis;

    auto depth = static_cast<uint16_t>((slave.viDepth + slave.smpOffs->header.vir) / 256u);
    if( depth > slave.smpOffs->header.vid )
    {
        depth = slave.smpOffs->header.vid;
    }

    slave.viDepth = depth;

    int value;
    switch( slave.smpOffs->header.vit )
    {
        case 0:
            value = fineSineData[slave.vip];
            break;
        case 1:
            value = fineRampDownData[slave.vip];
            break;
        case 2:
            value = fineSquareWave[slave.vip];
            break;
        case 3:
        default:
            value = (std::rand() & 0x7f) - 64;
    }

    value *= depth * 4;
    value /= 256;
    if( value < 0 )
    {
        pitchSlideDownLinear(slave, -value);
    }
    else if( value > 0 )
    {
        pitchSlideUpLinear(slave, value);
    }
}

void ItModule::updateInstruments()
{
    for( auto& slave : m_slaves )
    {
        if( (slave.flags & SCFLG_ON) == 0 )
        {
            continue;
        }

        slave.envFilterCutoff = 256;

        auto slaveFlags = slave.flags;
        if( slave.ins != 0 )
        {
            const auto isNoteOff = (slaveFlags & SCFLG_NOTE_OFF) != 0;

            if( (slaveFlags & SCFLG_PITCH_ENV) != 0 )
            {
                if( !updateEnvelope(slave.ptEnvelope, slave.insOffs->pitchEnv, isNoteOff) )
                {
                    slaveFlags &= ~SCFLG_PITCH_ENV;
                }
            }

            if( slave.insOffs->pitchEnv.hasFilter() )
            {
                slave.envFilterCutoff = slave.ptEnvelope.value * 256 / 64 + 128; // Range 0 -> 256
                BOOST_ASSERT(slave.envFilterCutoff >= 0 && slave.envFilterCutoff <= 256);
                slaveFlags |= SCFLG_RECALC_FINAL_VOL;
            }
            else
            {
                auto delta = slave.ptEnvelope.value * 256 / 8;
                BOOST_ASSERT(delta >= -1024 && delta <= 1024);
                if( delta < 0 )
                {
                    pitchSlideDownLinear(slave, -delta);
                    slaveFlags |= SCFLG_FREQ_CHANGE;
                }
                else if( delta > 0 )
                {
                    pitchSlideUpLinear(slave, delta);
                    slaveFlags |= SCFLG_FREQ_CHANGE;
                }
            }

            if( (slaveFlags & SCFLG_PAN_ENV) != 0 )
            {
                slaveFlags |= SCFLG_RECALC_PAN;

                if( !updateEnvelope(slave.pEnvelope, slave.insOffs->panEnv, isNoteOff) )
                {
                    slaveFlags &= ~SCFLG_PAN_ENV;
                }
            }

            if( (slaveFlags & SCFLG_VOL_ENV) != 0 )
            {
                slaveFlags |= SCFLG_RECALC_VOL;

                if( updateEnvelope(slave.vEnvelope, slave.insOffs->volEnv, isNoteOff) )
                {
                    if( (slaveFlags & SCFLG_FADEOUT) != 0 || (isNoteOff && slave.insOffs->volEnv.hasGlobalLoop()) )
                    {
                        goto UpdateInstruments13_fadeout;
                    }

                    goto UpdateInstruments5_recalcVol; // Volume calculation
                }

                // Envelope turned off...
                slaveFlags &= ~SCFLG_VOL_ENV;

                if( slave.vEnvelope.value == 0 )
                {
                    if( !slave.disowned )
                    {
                        slave.disowned = true;
                        slave.getHost()->disable();
                    }

                    slaveFlags |= SCFLG_NOTE_CUT;

                    goto UpdateInstruments5_recalcVol;
                }
            }
            else
            {
                if( (slaveFlags & SCFLG_FADEOUT) != 0 )
                {
                    goto UpdateInstruments13_fadeout;
                }

                // Also apply fade if No vol env AND sustain off

                if( (slaveFlags & SCFLG_NOTE_OFF) == 0 )
                {
                    goto UpdateInstruments5_recalcVol;
                }
            }

UpdateInstruments13_fadeout:
            slaveFlags |= SCFLG_FADEOUT;
            slave.fadeOut -= slave.insOffs->fadeout;
            if( slave.fadeOut <= 0 )
            {
                slave.fadeOut = 0;

                // Turn off channel
                if( !slave.disowned )
                {
                    slave.disowned = true;
                    slave.getHost()->disable();
                }

                slaveFlags |= SCFLG_NOTE_CUT;
            }

            slaveFlags |= SCFLG_RECALC_VOL;
        }

UpdateInstruments5_recalcVol:
        if( (slaveFlags & SCFLG_RECALC_VOL) != 0 )
        {
            slaveFlags &= ~SCFLG_RECALC_VOL;
            slaveFlags |= SCFLG_RECALC_FINAL_VOL;
            int volume = slave.effectiveVolume * slave.channelVolume; // AX = (0->4096)
            BOOST_ASSERT(volume >= 0 && volume <= 64 * 64);

            volume *= slave.fadeOut; // Fadeout Vol (0->1024), DX:AX = 0->4194304
            BOOST_ASSERT(volume >= 0 && volume <= 64 * 64 * 1024);

            volume /= 128; // AX = (0->32768)
            BOOST_ASSERT(volume >= 0 && volume <= 64 * 64 * 8);

            volume *= slave.sampleVolume; // DX:AX = 0->4194304
            volume /= 128; // AX = 0->32768
            BOOST_ASSERT(volume >= 0 && volume <= 64 * 64 * 8);

            BOOST_ASSERT(slave.vEnvelope.value >= 0 && slave.vEnvelope.value <= 64);
            volume *= slave.vEnvelope.value;
            volume /= 64;
            BOOST_ASSERT(volume >= 0 && volume <= 64 * 64 * 8);

            volume *= state().globalVolume;
            volume /= 128;
            BOOST_ASSERT(volume >= 0 && volume <= 32768);

            slave._16bVol = volume;
        }

        if( (slaveFlags & SCFLG_RECALC_PAN) != 0 )
        {
            slaveFlags &= ~SCFLG_RECALC_PAN;
            slaveFlags |= SCFLG_PAN_CHANGE;

            if( slave.pan == SurroundPan )
            {
                slave.midiFinalPan = SurroundPan;
                slave.finalPan = SurroundPan;
            }
            else
            {
                auto value = 32 - std::abs(32 - slave.pan);
                BOOST_ASSERT(value >= -32 && value <= 32);
                value = value * slave.pEnvelope.value / 32;
                BOOST_ASSERT(value >= -32 && value <= 32);
                value += slave.pan;
                BOOST_ASSERT(value >= 0 && value <= 64);

                slave.midiFinalPan = value;
                value -= 32;
                BOOST_ASSERT(value >= -32 && value <= 32);
                value *= slave.insOffs->pitchPanSeparation / 2; // AX = -2048->+2048
                BOOST_ASSERT(value >= -2048 && value <= 2048);
                value /= 64; // AL = -32->+32
                BOOST_ASSERT(value >= -32 && value <= 32);
                value += 32;
                slave.finalPan = value;
            }
        }

        slave.flags = slaveFlags;

        updateVibrato(slave);
    }
}

void ItModule::midiTranslateParametrized(HostChannel* host, SlaveChannel* slave, const MidiMacro& macro)
{
    uint8_t data = 0;
    bool inNybble = false;
    std::vector<uint8_t> sendBuffer;
    BOOST_ASSERT(macro.back() == 0);
    for( const char chr : macro )
    {
        if( chr == ' ' || chr == 0 )
        {
            if( inNybble )
            {
                sendBuffer.emplace_back(std::exchange(data, 0));
                inNybble = false;
            }
            continue;
        }
        else if( chr >= '0' && chr <= '9' )
        {
            data <<= 4u;
            data |= (chr - '0');
            if( inNybble )
            {
                sendBuffer.emplace_back(std::exchange(data, 0));
                inNybble = false;
            }
            else
            {
                inNybble = true;
            }
            continue;
        }
        else if( chr >= 'A' && chr <= 'F' )
        {
            data <<= 4u;
            data |= (chr - 'A' + 10);
            if( inNybble )
            {
                sendBuffer.emplace_back(std::exchange(data, 0));
                inNybble = false;
            }
            else
            {
                inNybble = true;
            }
            continue;
        }
        else if( chr == 'c' )
        {
            if( slave == nullptr )
            {
                continue;
            }

            data <<= 4u;
            data |= (slave->mch - 1);

            if( inNybble )
            {
                sendBuffer.emplace_back(std::exchange(data, 0));
                inNybble = false;
            }
            else
            {
                inNybble = true;
            }
            continue;
        }

        if( inNybble )
        {
            sendBuffer.emplace_back(std::exchange(data, 0));
            inNybble = false;
        }

        if( chr == 'z' )
        {
            sendBuffer.emplace_back(host->patternFxParam);
        }
        else if( chr == 'o' )
        {
            sendBuffer.emplace_back(host->o00);
        }

        if( slave == nullptr )
        {
            continue;
        }

        if( chr == 'n' )
        {
            sendBuffer.emplace_back(slave->effectiveNote);
        }
        else if( chr == 'm' )
        {
            sendBuffer.emplace_back(slave->loopDirBackward ? 1 : 0);
        }
        else if( chr == 'v' )
        {
            if( (slave->flags & SCFLG_MUTED) != 0 )
            {
                sendBuffer.emplace_back(0);
            }
            else
            {
                auto tmp = static_cast<uint8_t>(uint32_t(slave->effectiveBaseVolume) * state().globalVolume * slave->channelVolume / 16 * slave->sampleVolume
                                                / 32768);

                if( tmp == 0 )
                {
                    tmp = 1;
                }
                else if( data >= 0x80 )
                {
                    tmp = 0x7f;
                }
                sendBuffer.emplace_back(tmp);
            }
        }
        else if( chr == 'u' )
        {
            if( (slave->flags & SCFLG_MUTED) != 0 )
            {
                sendBuffer.emplace_back(0);
            }
            else
            {
                auto tmp = static_cast<uint8_t>(slave->_16bVol / 256u);

                if( tmp == 0 )
                {
                    tmp = 1;
                }
                else if( data >= 0x80 )
                {
                    tmp = 0x7f;
                }
                sendBuffer.emplace_back(tmp);
            }
        }
        else if( chr == 'h' )
        {
            sendBuffer.emplace_back(static_cast<uint8_t>(std::distance(&m_hosts[0], slave->getHost())));
        }
        else if( chr == 'x' || chr == 'y' )
        {
            auto tmp = 0;
            switch( chr )
            {
                case 'x':
                    tmp = slave->pan;
                    break;
                case 'y':
                    tmp = slave->midiFinalPan;
                    break;
                default:
                    BOOST_ASSERT(false);
            }
            tmp *= 2;
            if( tmp == 0x80 )
            {
                tmp = 0x7f;
            }
            else if( tmp > 0x80 )
            {
                tmp = 0x40;
            }
            sendBuffer.emplace_back(tmp);
        }
        else if( chr == 'p' )
        {
            sendBuffer.emplace_back(slave->midiProgram);
        }
        else if( chr == 'b' )
        {
            sendBuffer.emplace_back(slave->midiBank & 0xff);
        }
        else if( chr == 'a' )
        {
            sendBuffer.emplace_back(slave->midiBank >> 8u);
        }
    }

    if( slave != nullptr && sendBuffer.size() >= 4 && sendBuffer[0] == 0xf0 && sendBuffer[1] == 0xf0 )
    {
        if( sendBuffer[2] == 0 )
        {
            slave->filterCutoff = sendBuffer[3] & 0x7fu;
        }
        else if( sendBuffer[2] == 1 )
        {
            slave->filterResonance = sendBuffer[3] & 0x7fu;
        }
    }
}

void ItModule::initNoCommand(HostChannel& host)
{
    host.channelState.fx = 0;
    host.channelState.fxDesc = ppp::fxdesc::NullFx;

    if( (host.cellMask & (HCFLG_MSK_NOTE | HCFLG_MSK_INS)) != 0 )
    {
        if( host.effectiveNote > PATTERN_MAX_NOTE )
        {
            if( host.isEnabled() )
            {
                if( host.effectiveNote == PATTERN_NOTE_OFF )
                {
                    host.getSlave()->flags |= SCFLG_NOTE_OFF;
                    goto InitNoCommand11;
                }
                else if( host.effectiveNote == PATTERN_NOTE_CUT )
                {
                    host.disable();
                    host.getSlave()->flags |= SCFLG_NOTE_CUT;
                }
                else
                {
                    host.getSlave()->flags |= SCFLG_FADEOUT;
                }
            }
        }
        else if( host.isEnabled()
                 && (host.cellMask & HCFLG_MSK_NOTE) == 0
                 && host.patternNote == host.getSlave()->effectiveNote
                 && host.patternInstrument == host.getSlave()->ins )
        {
        }
        else if( host.isEnabled()
                 && (host.cellMask & HCFLG_MSK_VOL) != 0
                 && host.volumeFx >= 193
                 && host.volumeFx <= 202 )
        {
            initVolumeEffect(host); // porta/vibrato vol cmd
            return;
        }
        else if( allocateChannel(host) )
        {
            BOOST_ASSERT(host.vse >= 0 && host.vse <= 64);
            host.getSlave()->effectiveVolume = host.vse;
            host.getSlave()->effectiveBaseVolume = host.vse;

            if( (m_header.flags & ITHeader::FlgInstrumentMode) == 0 && (host.getSlave()->smpOffs->header.dfp & 0x80u) != 0 )
            {
                host.cp = host.getSlave()->smpOffs->header.dfp & 0x7fu;
                BOOST_ASSERT(host.cp <= 64 || host.cp == SurroundPan);
                host.getSlave()->pan = host.cp;
                host.getSlave()->ps = host.cp;
            }

            host.getSlave()->sampleOffset = 0;
            host.getSlave()->loopDirBackward = false;
            host.getSlave()->frequency = host.getSlave()->smpOffs->header.c5speed
                                         * std::pow(2.0f, (host.effectiveNote - 5 * 12) / 12.0f);
            host.getSlave()->frequencySet = host.getSlave()->frequency;

            host.enable();
            host.flags &= ~HCFLG_SLIDE;

InitNoCommand11:
            host.getSlave()->applySampleLoop();
            if( (host.cellMask & (HCFLG_MSK_INS | HCFLG_MSK_VOL)) == 0 )
            {
                goto LBL_volFx;
            }

            if( (m_header.flags & (ITHeader::FlgInstrumentMode | ITHeader::FlgOldEffects))
                == (ITHeader::FlgInstrumentMode | ITHeader::FlgOldEffects)
                && (host.cellMask & HCFLG_MSK_INS) != 0
                && host.patternInstrument != 0xff )
            {
                BOOST_ASSERT(host.patternInstrument > 0 && host.patternInstrument <= m_instruments.size());
                host.getSlave()->fadeOut = 0x400;
                host.getSlave()->setInstrument(*this, m_instruments[host.patternInstrument - 1], m_lastSlaveChannel);
            }
        }
    }

    if( (host.cellMask & HCFLG_MSK_VOL) != 0 )
    {
        if( host.patternVolume <= 64 )
        {
            host.vse = host.patternVolume;
            goto LBL_setVolume;
        }
        else if( (host.patternVolume & 0x7fu) <= 64 )
        {
            setPan(host, host.patternVolume & 0x7fu);
        }
    }

    if( (host.cellMask & HCFLG_MSK_INS) != 0 && host.sampleIndex != 0 )
    {
        host.vse = m_samples[host.sampleIndex - 1]->header.vol;
        BOOST_ASSERT(host.vse >= 0 && host.vse <= 64);
    }
    else
    {
        goto LBL_volFx;
    }

LBL_setVolume:
    if( host.isEnabled() )
    {
        BOOST_ASSERT(host.vse >= 0 && host.vse <= 64);
        host.getSlave()->effectiveVolume = host.vse;
        host.getSlave()->effectiveBaseVolume = host.vse;
        host.getSlave()->flags |= SCFLG_RECALC_VOL;
    }

LBL_volFx:
    if( (host.flags & HCFLG_RANDOM) != 0 )
    {
        applyRandomValues(host);
    }

    initVolumeEffect(host);
}

void ItModule::initCommandA(HostChannel& host)
{
    host.channelState.fx = 'A';
    host.channelState.fxDesc = ppp::fxdesc::SetTempo;

    int newSpeed = host.patternFxParam;
    if( newSpeed != 0 )
    {
        m_tickCountdown += newSpeed - state().speed;

        setSpeed(static_cast<uint8_t>(newSpeed));
    }

    initNoCommand(host);
}

void ItModule::initCommandB(HostChannel& host)
{
    host.channelState.fx = 'B';
    host.channelState.fxDesc = ppp::fxdesc::JumpOrder;

    if( host.patternFxParam <= state().order )
    {
        // FIXME
        //BOOST_THROW_EXCEPTION(std::runtime_error("Need to stop song"));
        setOrder(orderCount());
        return;
    }

    m_nextOrder = static_cast<int16_t>(host.patternFxParam - 1);
    m_nextRow = -2;
    initNoCommand(host);
}

void ItModule::initCommandC(HostChannel& host)
{
    host.channelState.fx = 'C';
    host.channelState.fxDesc = ppp::fxdesc::PatternBreak;

    if( !m_patternLooping )
    {
        m_breakRow = host.patternFxParam;
        m_nextRow = -2;
    }

    initNoCommand(host);
}

void ItModule::initCommandDKL(HostChannel& host)
{
    host.getSlave()->flags |= SCFLG_RECALC_VOL;

    const auto hiNybble = static_cast<uint8_t>((host.dkl & 0xf0u) >> 4u);
    const auto loNybble = static_cast<uint8_t>(host.dkl & 0x0fu);

    if( loNybble == 0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::VolSlideUp;

        host.vch = hiNybble;
        host.flags |= HCFLG_UPD_IF_ON;
        if( hiNybble == 0x0f )
        {
            commandD(host);
        }

        return;
    }
    else if( hiNybble == 0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::VolSlideDown;

        host.vch = -int8_t(loNybble);
        host.flags |= HCFLG_UPD_IF_ON;
        if( loNybble == 0x0f )
        {
            commandD(host);
        }

        return;
    }

    int volume = host.getSlave()->effectiveBaseVolume;
    if( loNybble == 0x0f )
    {
        host.channelState.fxDesc = ppp::fxdesc::SlowVolSlideUp;

        host.vch = 0;
        volume += hiNybble;
        if( volume > 64 )
        {
            volume = 64;
        }
    }
    else if( hiNybble == 0x0f )
    {
        host.channelState.fxDesc = ppp::fxdesc::SlowVolSlideDown;

        host.vch = 0;
        volume -= loNybble;
        if( volume < 0 )
        {
            volume = 0;
        }
    }

    BOOST_ASSERT(volume >= 0 && volume <= 64);
    host.getSlave()->effectiveVolume = static_cast<uint8_t>(volume);
    host.getSlave()->effectiveBaseVolume = static_cast<uint8_t>(volume);
    host.vse = static_cast<uint8_t>(volume);
}

void ItModule::initCommandD(HostChannel& host)
{
    initNoCommand(host);

    host.channelState.fx = 'D';

    if( host.patternFxParam != 0 )
    {
        host.dkl = host.patternFxParam;
    }

    if( !host.isEnabled() )
    {
        return;
    }

    initCommandDKL(host);
}

void ItModule::initCommandE(HostChannel& host)
{
    initNoCommand(host);

    host.channelState.fx = 'E';

    if( host.patternFxParam != 0 )
    {
        host.efg = host.patternFxParam;
    }

    if( !host.isEnabled() )
    {
        return;
    }

    if( host.efg == 0 )
    {
        return;
    }

    const auto hiNybble = host.efg & 0xf0u;
    if( hiNybble < 0xe0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::FastPitchSlideDown;
        host.slideSpeed = uint16_t(host.efg * 4);
        host.flags |= HCFLG_UPD_IF_ON;
        return;
    }

    uint16_t value = static_cast<uint16_t>(host.efg & 0x0fu);
    if( value == 0 )
    {
        return;
    }

    if( hiNybble != 0xe0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::PitchSlideDown;
        value *= 4;
    }
    else
    {
        host.channelState.fxDesc = ppp::fxdesc::SlowPitchSlideDown;
    }

    pitchSlideDown(*host.getSlave(), value);

    host.getSlave()->frequencySet = host.getSlave()->frequency;
}

void ItModule::initCommandF(HostChannel& host)
{
    initNoCommand(host);

    host.channelState.fx = 'F';

    if( host.patternFxParam != 0 )
    {
        host.efg = host.patternFxParam;
    }

    if( !host.isEnabled() )
    {
        return;
    }

    if( host.efg == 0 )
    {
        return;
    }

    const auto hiNybble = host.efg & 0xf0u;
    if( hiNybble < 0xe0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::FastPitchSlideUp;
        // regular slide
        host.slideSpeed = uint16_t(host.efg * 4);
        host.flags |= HCFLG_UPD_IF_ON;
        return;
    }

    uint16_t value = static_cast<uint16_t>(host.efg & 0x0fu);
    if( value == 0 )
    {
        return;
    }

    if( hiNybble != 0xe0 )
    { // fine slide
        host.channelState.fxDesc = ppp::fxdesc::PitchSlideUp;
        value *= 4;
    }
    else
    {
        host.channelState.fxDesc = ppp::fxdesc::SlowPitchSlideUp;
    }

    pitchSlideUp(host, value);

    host.getSlave()->frequencySet = host.getSlave()->frequency;
}

void ItModule::initCommandG(HostChannel& host)
{
    // Check whether channel on/owned
    if( host.patternFxParam != 0 )
    {
        if( (m_header.flags & ITHeader::FlgLinkedEffects) == 0 )
        {
            host.linkedPortaFxValue = host.patternFxParam;
        }
        else
        {
            host.efg = host.patternFxParam;
        }
    }

    if( !host.isEnabled() )
    {
        initNoCommand(host);

        host.channelState.fx = 'G';
        host.channelState.fxDesc = ppp::fxdesc::Porta;
        return;
    }

    initPorta(host);

    host.channelState.fx = 'G';
    host.channelState.fxDesc = ppp::fxdesc::Porta;
}

// InitcommandG11
void ItModule::initPorta(HostChannel& host)
{
    auto slave = host.getSlave();
    if( (host.cellMask & HCFLG_MSK_INS) != 0 && host.sampleIndex != 0 )
    {
        if( (m_header.flags & ITHeader::FlgLinkedEffects) != 0 )
        {
            host.sampleIndex = slave->smp + 1;
            slave->sampleVolume = m_samples[slave->smp]->header.gvl * 2;
            BOOST_ASSERT(slave->sampleVolume <= 128);
        }
        else
        {
            slave->effectiveNote = host.patternNote;
            if( slave->ins != std::exchange(slave->ins, host.patternInstrument)
                && host.sampleIndex - 1 != slave->smp )
            {
                if( !applySample(host, slave) )
                {
                    return;
                }
            }
        }

        // apply instrument
        if( (m_header.flags & ITHeader::FlgInstrumentMode) != 0 )
        {
            slave->fadeOut = 1024;

            BOOST_ASSERT(host.patternInstrument > 0 && host.patternInstrument <= m_instruments.size());
            auto instrument = &m_instruments[host.patternInstrument - 1];
            slave->setInstrument(*this, *instrument, m_lastSlaveChannel);

            if( (slave->flags & SCFLG_ON) != 0 )
            {
                slave->flags &= ~SCFLG_NEW_NOTE;
            }

            slave->sampleVolume = (uint32_t(instrument->gbv) * slave->sampleVolume) / 128u;
            BOOST_ASSERT(slave->sampleVolume <= 128);
        }
    }

    if( (host.cellMask & HCFLG_MSK_NOTE) != 0 )
    {
        if( host.effectiveNote <= PATTERN_MAX_NOTE )
        {
            slave->effectiveNote = host.effectiveNote;

            host.portaTargetFrequency = slave->smpOffs->header.c5speed
                                        * std::pow(2.0f, (host.effectiveNote - 5 * 12) / 12.0f);
            host.flags |= HCFLG_SLIDE;
        }
        else if( host.isEnabled() )
        {
            if( host.effectiveNote == PATTERN_NOTE_OFF )
            {
                slave->flags |= SCFLG_NOTE_OFF;
                slave->applySampleLoop();
            }
            else if( host.effectiveNote == PATTERN_NOTE_CUT )
            {
                host.disable();
                slave->flags = SCFLG_NOTE_CUT;
            }
            else
            {
                slave->flags |= SCFLG_FADEOUT;
            }
        }
    }

    bool volumeAppliedFromCell = false;
    if( (host.cellMask & HCFLG_MSK_VOL) != 0 )
    {
        if( host.patternVolume <= 64 )
        {
            setVolume(host, host.patternVolume);
            volumeAppliedFromCell = true;
        }
        else if( (host.patternVolume & 0x7f) <= 64 )
        {
            setPan(host, host.patternVolume & 0x7f);
        }
    }

    if( !volumeAppliedFromCell && (host.cellMask & HCFLG_MSK_INS) != 0 )
    {
        setVolume(host, slave->smpOffs->header.vol);
    }

    if( (host.flags & HCFLG_SLIDE) != 0 )
    {
        uint16_t value;
        if( (m_header.flags & ITHeader::FlgLinkedEffects) == 0 )
        {
            value = host.linkedPortaFxValue;
        }
        else
        {
            value = host.efg;
        }

        if( value != 0 )
        {
            host.slideSpeed = uint16_t(value * 4);

            if( host.portaTargetFrequency != slave->frequencySet )
            {
                if( (host.flags & HCFLG_UPD_VOL_IF_ON) == 0 )
                {
                    host.flags |= HCFLG_UPD_IF_ON;
                }
            }
        }
    }

    // Don't call volume effects if it has a Gxx!
    if( (host.flags & HCFLG_UPD_VOL_IF_ON) == 0 )
    {
        initVolumeEffect(host);
    }
}

void ItModule::initCommandH(HostChannel& host)
{
    if( (host.cellMask & HCFLG_MSK_NOTE) != 0 && host.patternNote <= PATTERN_MAX_NOTE )
    {
        host.vibratoPosition = 0;
        host.lvi = 0;
    }

    if( host.patternFxParam != 0 )
    {
        const auto speed = host.patternFxParam & 0xf0u;
        auto depth = host.patternFxParam & 0x0fu;

        if( speed != 0 )
        {
            host.vsp = speed / 4u;
        }

        depth *= 4;
        if( depth != 0 )
        {
            if( (m_header.flags & ITHeader::FlgOldEffects) != 0 )
            {
                depth *= 2;
            }

            host.vdp = depth;
        }
    }

    initNoCommand(host);

    host.channelState.fx = 'H';
    host.channelState.fxDesc = ppp::fxdesc::Vibrato;

    if( !host.isEnabled() )
    {
        return;
    }

    host.flags |= HCFLG_UPD_IF_ON;

    initVibrato(host);
}

void ItModule::initCommandI(HostChannel& host)
{
    initNoCommand(host);

    host.channelState.fx = 'I';
    host.channelState.fxDesc = ppp::fxdesc::Tremor;

    if( host.patternFxParam != 0 )
    {
        host.i00 = host.patternFxParam;
    }

    if( !host.isEnabled() )
    {
        return;
    }

    host.flags |= HCFLG_UPD_IF_ON;

    uint8_t ontime = (host.i00 & 0xf0) >> 4;
    uint8_t offtime = host.i00 & 0x0f;

    if( (m_header.flags & ITHeader::FlgOldEffects) != 0 )
    {
        ++ontime;
        ++offtime;
    }

    host.tremorOnTime = ontime;
    host.tremorOffTime = offtime;

    commandI(host);
}

void ItModule::initCommandJ(HostChannel& host)
{
    initNoCommand(host);

    host.channelState.fx = 'J';
    host.channelState.fxDesc = ppp::fxdesc::Arpeggio;

    host.arpeggioStage = 0;

    if( host.patternFxParam != 0 )
    {
        host.j00 = host.patternFxParam;
    }

    if( !host.isEnabled() )
    {
        return;
    }

    host.flags |= HCFLG_UPD_IF_ON;

    host.arpeggioStage1 = std::pow(2, (host.j00 >> 4) / 12.0f);
    host.arpeggioStage2 = std::pow(2, (host.j00 & 0x0f) / 12.0f);
}

void ItModule::initCommandK(HostChannel& host)
{
    if( host.patternFxParam != 0 )
    {
        host.dkl = host.patternFxParam;
    }

    initNoCommand(host);

    host.channelState.fx = 'K';
    host.channelState.fxDesc = ppp::fxdesc::VibVolSlide;

    if( !host.isEnabled() )
    {
        return;
    }

    initVibrato(host);
    initCommandDKL(host);
    host.flags |= HCFLG_UPD_ALWAYS;
}

void ItModule::initCommandL(HostChannel& host)
{
    host.channelState.fx = 'L';
    host.channelState.fxDesc = ppp::fxdesc::PortaVolSlide;

    if( host.patternFxParam != 0 )
    {
        host.dkl = host.patternFxParam;
    }

    if( !host.isEnabled() )
    {
        return;
    }

    initPorta(host);
    initCommandDKL(host);
    host.flags |= HCFLG_UPD_ALWAYS;
}

void ItModule::initCommandM(HostChannel& host)
{
    initNoCommand(host);

    host.channelState.fx = 'M';
    host.channelState.fxDesc = ppp::fxdesc::ChannelVolume;

    if( host.patternFxParam > 64 )
    {
        return;
    }

    host.channelVolume = host.patternFxParam;
    if( !host.isEnabled() )
    {
        return;
    }

    host.getSlave()->channelVolume = host.channelVolume;
    host.getSlave()->flags |= SCFLG_RECALC_VOL;
}

void ItModule::initCommandN(HostChannel& host)
{
    if( host.patternFxParam != 0 )
    {
        host.n00 = host.patternFxParam;
    }

    initNoCommand(host);

    host.channelState.fx = 'N';

    if( (host.n00 & 0x0fu) == 0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::ChannelVolSlideUp;
        host.channelVolumeChange = host.n00 >> 4u;
        host.flags |= HCFLG_UPD_ALWAYS;
        return;
    }

    if( (host.n00 & 0xf0u) == 0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::ChannelVolSlideDown;
        host.channelVolumeChange = -int(host.n00);
        host.flags |= HCFLG_UPD_ALWAYS;
        return;
    }

    auto loNybble = host.n00 & 0x0fu;
    auto hiNybble = (host.n00 & 0xf0u) >> 4u;

    int volume;
    if( loNybble == 0x0f )
    {
        host.channelState.fxDesc = ppp::fxdesc::ChannelVolSlideUp;
        volume = host.channelVolume + hiNybble;
        if( volume > 64 )
        {
            volume = 64;
        }
    }
    else if( hiNybble == 0x0f )
    {
        host.channelState.fxDesc = ppp::fxdesc::ChannelVolSlideDown;
        volume = host.channelVolume - loNybble;
        if( volume < 0 )
        {
            volume = 0;
        }
    }

    host.channelVolume = volume;
    if( !host.isEnabled() )
    {
        return;
    }

    host.getSlave()->channelVolume = host.channelVolume;
    host.getSlave()->flags |= SCFLG_RECALC_VOL;
}

void ItModule::initCommandO(HostChannel& host)
{
    if( host.patternFxParam != 0 )
    {
        host.o00 = host.patternFxParam;
    }

    initNoCommand(host);

    host.channelState.fx = 'O';
    host.channelState.fxDesc = ppp::fxdesc::Offset;

    if( (host.cellMask & (HCFLG_MSK_NOTE | HCFLG_MSK_INS)) == 0 )
    {
        return;
    }

    if( host.effectiveNote > PATTERN_MAX_NOTE )
    {
        return;
    }

    if( !host.isEnabled() )
    {
        return;
    }

    auto slave = host.getSlave();
    auto offset = (uint32_t(host.o00) << 8u) | (uint32_t(host.oxh) << 16u);

    if( offset >= slave->loopEnd )
    {
        if( (m_header.flags & ITHeader::FlgOldEffects) == 0 )
        {
            return;
        }

        offset = slave->loopEnd - 1;
    }

    slave->sampleOffset = offset;
    slave->loopDirBackward = false;
    BOOST_ASSERT(slave->sampleOffset >= 0 && slave->sampleOffset < m_samples[slave->smp]->length()
                 && slave->sampleOffset < slave->loopEnd);
}

void ItModule::initCommandP(HostChannel& host)
{
    if( host.patternFxParam != 0 )
    {
        host.p00 = host.patternFxParam;
    }

    initNoCommand(host);

    host.channelState.fx = 'P';
    host.channelState.fxDesc = ppp::fxdesc::SetPanPos;

    auto pan = host.cp;
    if( host.isEnabled() )
    {
        pan = host.getSlave()->ps;
    }

    if( pan == SurroundPan )
    {
        return;
    }

    if( (host.p00 & 0x0fu) == 0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::PanSlideLeft;
        host.panSlideChange = -int(host.p00 >> 4u);
        host.flags |= HCFLG_UPD_ALWAYS;
        return;
    }
    if( (host.p00 & 0xf0u) == 0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::PanSlideRight;
        host.panSlideChange = host.p00;
        host.flags |= HCFLG_UPD_ALWAYS;
        return;
    }
    auto loNybble = host.p00 & 0x0fu;
    auto hiNybble = host.p00 & 0xf0u;
    if( loNybble == 0x0f )
    {
        host.channelState.fxDesc = ppp::fxdesc::PanSlideLeft;
        hiNybble >>= 4;
        auto p = pan - int(hiNybble);
        if( p < 0 )
        {
            p = 0;
        }
        setPan(host, p);
    }
    else if( hiNybble == 0xf0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::PanSlideRight;
        auto p = pan + loNybble;
        if( p > 64 )
        {
            p = 64;
        }
        setPan(host, p);
    }
}

void ItModule::initCommandQ(HostChannel& host)
{
    initNoCommand(host);

    host.channelState.fx = 'Q';
    host.channelState.fxDesc = ppp::fxdesc::Retrigger;

    if( host.patternFxParam != 0 )
    {
        host.q00 = host.patternFxParam;
    }

    if( !host.isEnabled() )
    {
        return;
    }

    host.flags |= HCFLG_UPD_IF_ON;

    if( (host.cellMask & HCFLG_MSK_NOTE) == 0 )
    {
        commandQ(host);
        return;
    }

    host.retriggerCountdown = host.q00 & 0x0fu;
}

void ItModule::initCommandR(HostChannel& host)
{
    if( host.patternFxParam != 0 )
    {
        const auto speed = (host.patternFxParam & 0xf0u) >> 2u;
        if( speed != 0 )
        {
            host.tremoloSpeed = speed;
        }

        const auto depth = (host.patternFxParam & 0x0fu) << 1u;
        if( depth != 0 )
        {
            host.tremoloDepth = depth;
        }
    }

    initNoCommand(host);

    host.channelState.fx = 'R';
    host.channelState.fxDesc = ppp::fxdesc::Tremolo;

    if( !host.isEnabled() )
    {
        return;
    }

    host.flags |= HCFLG_UPD_IF_ON;

    if( (m_header.flags & ITHeader::FlgOldEffects) != 0 )
    {
        host.getSlave()->flags |= SCFLG_RECALC_FINAL_VOL;
        initTremolo(host, host.ltr);
    }
    else
    {
        commandR(host);
    }
}

void ItModule::initCommandS(HostChannel& host)
{
    if( host.patternFxParam != 0 )
    {
        host.s00 = host.patternFxParam;
    }

    auto fxType = static_cast<uint8_t>(host.s00 & 0xf0u);
    auto fxData = static_cast<uint8_t>(host.s00 & 0x0fu);

    host.sfxData = fxData;
    host.sfxType = fxType;

    switch( fxType >> 4u )
    {
        case 0x00:
        case 0x01:
        case 0x02:
            initNoCommand(host);
            return;
        case 0x03:
            if( fxData <= 3 )
            {
                host.vwf = fxData;
            }
            initNoCommand(host);
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::SetVibWaveform;
            return;
        case 0x04:
            if( fxData <= 3 )
            {
                host.tremoloWaveForm = fxData;
            }
            initNoCommand(host);
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::SetTremWaveform;
            return;
        case 0x05:
            if( fxData <= 3 )
            {
                host.panbrelloWaveform = fxData;
                host.panbrelloOffset = 0;
            }
            initNoCommand(host);
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::SetPanbrelloWaveform;
            return;
        case 0x06:
            m_tickCountdown = fxData;
            initNoCommand(host);
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::PatternDelay;
            return;
        case 0x07:
            switch( fxData )
            {
                case 0x00:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::NoteCut;
                    for( auto& slave : m_slaves )
                    {
                        if( host.getSlave() != nullptr && &host == host.getSlave()->getHost() && host.getSlave()->disowned )
                        {
                            slave.flags |= SCFLG_NOTE_CUT;
                        }
                    }
                    return;
                case 0x01:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::KeyOff;
                    for( auto& slave : m_slaves )
                    {
                        if( &host == slave.getHost() && slave.disowned )
                        {
                            slave.flags |= SCFLG_NOTE_OFF;
                            slave.applySampleLoop();
                        }
                    }
                    return;
                case 0x02:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::Fadeout;
                    for( auto& slave : m_slaves )
                    {
                        if( &host == slave.getHost() && slave.disowned )
                        {
                            slave.flags |= SCFLG_FADEOUT;
                            slave.applySampleLoop();
                        }
                    }
                    return;
                case 0x03:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::NNA;
                    if( host.isEnabled() )
                    {
                        host.getSlave()->nna = NNA_CUT;
                    }
                    return;
                case 0x04:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::NNA;
                    if( host.isEnabled() )
                    {
                        host.getSlave()->nna = NNA_CONTINUE;
                    }
                    return;
                case 0x05:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::NNA;
                    if( host.isEnabled() )
                    {
                        host.getSlave()->nna = NNA_NOTE_OFF;
                    }
                    return;
                case 0x06:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::NNA;
                    if( host.isEnabled() )
                    {
                        host.getSlave()->nna = NNA_NOTE_FADE;
                    }
                    return;
                case 0x07:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::EnvelopDisable;
                    if( host.isEnabled() )
                    {
                        host.getSlave()->flags &= ~SCFLG_VOL_ENV;
                    }
                    return;
                case 0x08:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::EnvelopEnable;
                    if( host.isEnabled() )
                    {
                        host.getSlave()->flags |= SCFLG_VOL_ENV;
                    }
                    return;
                case 0x09:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::EnvelopDisable;
                    if( host.isEnabled() )
                    {
                        host.getSlave()->flags &= ~SCFLG_PAN_ENV;
                    }
                    return;
                case 0x0a:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::EnvelopEnable;
                    if( host.isEnabled() )
                    {
                        host.getSlave()->flags |= SCFLG_PAN_ENV;
                    }
                    return;
                case 0x0b:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::EnvelopDisable;
                    if( host.isEnabled() )
                    {
                        host.getSlave()->flags &= ~SCFLG_PITCH_ENV;
                    }
                    return;
                case 0x0c:
                    initNoCommand(host);
                    host.channelState.fx = 'S';
                    host.channelState.fxDesc = ppp::fxdesc::EnvelopEnable;
                    if( host.isEnabled() )
                    {
                        host.getSlave()->flags |= SCFLG_PITCH_ENV;
                    }
                    return;
                case 0x0d:
                case 0x0e:
                case 0x0f:
                    initNoCommand(host);
                    return;
            }
        case 0x08:
            initNoCommand(host);
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::SetPanPos;
            setPan(host, static_cast<uint8_t>(((fxData * 16) + 2) / 4));
            break;

        case 0x09:
            if( fxData == 1 )
            {
                initNoCommand(host);
                host.channelState.fx = 'S';
                host.channelState.fxDesc = ppp::fxdesc::SetPanPos;
                setPan(host, SurroundPan);
                return;
            }
            initNoCommand(host);
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::SetPanPos;
            return;
        case 0x0a:
            host.oxh = fxData;
            initNoCommand(host);
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::Offset;
            return;
        case 0x0b:
            initNoCommand(host);
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::PatternLoop;
            if( fxData == 0 )
            {
                host.plr = state().row;
                return;
            }

            if( host.plc != 0 )
            {
                --host.plc;
                if( host.plc == 0 )
                {
                    host.plr = state().row + 1;
                    return;
                }
            }
            else
            {
                host.plc = fxData;
            }

            fxData = host.plr - 1;
            fxType = 0;

            m_nextRow = fxData;
            m_patternLooping = true;

            return;
        case 0x0c:
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::NoteCut;
            host.flags |= HCFLG_UPD_IF_ON;
            return;
        case 0x0d:
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::NoteDelay;
            host.flags |= HCFLG_UPD_ALWAYS;
            return;
        case 0x0e:
            if( !m_rowDelayActive )
            {
                m_rowDelay = fxData + 1u;
                m_rowDelayActive = true;
            }
            initNoCommand(host);
            host.channelState.fx = 'S';
            host.channelState.fxDesc = ppp::fxdesc::PatternDelay;
            return;
        case 0x0f:
            host.sfx = fxData;
            initNoCommand(host);
            return;
    }
}

void ItModule::initCommandT(HostChannel& host)
{
    if( host.patternFxParam != 0 )
    {
        host.t00 = host.patternFxParam;
    }

    if( host.t00 < 0x20 )
    {
        initNoCommand(host);
        host.flags |= HCFLG_UPD_ALWAYS;
        host.channelState.fx = 'T';
        host.channelState.fxDesc = ppp::fxdesc::SetTempo;
        return;
    }

    setTempo(host.t00);
    initNoCommand(host);
    host.channelState.fx = 'T';
    host.channelState.fxDesc = ppp::fxdesc::SetTempo;
}

void ItModule::initCommandU(HostChannel& host)
{
    if( (host.cellMask & HCFLG_MSK_NOTE) != 0 )
    {
        host.vibratoPosition = 0;
        host.lvi = 0;
    }

    auto hiNybble = host.patternFxParam & 0xf0u;
    auto loNybble = host.patternFxParam & 0x0fu;
    if( hiNybble != 0 )
    {
        hiNybble >>= 2;
        host.vsp = hiNybble;
    }
    if( loNybble != 0 )
    {
        if( (m_header.flags & ITHeader::FlgOldEffects) != 0 )
        {
            loNybble *= 2;
        }
        host.vdp = loNybble;
    }

    initNoCommand(host);
    host.channelState.fx = 'U';
    host.channelState.fxDesc = ppp::fxdesc::FineVibrato;
    if( !host.isEnabled() )
    {
        return;
    }

    host.flags |= HCFLG_UPD_IF_ON;
    initVibrato(host);
}

void ItModule::initCommandV(HostChannel& host)
{
    if( host.patternFxParam <= 0x80 )
    {
        state().globalVolume = host.patternFxParam;
        recalculateAllVolumes();
    }

    initNoCommand(host);

    host.channelState.fx = 'V';
    host.channelState.fxDesc = ppp::fxdesc::GlobalVolume;
}

void ItModule::initCommandW(HostChannel& host)
{
    initNoCommand(host);

    host.channelState.fx = 'W';
    host.channelState.fxDesc = ppp::fxdesc::GlobalVolume;

    if( host.patternFxParam != 0 )
    {
        host.w00 = host.patternFxParam;
    }

    if( host.w00 == 0 )
    {
        return;
    }

    auto loNybble = host.w00 & 0x0fu;
    auto hiNybble = host.w00 & 0xf0u;

    if( hiNybble == 0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::GlobalVolSlideDown;
        host.globalVolumeChange = -loNybble;
        host.flags |= HCFLG_UPD_ALWAYS;
    }
    else if( loNybble == 0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::GlobalVolSlideUp;
        host.globalVolumeChange = hiNybble >> 4;
        host.flags |= HCFLG_UPD_ALWAYS;
    }
    else if( hiNybble == 0xf0 )
    {
        host.channelState.fxDesc = ppp::fxdesc::GlobalVolSlideDown;
        if( state().globalVolume < loNybble )
        {
            state().globalVolume = 0;
        }
        else
        {
            state().globalVolume -= loNybble;
        }

        recalculateAllVolumes();
    }
    else if( loNybble == 0x0f )
    {
        host.channelState.fxDesc = ppp::fxdesc::GlobalVolSlideUp;
        hiNybble >>= 4;
        state().globalVolume += hiNybble;
        if( state().globalVolume > 0x80 )
        {
            state().globalVolume = 0x80;
        }

        recalculateAllVolumes();
    }
}

void ItModule::initCommandX(HostChannel& host)
{
    initNoCommand(host);
    host.channelState.fx = 'W';
    host.channelState.fxDesc = ppp::fxdesc::SetPanPos;
    setPan(host, (int(host.patternFxParam) + 2) / 4);
}

void ItModule::initCommandY(HostChannel& host)
{
    if( host.patternFxParam != 0 )
    {
        auto speed = (host.patternFxParam & 0xf0u) >> 4u;
        if( speed != 0 )
        {
            host.panbrelloSpeed = speed;
        }

        auto depth = (host.patternFxParam & 0x0fu) * 2u;
        if( depth != 0 )
        {
            host.panbrelloDepth = depth;
        }
    }

    initNoCommand(host);

    host.channelState.fx = 'Y';
    host.channelState.fxDesc = ppp::fxdesc::Panbrello;

    if( host.isEnabled() )
    {
        host.flags |= HCFLG_UPD_IF_ON;
        commandY(host);
    }
}

void ItModule::initCommandZ(HostChannel& host)
{
    initNoCommand(host);

    host.channelState.fx = 'Z';
    host.channelState.fxDesc = "Macro ";

    if( (host.patternFxParam & 0x80u) == 0 )
    {
        midiTranslateParametrized(&host, host.getSlave(), m_midiDataArea.m00[host.sfx & 0x0f]);
    }
    else
    {
        midiTranslateParametrized(&host, host.getSlave(), m_midiDataArea.m80[host.patternFxParam & 0x7f]);
    }
}

void ItModule::commandD(HostChannel& host)
{
    int8_t volume = host.vch + host.getSlave()->effectiveBaseVolume;
    if( volume < 0 )
    {
        host.flags &= ~HCFLG_UPD_IF_ON;
        volume = 0;
    }
    else if( volume > 64 )
    {
        host.flags &= ~HCFLG_UPD_IF_ON;
        volume = 64;
    }

    host.getSlave()->flags |= SCFLG_RECALC_VOL;
    host.getSlave()->effectiveVolume = volume;
    host.getSlave()->effectiveBaseVolume = volume;
    host.vse = volume;
}

void ItModule::commandE(HostChannel& host)
{
    pitchSlideDown(*host.getSlave(), host.slideSpeed);
    host.getSlave()->frequencySet = host.getSlave()->frequency;
}

void ItModule::commandF(HostChannel& host)
{
    pitchSlideUp(host, host.slideSpeed);
    host.getSlave()->frequencySet = host.getSlave()->frequency;
}

void ItModule::commandG(HostChannel& host)
{
    if( (host.flags & HCFLG_SLIDE) == 0 )
    {
        return;
    }

    BOOST_ASSERT(host.portaTargetFrequency != 0);

    if( host.portaTargetFrequency > host.getSlave()->frequencySet )
    {
        pitchSlideUp(host, host.slideSpeed);
        if( (host.getSlave()->flags & SCFLG_NOTE_CUT) == 0 )
        {
            if( host.getSlave()->frequency < host.portaTargetFrequency )
            {
                host.getSlave()->frequencySet = host.getSlave()->frequency;
                return;
            }
        }

        host.getSlave()->flags &= ~SCFLG_NOTE_CUT;
        host.enable();
    }
    else
    {
        pitchSlideDown(*host.getSlave(), host.slideSpeed);
        if( host.getSlave()->frequency > host.portaTargetFrequency )
        {
            host.getSlave()->frequencySet = host.getSlave()->frequency;
            return;
        }
    }

    host.flags &= ~(HCFLG_UPD_MODE | HCFLG_SLIDE);
    host.getSlave()->frequency = host.portaTargetFrequency;
    host.getSlave()->frequencySet = host.portaTargetFrequency;
}

void ItModule::commandH(HostChannel& host)
{
    host.getSlave()->flags |= SCFLG_FREQ_CHANGE;

    host.vibratoPosition += host.vsp;

    switch( host.vwf )
    {
        case 0:
            host.lvi = fineSineData[host.vibratoPosition];
            break;
        case 1:
            host.lvi = fineRampDownData[host.vibratoPosition];
            break;
        case 2:
            host.lvi = fineSquareWave[host.vibratoPosition];
            break;
        case 3:
        default:
            host.lvi = (std::rand() & 0x7f) - 64;
    }

    setVibrato(host);
}

void ItModule::commandI(HostChannel& host)
{
    host.getSlave()->flags |= SCFLG_RECALC_VOL;
    --host.tremorCountdown;
    if( host.tremorCountdown <= 0 )
    {
        host.tremorOn = !host.tremorOn;
        host.tremorCountdown = (host.tremorOn ? host.tremorOnTime : host.tremorOffTime);
    }

    if( !host.tremorOn )
    {
        host.patternVolume = 0;
    }
}

void ItModule::commandJ(HostChannel& host)
{
    host.getSlave()->flags |= SCFLG_FREQ_CHANGE;

    ++host.arpeggioStage;
    if( host.arpeggioStage >= 3 )
    {
        host.arpeggioStage = 0;
    }

    uint64_t frq = host.getSlave()->frequency;
    switch( host.arpeggioStage )
    {
        case 0:
        default:
            break;
        case 1:
            frq *= host.arpeggioStage1;
            break;
        case 2:
            frq *= host.arpeggioStage2;
            break;
    }
    if( frq > std::numeric_limits<uint32_t>::max() )
    {
        host.getSlave()->frequency = 0;
    }

    host.getSlave()->frequency = static_cast<uint32_t>(frq);
}

void ItModule::commandK(HostChannel& host)
{
    commandH(host);
    commandD(host);
}

void ItModule::commandL(HostChannel& host)
{
    if( (host.flags & HCFLG_SLIDE) != 0 )
    {
        commandG(host);
        host.flags |= HCFLG_UPD_IF_ON;
    }

    commandD(host);
}

void ItModule::commandN(HostChannel& host)
{
    if( !host.isEnabled() )
    {
        return;
    }

    int8_t tmp = host.channelVolume + host.channelVolumeChange;
    if( tmp < 0 )
    {
        tmp = 0;
    }
    else if( tmp > 64 )
    {
        tmp = 64;
    }

    host.getSlave()->channelVolume = tmp;
    host.getSlave()->flags |= SCFLG_RECALC_VOL;
}

void ItModule::commandP(HostChannel& host)
{
    int pan;
    if( host.isEnabled() )
    {
        pan = host.getSlave()->ps;
    }
    else
    {
        pan = host.cp;
    }

    pan += host.panSlideChange;
    if( pan < 0 )
    {
        pan = 0;
    }
    else if( pan > 64 )
    {
        pan = 64;
    }

    setPan(host, static_cast<uint8_t>(pan));
}

void ItModule::commandQ(HostChannel& host)
{
    --host.retriggerCountdown;
    if( host.retriggerCountdown > 0 )
    {
        return;
    }

    host.retriggerCountdown = host.q00 & 0x0f;

    if( (m_header.flags & ITHeader::FlgInstrumentMode) == 0 )
    {
        auto tmp = host.getSlave() + 64;
        *tmp = *host.getSlave();
        tmp->flags |= SCFLG_NOTE_CUT;
        tmp->disowned = true;
    }
    else
    {
        for( auto& slave : m_slaves )
        {
            if( (slave.flags & SCFLG_ON) != 0 )
            {
                continue;
            }

            slave = *host.getSlave();
            host.getSlave()->flags |= SCFLG_NOTE_CUT;
            host.getSlave()->disowned = true;
            host.setSlave(&slave);
            break;
        }
    }

    host.getSlave()->sampleOffset = 0;
    host.getSlave()->loopDirBackward = false;

    host.getSlave()->flags |= SCFLG_LOOP_CHANGE | SCFLG_NEW_NOTE | SCFLG_RECALC_FINAL_VOL;

    int vol = host.getSlave()->effectiveBaseVolume;

    switch( (host.q00 & 0xf0u) >> 4u )
    {
        case 0x00:
            return;
        case 0x01:
            vol -= 1;
            break;
        case 0x02:
            vol -= 2;
            break;
        case 0x03:
            vol -= 4;
            break;
        case 0x04:
            vol -= 8;
            break;
        case 0x05:
            vol -= 16;
            break;
        case 0x06:
            vol = vol * 2 / 3;
            break;
        case 0x07:
            vol /= 2;
            break;
        case 0x08:
            return;
        case 0x09:
            vol += 1;
            break;
        case 0x0a:
            vol += 2;
            break;
        case 0x0b:
            vol += 4;
            break;
        case 0x0c:
            vol += 8;
            break;
        case 0x0d:
            vol += 16;
            break;
        case 0x0e:
            vol = vol * 3 / 2;
            break;
        case 0x0f:
            vol *= 2;
            break;
    }

    if( vol < 0 )
    {
        vol = 0;
    }
    else if( vol > 64 )
    {
        vol = 64;
    }

    host.getSlave()->effectiveVolume = static_cast<uint8_t>(vol);
    host.getSlave()->effectiveBaseVolume = static_cast<uint8_t>(vol);
    host.vse = static_cast<uint8_t>(vol);
    host.getSlave()->flags |= SCFLG_RECALC_VOL;
}

void ItModule::commandR(HostChannel& host)
{
    host.getSlave()->flags |= SCFLG_RECALC_VOL;
    host.tremoloPosition += host.tremoloSpeed;
    int8_t value;
    switch( host.tremoloWaveForm )
    {
        case 0:
            value = fineSineData[host.tremoloPosition];
            break;
        case 1:
            value = fineRampDownData[host.tremoloPosition];
            break;
        case 2:
            value = fineSquareWave[host.tremoloPosition];
            break;
        case 3:
        default:
            value = (std::rand() & 0x7f) - 64;
    }

    initTremolo(host, value);
}

void ItModule::commandS(HostChannel& host)
{
    if( host.sfxType == 0xd0 )
    {
        --host.sfxData;
        if( host.sfxData <= 0 )
        {
            host.flags &= ~HCFLG_UPD_MODE;
            initNoCommand(host);
            host.flags |= HCFLG_ROW_UPDATED;

            if( (m_header.chnPan[std::distance(&m_hosts[0], &host)] & 0x80u) != 0 && host.isEnabled() )
            {
                host.getSlave()->flags |= SCFLG_MUTED;
            }
        }
    }
    else if( host.sfxType == 0xc0 )
    {
        if( host.isEnabled() )
        {
            --host.sfxData;
            if( host.sfxData <= 0 )
            {

                host.disable();
                host.getSlave()->flags |= SCFLG_NOTE_CUT;
            }
        }
    }
}

void ItModule::commandT(HostChannel& host)
{
    int newTempo = state().tempo;
    if( (host.t00 & 0xf0u) == 0 )
    {
        newTempo -= host.t00;
        if( newTempo < 0x20 )
        {
            newTempo = 0x20;
        }
    }
    else
    {
        newTempo += host.t00;
        newTempo -= 0x10;
        if( newTempo > 0xff )
        {
            newTempo = 0xff;
        }
    }

    setTempo(static_cast<uint8_t>(newTempo));
}

void ItModule::commandW(HostChannel& host)
{
    int newVolume = state().globalVolume;
    newVolume += host.globalVolumeChange;
    if( newVolume < 0 )
    {
        newVolume = 0;
    }
    else if( newVolume > 128 )
    {
        newVolume = 128;
    }
    state().globalVolume = static_cast<uint8_t>(newVolume);
    recalculateAllVolumes();
}

void ItModule::commandY(HostChannel& host)
{
    if( !host.isEnabled() )
    {
        return;
    }

    auto slave = host.getSlave();

    int value;
    switch( host.panbrelloWaveform )
    {
        case 0:
            host.panbrelloOffset += host.panbrelloSpeed;
            value = fineSineData[host.panbrelloOffset];
            break;
        case 1:
            host.panbrelloOffset += host.panbrelloSpeed;
            value = fineRampDownData[host.panbrelloOffset];
            break;
        case 2:
            host.panbrelloOffset += host.panbrelloSpeed;
            value = fineSquareWave[host.panbrelloOffset];
            break;
        case 3:
        default:
            if( host.panbrelloOffset <= 1 )
            {
                host.panbrelloOffset = host.panbrelloSpeed;

                value = (std::rand() & 0x7f) - 64;
                host.lastRandomPanbrelloValue = value;
            }
            else
            {
                --host.panbrelloOffset;
                value = host.lastRandomPanbrelloValue;
            }
    }

    if( slave->ps == SurroundPan )
    {
        return;
    }

    BOOST_ASSERT(value >= -64 && value <= 64);
    auto pan = slave->ps + ((value * host.panbrelloDepth * 4) + 128) / 256;
    if( pan < 0 )
    {
        pan = 0;
    }
    else if( pan > 64 )
    {
        pan = 64;
    }

    slave->flags |= SCFLG_RECALC_PAN;
    slave->pan = pan;
    BOOST_ASSERT(slave->pan <= 64 || slave->pan == SurroundPan);
}

void ItModule::volumeCommandC(HostChannel& host)
{
    auto slave = host.getSlave();
    auto volume = slave->effectiveBaseVolume + host.volumeFxParam;
    if( volume > 64 )
    {
        host.flags &= ~HCFLG_UPD_VOL_IF_ON;
        volume = 64;
    }

    setVolume(host, volume);
}

void ItModule::volumeCommandD(HostChannel& host)
{
    auto slave = host.getSlave();
    auto volume = slave->effectiveBaseVolume - host.volumeFxParam;
    if( volume < 0 )
    {
        host.flags &= ~HCFLG_UPD_VOL_IF_ON;
        volume = 0;
    }

    setVolume(host, volume);
}

void ItModule::volumeCommandE(HostChannel& host)
{
    pitchSlideDown(*host.getSlave(), host.efg * 4u);
    host.getSlave()->frequencySet = host.getSlave()->frequency;
}

void ItModule::volumeCommandF(HostChannel& host)
{
    pitchSlideUp(host, host.efg * 4u);
    host.getSlave()->frequencySet = host.getSlave()->frequency;
}

void ItModule::volumeCommandG(HostChannel& host)
{
    if( (host.flags & HCFLG_SLIDE) == 0 )
    {
        return;
    }

    uint16_t value;
    if( (m_header.flags & ITHeader::FlgLinkedEffects) == 0 )
    {
        value = host.linkedPortaFxValue;
    }
    else
    {
        value = host.efg;
    }

    if( value == 0 )
    {
        return;
    }

    value <<= 2;
    if( host.portaTargetFrequency > host.getSlave()->frequencySet )
    {
        pitchSlideUp(host, value);

        if( host.getSlave()->flags & SCFLG_NOTE_CUT )
        {
            host.getSlave()->flags &= ~SCFLG_NOTE_CUT;
            host.enable();
            host.flags &= ~(HCFLG_UPD_VOL_IF_ON | HCFLG_SLIDE);
            host.getSlave()->frequency = host.portaTargetFrequency;
            host.getSlave()->frequencySet = host.portaTargetFrequency;
            return;
        }

        if( host.getSlave()->frequency < host.portaTargetFrequency )
        {
            host.getSlave()->frequencySet = host.getSlave()->frequency;
            return;
        }
    }
    else
    {
        pitchSlideDown(*host.getSlave(), value);

        if( host.getSlave()->frequency > host.portaTargetFrequency )
        {
            host.getSlave()->frequencySet = host.getSlave()->frequency;
            return;
        }

        host.flags &= ~(HCFLG_UPD_VOL_IF_ON | HCFLG_SLIDE);
        host.getSlave()->frequency = host.portaTargetFrequency;
        host.getSlave()->frequencySet = host.portaTargetFrequency;
    }
}

SlaveChannel* ItModule::allocateChannel(HostChannel& host)
{
    m_lastSlaveChannel = nullptr;

    if( (m_header.flags & ITHeader::FlgInstrumentMode) == 0 )
    {
        return allocateSampleChannel(host);
    }

    // Instrument handler!
    if( host.patternInstrument == 0xff )
    {
        return allocateSampleChannel(host);
    }

    if( host.patternInstrument == 0 )
    {
        return nullptr;
    }

    auto hostIntrument = &m_instruments[host.patternInstrument - 1];

    uint8_t refDca;
    const uint8_t SlaveChannel::* testeeMember;
    uint8_t refValue;
    uint8_t refNna;
    const HostChannel* refHostChan = nullptr;

    if( host.isEnabled() )
    {
        // New note action handling...
        if( host.getSlave()->insOffs == hostIntrument )
        {
            m_lastSlaveChannel = host.getSlave();
        }

        refNna = host.getSlave()->nna;
        if( host.getSlave()->nna != NNA_CUT )
        {
            host.getSlave()->disowned = true;

AllocateHandleNNA:
            if( host.getSlave()->effectiveBaseVolume != 0 && host.getSlave()->channelVolume != 0 && host.getSlave()->sampleVolume != 0 )
            {
                switch( refNna )
                {
                    case NNA_CUT:
                    case NNA_CONTINUE:
                        break;
                    case NNA_NOTE_OFF:
                        host.getSlave()->flags |= SCFLG_NOTE_OFF;
                        host.getSlave()->applySampleLoop();
                        break;
                    case NNA_NOTE_FADE:
                    default:
                        host.getSlave()->flags |= SCFLG_FADEOUT;
                        break;
                }
                goto AllocateChannel8_initSearch;
            }
        }

        host.getSlave()->flags |= SCFLG_NOTE_CUT;
        host.getSlave()->disowned = true;

        return allocateSampleSearch(host, *hostIntrument);
    }

AllocateChannel8_initSearch:
    if( host.getSlave() == nullptr || host.getSlave()->insOffs == nullptr || host.getSlave()->insOffs->dct == DCT_OFF )
    {
        return allocateSampleSearch(host, *hostIntrument);
    }

    // Duplicate check...
    switch( host.getSlave()->insOffs->dct )
    {
        case DCT_NOTE:
            refValue = host.patternNote;
            testeeMember = &SlaveChannel::effectiveNote;
            break;
        case DCT_INSTRUMENT:
            refValue = host.patternInstrument;
            testeeMember = &SlaveChannel::ins; // Duplicate instrument
            break;
        case DCT_SAMPLE:
        default:
            if( host.sampleIndex < 1 )
            {
                return allocateSampleSearch(host, *hostIntrument);
            }
            refValue = host.sampleIndex - 1;
            testeeMember = &SlaveChannel::smp; // Duplicate sample
            break;
    }

    refHostChan = &host;
    refDca = hostIntrument->dca;

    for( auto& slave : m_slaves )
    {
        if( (slave.flags & SCFLG_ON) == 0 )
        {
            continue;
        }

        BOOST_ASSERT(refHostChan != nullptr);
        if( refHostChan != slave.getHost() || !slave.disowned )
        {
            continue;
        }

        // OK. same channel... now..
        if( host.patternInstrument != slave.ins )
        {
            continue;
        }

        if( refValue != slave.*testeeMember )
        {
            continue;
        }

        if( refDca != slave.dca )
        {
            continue;
        }

        switch( refDca )
        {
            case DCA_CUT:
                slave.flags |= SCFLG_NOTE_CUT;
                slave.disowned = true;
                return allocateSampleSearch(host, *hostIntrument);
            case DCA_NOTE_OFF:
                slave.dca = DCA_CUT;
                refNna = NNA_NOTE_OFF;
                break;
            case DCA_NOTE_FADE:
                slave.dca = DCA_CUT;
                refNna = NNA_NOTE_FADE;
                break;
            default:
                BOOST_ASSERT(false);
        }
        goto AllocateHandleNNA;
    }

    return allocateSampleSearch(host, *hostIntrument);
}

SlaveChannel* ItModule::allocateSampleChannel(HostChannel& host)
{
    SlaveChannel* slave = host.getSlave();

    if( (slave->flags & SCFLG_ON) != 0 )
    {
        // copy out channel
        slave->flags |= SCFLG_NOTE_CUT;
        slave->disowned = true;

        slave[64] = slave[0];
    }

    slave->setHost(&host);
    slave->disowned = false;
    slave->flags = SCFLG_NEW_NOTE | SCFLG_FREQ_CHANGE | SCFLG_RECALC_VOL | SCFLG_RECALC_PAN | SCFLG_ON;

    slave->channelVolume = host.channelVolume;
    slave->pan = host.cp;
    BOOST_ASSERT(slave->pan <= 64 || slave->pan == SurroundPan);
    slave->ps = host.cp;

    // Get sample offset.
    // General stuff.
    slave->fadeOut = 0x400;
    slave->vEnvelope.value = 64;
    slave->filterCutoff = 0x7f;
    slave->filterResonance = 0;

    slave->effectiveNote = host.patternNote;
    slave->ins = host.patternInstrument;

    if( host.sampleIndex == 0 )
    {
        slave->flags = SCFLG_NOTE_CUT;
        host.disable();
        return nullptr;
    }

    slave->smp = host.sampleIndex - 1; // TODO

    slave->smpOffs = m_samples[slave->smp].get();

    slave->vip = 0;
    slave->viDepth = 0;
    slave->pEnvelope.value = 0;
    slave->ptEnvelope.value = 0;
    slave->loopDirBackward = false;

    if( slave->smpOffs->header.samplePointer == 0 || (slave->smpOffs->header.flg & ItSampleHeader::FlgWithHeader) == 0 )
    {
        slave->flags = SCFLG_NOTE_CUT;
        host.disable();
        return nullptr;
    }

    slave->sampleVolume = slave->smpOffs->header.gvl * 2u;

    return slave;
}

SlaveChannel* ItModule::allocateSampleSearch(HostChannel& host, const ItInstrument& instrument)
{
    for( auto& slave : m_slaves )
    {
        if( (slave.flags & SCFLG_ON) == 0 )
        {
            return allocateChannelInstrument(&slave, host, instrument);
        }
    }

    // Common sample search
    struct SampleUsage
    {
        uint8_t count = 0;
        SlaveChannel* channel = nullptr;
        uint16_t volume = 0;
    };
    std::vector<SampleUsage> sampleUsages(m_slaves.size());

    for( auto& slave : m_slaves )
    {
        if( slave.smp > 99 )
        {
            continue;
        }

        ++sampleUsages[slave.smp].count;
        if( !slave.disowned )
        {
            continue;
        }

        if( sampleUsages[slave.smp].volume <= slave._16bVol )
        {
            continue;
        }

        sampleUsages[slave.smp].channel = &slave;
        sampleUsages[slave.smp].volume = slave._16bVol;
    }

    // OK.. now search table for maximum occurrence of sample...

    SlaveChannel* result = nullptr;
    int maxCount = 2; // Find maximum count, has to be greater than 2 channels

    for( const auto& di : sampleUsages )
    {
        if( di.count <= maxCount )
        {
            continue;
        }

        maxCount = di.count;
        result = di.channel;
    }

    if( result == nullptr )
    {
        result = searchUsedChannel(host);
    }

    BOOST_ASSERT(result != nullptr);

    return allocateChannelInstrument(result, host, instrument);
}

SlaveChannel* ItModule::allocateChannelInstrument(SlaveChannel* slave, HostChannel& host, const ItInstrument& instrument)
{
    BOOST_ASSERT(slave != nullptr);
    host.setSlave(slave);
    slave->setHost(&host);
    slave->disowned = false;

    slave->vip = 0;
    slave->viDepth = 0;
    slave->loopDirBackward = false;

    slave->setInstrument(*this, instrument, m_lastSlaveChannel);

    slave->sampleVolume = instrument.gbv;

    slave->fadeOut = 0x400;
    slave->effectiveNote = host.patternNote;
    slave->ins = host.patternInstrument;

    if( host.sampleIndex == 0 )
    {
        slave->flags = SCFLG_NOTE_CUT;
        host.disable();
        return nullptr;
    }

    slave->smp = host.sampleIndex - 1;
    slave->smpOffs = m_samples[slave->smp].get();

    if( slave->smpOffs->header.length == 0 || (slave->smpOffs->header.flg & ItSampleHeader::FlgWithHeader) == 0 )
    {
        slave->flags = SCFLG_NOTE_CUT;
        host.disable();
        return nullptr;
    }

    slave->sampleVolume = slave->smpOffs->header.gvl * slave->sampleVolume / 64u;
    BOOST_ASSERT(slave->sampleVolume >= 0 && slave->sampleVolume <= 128);

    return slave;
}

SlaveChannel* ItModule::searchUsedChannel(HostChannel& host)
{
    // Find out which host channel has the most
    // (disowned) slave channels
    // Then find the softest non-single sample
    // in that channel.

    struct Info
    {
        int count = 0;
    };
    std::map<const HostChannel*, Info> infos;
    for( const auto& slave : m_slaves )
    {
        ++infos[slave.getHost()].count;
    }

Retry:

    // OK.. search through and find the most heavily used channel

    int maxUse = 1;
    const HostChannel* maxUsedChannel = nullptr;

    for( const auto& info : infos )
    {
        if( info.second.count <= maxUse )
        {
            continue;
        }

        maxUse = info.second.count;
        maxUsedChannel = info.first;
    }

    if( maxUse <= 1 )
    {
        //  Now search for softest disowned sample (not non-single)

        SlaveChannel* minVolumeChannel = nullptr;
        uint16_t minVolume = 0xffff;

        for( auto& slave : m_slaves )
        {
            if( !slave.disowned || slave._16bVol > minVolume )
            {
                continue;
            }

            minVolumeChannel = &slave;
            minVolume = slave._16bVol;
        }

        if( minVolumeChannel == nullptr )
        {
            host.disable();
        }

        return minVolumeChannel;
    }
    else
    {
        // Search for disowned only

        auto refSample = host.sampleIndex - 1;
        SlaveChannel* result = nullptr;
        uint16_t minVolume = 0xffff;

        for( auto& outer : m_slaves )
        {
            if( maxUsedChannel != outer.getHost() || !outer.disowned || minVolume <= outer._16bVol )
            {
                continue;
            }

            // Now check if any other channel contains this sample
            if( refSample != outer.smp )
            {
                const auto old = std::exchange(outer.smp, 0xff);

                bool found = false;
                for( auto& inner : m_slaves )
                {
                    if( refSample == inner.smp || old == inner.smp )
                    {
                        found = true;
                        break;
                    }
                }

                outer.smp = old;

                if( !found )
                {
                    continue;
                }

                outer.smp = old;
            }

            // OK found a second sample.
            result = &outer;
            minVolume = outer._16bVol;
        }

        if( result == nullptr )
        {
            infos[maxUsedChannel].count = 0;
            goto Retry; // Next cycle...
        }

        minVolume = 0xffff;

        for( auto& slave : m_slaves )
        {
            if( result->smp != slave.smp || !slave.disowned || minVolume <= slave._16bVol )
            {
                result = &slave;
                minVolume = slave._16bVol;
            }
        }

        return result;
    }
}

void ItModule::M32MixHandler(MixerFrameBuffer& mixBuffer, bool preprocess)
{
    static constexpr int MixVolume = 0x80; //!< 0..128

    // Check each channel...
    // Prepare mixing stuff

    // Work backwards
    for( auto& slave : m_slaves )
    {
        if( (slave.flags & SCFLG_ON) == 0 )
        {
            continue;
        }

        auto slaveFlags = slave.flags;
        if( (slaveFlags & SCFLG_NOTE_CUT) != 0 )
        {
            // Note cut.
            slave.flags &= ~SCFLG_ON;
            slave._16bVol = 0;
            slaveFlags |= SCFLG_RECALC_FINAL_VOL;
        }

        // M32MixHandler13
        if( (slaveFlags & SCFLG_FREQ_CHANGE) != 0 )
        {
            if( slave.frequency / 65536 >= frequency() )
            {
                slave.flags = SCFLG_NOTE_CUT;

                if( !slave.disowned )
                {
                    slave.getHost()->disable();
                }
                continue;
            }

            slave.sampleOffset.setStepSize(frequency(), slave.frequency);
        }

        // M32MixHandler3
        if( (slaveFlags & SCFLG_NEW_NOTE) != 0 )
        {
            slave.mixVolumeL = 0;
            slave.mixVolumeR = 0;
        }

        // M32MixHandler12
        if( (slaveFlags & (SCFLG_PAN_CHANGE | SCFLG_LOOP_CHANGE | SCFLG_RECALC_FINAL_VOL)) != 0 )
        {
            // M32MixHandlerNoFilter

            if( (slaveFlags & SCFLG_MUTED) != 0 )
            {
                // Zero volume
                slave.mixVolumeL = 0;
                slave.mixVolumeR = 0;
            }
            else if( slave.finalPan == SurroundPan )
            {
                // M32MixHandler6
                auto volume = static_cast<uint16_t>(slave._16bVol * MixVolume / 2 / 256);
                BOOST_ASSERT(volume <= 8192);
                slave.mixVolumeL = volume;
                slave.mixVolumeR = volume;
            }
            else
            {
                BOOST_ASSERT(slave.finalPan <= 64);
                slave.mixVolumeL = uint32_t(slave.finalPan) * MixVolume * slave._16bVol / (256 * 64);
                slave.mixVolumeR = uint32_t(64 - slave.finalPan) * MixVolume * slave._16bVol / (256 * 64);
            }
        }

        {
            auto tmpBuf = ppp::read(
                *slave.smpOffs,
                slave.lpm,
                interpolation(),
                slave.sampleOffset,
                mixBuffer.size(),
                slave.loopDirBackward,
                slave.loopBeg,
                slave.loopEnd,
                preprocess);

            BOOST_ASSERT(tmpBuf.size() <= mixBuffer.size());

            if( !preprocess )
            {
                slave.filterL.update(frequency(), (slave.filterCutoff & 0x7fu) * slave.envFilterCutoff, slave.filterResonance);
                slave.filterR.update(frequency(), (slave.filterCutoff & 0x7fu) * slave.envFilterCutoff, slave.filterResonance);

                for( size_t i = 0; i < tmpBuf.size(); ++i )
                {
                    const auto l = slave.filterL.filter(static_cast<MixerSample>(tmpBuf[i].left) * slave.mixVolumeL);
                    const auto r = slave.filterR.filter(static_cast<MixerSample>(tmpBuf[i].right) * slave.mixVolumeR);

                    mixBuffer[i].left += l;
                    mixBuffer[i].right += r;
                }
            }

            if( mixBuffer.size() != tmpBuf.size() )
            {
                slave.flags = SCFLG_NOTE_CUT;
                if( !slave.disowned )
                {
                    slave.getHost()->disable();
                }
            }
        }

        slave.flags &=
            SCFLG_ON | SCFLG_NOTE_OFF | SCFLG_FADEOUT | SCFLG_CTR_PAN | SCFLG_MUTED | SCFLG_VOL_ENV |
            SCFLG_PAN_ENV |
            SCFLG_PITCH_ENV;
    }
}

ChannelState ItModule::internal_channelStatus(size_t idx) const
{
    if( idx >= m_hosts.size() )
    {
        BOOST_THROW_EXCEPTION(std::out_of_range("Requested channel index out of range"));
    }

    return m_hosts[idx].channelState;
}

int ItModule::internal_channelCount() const
{
    return m_hosts.size();
}

light4cxx::Logger* ItModule::logger()
{
    return light4cxx::Logger::get(AbstractModule::logger()->name() + ".it");
}
}
}
