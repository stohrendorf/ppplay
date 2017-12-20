#include "modmodule.h"
#include "modsample.h"
#include "modchannel.h"
#include "modpattern.h"
#include "modcell.h"

#include "stream/abstractarchive.h"
#include "stream/stream.h"
#include <genmod/channelstate.h>
#include <genmod/orderentry.h>

#include <boost/exception/all.hpp>

#include <array>

namespace ppp
{
namespace mod
{
namespace
{
struct LoadingMode
{
    enum
    {
        Smp31, //!< @brief Take orders after song length into account for pattern count determination
        Smp31Malformed, //!< @brief Do NOT take orders after song length into account for pattern count determination (see DRAGNET.MOD)
        Smp15,
        Count
    };
};
}

AbstractModule* ModModule::factory(Stream* stream, uint32_t frequency, int maxRpt, Sample::Interpolation inter)
{
    auto result = new ModModule(maxRpt, inter);
    for( int i = 0; i < LoadingMode::Count; i++ )
    {
        stream->seek(0);
        stream->clear();
        if( !result->load(stream, i) )
        {
            delete result;
            if( i == LoadingMode::Count - 1 )
            {
                return nullptr;
            }
            result = new ModModule(maxRpt, inter);
        }
        else
        {
            break;
        }
    }
    if( !result->initialize(frequency) )
    {
        delete result;
        return nullptr;
    }
    return result;
}

ModModule::ModModule(int maxRpt, Sample::Interpolation inter)
    : AbstractModule(maxRpt, inter), m_samples(), m_patterns(), m_channels(), m_patLoopRow(-1), m_patLoopCount(-1), m_breakRow(-1), m_patDelayCount(-1)
      , m_breakOrder(~0)
{
}

ModModule::~ModModule()
{
    deleteAll(m_samples);
    deleteAll(m_patterns);
    deleteAll(m_channels);
}

ModSample* ModModule::sampleAt(size_t idx) const
{
    if( idx == 0 )
    {
        return nullptr;
    }
    idx--;
    if( idx >= m_samples.size() )
    {
        return nullptr;
    }
    return m_samples[idx];
}

namespace
{
/**
 * @brief Maps module IDs to their respective channel counts
 */
struct IdMetaInfo
{
    const std::string id;
    const uint8_t channels;
    const std::string tracker;
};

const std::array<const IdMetaInfo, 31> idMetaData = {{
                                                         {"M.K.", 4, "ProTracker"},
                                                         {"M!K!", 4, "ProTracker"},
                                                         {"FLT4", 4, "Startrekker"},
                                                         {"FLT8", 8, "Startrekker"},
                                                         {"CD81", 8, "Falcon"}, //< @todo Check tracker name
                                                         {"TDZ1", 1, "TakeTracker"},
                                                         {"TDZ2", 2, "TakeTracker"},
                                                         {"TDZ3", 3, "TakeTracker"},
                                                         {"5CHN", 5, "TakeTracker"},
                                                         {"7CHN", 7, "TakeTracker"},
                                                         {"9CHN", 9, "TakeTracker"},
                                                         {"11CH", 11, "TakeTracker"},
                                                         {"13CH", 13, "TakeTracker"},
                                                         {"15CH", 15, "TakeTracker"},
                                                         {"2CHN", 2, "FastTracker"},
                                                         {"4CHN", 4, "FastTracker"},
                                                         {"6CHN", 6, "FastTracker"},
                                                         {"8CHN", 8, "FastTracker"},
                                                         {"10CH", 10, "FastTracker"},
                                                         {"12CH", 12, "FastTracker"},
                                                         {"14CH", 14, "FastTracker"},
                                                         {"16CH", 16, "FastTracker"},
                                                         {"18CH", 18, "FastTracker"},
                                                         {"20CH", 20, "FastTracker"},
                                                         {"22CH", 22, "FastTracker"},
                                                         {"24CH", 24, "FastTracker"},
                                                         {"26CH", 26, "FastTracker"},
                                                         {"28CH", 28, "FastTracker"},
                                                         {"30CH", 30, "FastTracker"},
                                                         {"32CH", 32, "FastTracker"},
                                                         {"OCTA", 8, "Octalyzer"}, //< @todo Check tracker name
                                                     }
};

const IdMetaInfo* findMeta(Stream* stream)
{
    char id[5];
    stream->read(id, 4);
    id[4] = '\0';
    for( const IdMetaInfo& mi : idMetaData )
    {
        if( id == mi.id )
        {
            return &mi;
        }
    }
    // revert...
    stream->seekrel(-4);
    return nullptr;
}

const IdMetaInfo smp15MetaInfo = {
    std::string(),
    4,
    "Amiga ProTracker"
};
} // anonymous namespace

bool ModModule::load(Stream* stream, int loadMode)
{
    noConstMetaInfo().filename = stream->name();
    setTempo(125);
    setSpeed(6);
    state().globalVolume = 0x40;
    char modName[20];
    stream->read(modName, 20);
    noConstMetaInfo().title = stringncpy(modName, 20);
    const IdMetaInfo* meta = nullptr;
    if( loadMode != LoadingMode::Smp15 )
    {
        // check 31-sample mod
        logger()->info(L4CXX_LOCATION, "Probing meta-info for 31-sample mod...");
        stream->seek(0x438);
        meta = findMeta(stream);
        if( meta == nullptr )
        {
            logger()->warn(L4CXX_LOCATION, "Could not find a valid module ID");
            return false;
        }
    }
    else
    {
        logger()->info(L4CXX_LOCATION, "Trying to load 15-sample mod...");
        meta = &smp15MetaInfo;
    }
    logger()->debug(L4CXX_LOCATION, "%d-channel, ID '%s', Tracker '%s'", int(meta->channels), meta->id, meta->tracker);
    noConstMetaInfo().trackerInfo = meta->tracker;
    for( int i = 0; i < meta->channels; i++ )
    {
        m_channels.emplace_back(new ModChannel(this, ((i + 1) & 2) == 0));
    }
    stream->seek(20);
    const int numSamples = loadMode == LoadingMode::Smp15 ? 15 : 31;
    for( int i = 0; i < numSamples; i++ )
    {
        auto smp = new ModSample();
        m_samples.push_back(smp);
        if( !smp->loadHeader(stream) )
        {
            logger()->warn(L4CXX_LOCATION, "Sample header could not be loaded");
            return false;
        }
    }
    uint8_t maxPatNum = 0;
    {
        // load orders
        uint8_t songLen;
        *stream >> songLen;
        if( songLen > 128 )
        {
            songLen = 128;
        }
        logger()->debug(L4CXX_LOCATION, "Song length: %d", int(songLen));
        uint8_t tmp;
        *stream >> tmp; // skip the restart pos
        for( uint_fast8_t i = 0; i < songLen; i++ )
        {
            *stream >> tmp;
            if( tmp >= 64 )
            {
                continue;
            }
            if( tmp > maxPatNum )
            {
                maxPatNum = tmp;
            }
            logger()->trace(L4CXX_LOCATION, "Order %d index: %d", int(i), int(tmp));
            addOrder(std::make_unique<OrderEntry>(tmp));
        }
        if( loadMode != LoadingMode::Smp31Malformed )
        {
            while( songLen++ < 128 )
            {
                *stream >> tmp;
                if( tmp >= 64 )
                {
                    continue;
                }
                if( tmp > maxPatNum )
                {
                    maxPatNum = tmp;
                }
            }
        }
        else
        {
            stream->seekrel(128 - songLen);
        }
    }
    if( loadMode != LoadingMode::Smp15 )
    {
        stream->seekrel(4); // skip the ID
    }
    logger()->debug(L4CXX_LOCATION, "%d patterns @ %#x", int(maxPatNum), stream->pos());
    for( uint_fast8_t i = 0; i <= maxPatNum; i++ )
    {
        auto* pat = new ModPattern();
        m_patterns.push_back(pat);
        logger()->debug(L4CXX_LOCATION, "Loading pattern %u", int(i));
        if( !pat->load(stream, meta->channels) )
        {
            logger()->warn(L4CXX_LOCATION, "Could not load pattern");
            return false;
        }
    }
    logger()->debug(L4CXX_LOCATION, "Sample start @ %#x", stream->pos());
    for( auto& smp : m_samples )
    {
        if( !smp->loadData(stream) )
        {
            logger()->warn(L4CXX_LOCATION, "Could not load sample data");
            return false;
        }
    }
    logger()->debug(L4CXX_LOCATION, "pos=%#x size=%#x delta=%#x", stream->pos(), stream->size(), stream->size() - stream->pos());
    return stream->good() && stream->size() - stream->pos() < 0x100;
}

size_t ModModule::internal_buildTick(AudioFrameBuffer* buf)
{
    if( state().tick == 0 && state().order >= orderCount() )
    {
        if( buf )
        {
            buf->reset();
        }
        return 0;
    }
    try
    {
        if( buf && !buf->get() )
        {
            buf->reset(new AudioFrameBuffer::element_type);
        }
        if( state().tick == 0 )
        {
            checkGlobalFx();
        }
        if( orderAt(state().order)->playbackCount() >= maxRepeat() )
        {
            //logger()->info( L4CXX_LOCATION, "Song end reached: Maximum repeat count reached" );
            if( buf )
            {
                buf->reset();
            }
            return 0;
        }
        // update channels...
        state().pattern = orderAt(state().order)->index();
        ModPattern* currPat = getPattern(state().pattern);
        if( !currPat )
        {
            return 0;
        }
        if( buf )
        {
            MixerFrameBuffer mixerBuffer(new MixerFrameBuffer::element_type(tickBufferLength()));
            for( int currTrack = 0; currTrack < channelCount(); currTrack++ )
            {
                ModChannel* chan = m_channels[currTrack];
                const ModCell& cell = currPat->at(currTrack, state().row);
                chan->update(cell, false); // m_patDelayCount != -1);
                chan->mixTick(&mixerBuffer);
            }
            buf->get()->resize(mixerBuffer->size());
            MixerSampleFrame* mixerBufferPtr = &mixerBuffer->front();
            BasicSampleFrame* bufPtr = &buf->get()->front();
            for( size_t i = 0; i < mixerBuffer->size(); i++ )
            {  // postprocess...
                *bufPtr = mixerBufferPtr->rightShiftClip(2);
                bufPtr++;
                mixerBufferPtr++;
            }
        }
        else
        {
            for( int currTrack = 0; currTrack < channelCount(); currTrack++ )
            {
                ModChannel* chan = m_channels[currTrack];
                const ModCell& cell = currPat->at(currTrack, state().row);
                chan->update(cell, false); // m_patDelayCount != -1);
                chan->mixTick(nullptr);
            }
        }
        nextTick();
        if( !adjustPosition() )
        {
            logger()->info(L4CXX_LOCATION, "Song end reached: adjustPosition() failed");
            if( buf )
            {
                buf->reset();
            }
            return 0;
        }
        state().playedFrames += tickBufferLength();
        return tickBufferLength();
    }
    catch( ... )
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(boost::current_exception_diagnostic_information()));
    }
}

bool ModModule::adjustPosition()
{
    bool orderChanged = false;
    bool rowChanged = false;
    size_t newOrder = state().order;
    if( m_patDelayCount != -1 )
    {
        m_patDelayCount--;
    }
    if( state().tick == 0 && m_patDelayCount == -1 )
    {
        if( m_breakOrder != 0xffff )
        {
            logger()->debug(L4CXX_LOCATION, "Order break");
            if( m_breakOrder < orderCount() )
            {
                if( m_breakRow == -1 )
                {
                    orderAt(m_breakOrder)->increasePlaybackCount();
                }
                orderChanged = true;
                newOrder = m_breakOrder;
            }
            setRow(0);
            rowChanged = true;
        }
        if( m_breakRow != -1 )
        {
            if( m_breakRow <= 63 )
            {
                setRow(m_breakRow);
                rowChanged = true;
            }
            if( m_breakOrder == 0xffff )
            {
                if( m_patLoopCount == -1 )
                {
                    logger()->debug(L4CXX_LOCATION, "Row break");
                    ++newOrder;
                    orderChanged = true;
                }
            }
        }
        if( m_breakRow == -1 && m_breakOrder == 0xffff && m_patDelayCount == -1 )
        {
            setRow((state().row + 1) & 0x3f);
            rowChanged = true;
            if( state().row == 0 )
            {
                newOrder = state().order + 1;
                orderChanged = true;
            }
        }
        m_breakRow = -1;
        m_breakOrder = ~0;
    }
    if( newOrder >= orderCount() )
    {
        logger()->debug(L4CXX_LOCATION, "state().order>=orderCount()");
        setOrder(newOrder);
        return false;
    }
    if( orderChanged )
    {
        m_patLoopRow = 0;
        m_patLoopCount = -1;
        state().pattern = orderAt(newOrder)->index();
        setOrder(newOrder);
    }
    if( rowChanged )
    {
        if( !orderAt(newOrder)->increaseRowPlayback(state().row) )
        {
            logger()->info(L4CXX_LOCATION, "Row playback counter reached limit");
            setOrder(orderCount());
            return false;
        }
    }
    return true;
}

ChannelState ModModule::internal_channelStatus(size_t idx) const
{
    if( idx >= m_channels.size() )
    {
        BOOST_THROW_EXCEPTION(std::out_of_range("Requested channel index out of range"));
    }
    return m_channels[idx]->status();
}

AbstractArchive& ModModule::serialize(AbstractArchive* data)
{
    AbstractModule::serialize(data)
    % m_breakRow
    % m_breakOrder
    % m_patLoopRow
    % m_patLoopCount
    % m_patDelayCount;
    for( auto& chan : m_channels )
    {
        if( !chan )
        {
            continue;
        }
        data->archive(chan);
    }
    return *data;
}

int ModModule::internal_channelCount() const
{
    return m_channels.size();
}

bool ModModule::existsSample(size_t idx) const
{
    if( idx == 0 || idx > 30 )
    {
        return false;
    }
    return m_samples[idx - 1]->length() > 0;
}

void ModModule::checkGlobalFx()
{
    try
    {
        state().pattern = orderAt(state().order)->index();
        const ModPattern* currPat = getPattern(state().pattern);
        if( !currPat )
        {
            return;
        }
        // check for pattern loops
        int patLoopCounter = 0;
        for( int currTrack = 0; currTrack < channelCount(); currTrack++ )
        {
            const ModCell& cell = currPat->at(currTrack, state().row);
            if( cell.effect() == 0x0f )
            { continue; }
            uint8_t fx = cell.effect();
            uint8_t fxVal = cell.effectValue();
            if( fx != 0x0e )
            { continue; }
            if( (fxVal >> 4) != 0x06 )
            { continue; }
            if( (fxVal & 0x0f) == 0x00 )
            { // loop start
                m_patLoopRow = state().row;
            }
            else
            { // loop return
                patLoopCounter++;
                if( m_patLoopCount == -1 )
                {  // first loop return -> set loop count
                    m_patLoopCount = fxVal & 0x0f;
                    m_breakRow = m_patLoopRow;
                }
                else if( m_patLoopCount > 1 )
                {  // non-initial return -> decrease loop counter
                    m_patLoopCount--;
                    m_breakRow = m_patLoopRow;
                }
                else
                { // loops done...
                    if( patLoopCounter == 1 )
                    {  // one loop, all ok
                        m_patLoopCount = -1;
                        m_breakRow = -1;
                        m_patLoopRow = state().row + 1;
                    }
                    else
                    { // we got an "infinite" loop...
                        m_patLoopCount = 127;
                        m_breakRow = m_patLoopRow;
                        logger()->warn(L4CXX_LOCATION, "Infinite pattern loop detected");
                    }
                }
            }
        }
        // check for pattern delays
        for( int currTrack = 0; currTrack < channelCount(); currTrack++ )
        {
            const ModCell& cell = currPat->at(currTrack, state().row);
            if( cell.effect() == 0x0f )
            { continue; }
            uint8_t fx = cell.effect();
            uint8_t fxVal = cell.effectValue();
            if( fx != 0x0e )
            { continue; }
            if( (fxVal >> 4) != 0x0e )
            { continue; }
            if( (fxVal & 0x0f) == 0 )
            { continue; }
            if( m_patDelayCount != -1 )
            { continue; }
            m_patDelayCount = fxVal & 0x0f;
        }
        if( m_patDelayCount > 1 )
        {
            m_patDelayCount--;
        }
        else
        {
            m_patDelayCount = -1;
        }
        // now check for breaking effects
        for( int currTrack = 0; currTrack < channelCount(); currTrack++ )
        {
            if( m_patLoopCount != -1 )
            { break; }
            const ModCell& cell = currPat->at(currTrack, state().row);
            if( cell.effect() == 0x0f )
            { continue; }
            uint8_t fx = cell.effect();
            uint8_t fxVal = cell.effectValue();
            if( fx == 0x0b )
            {
                m_breakOrder = fxVal;
            }
            else if( fx == 0x0d )
            {
                m_breakRow = (fxVal >> 4) * 10 + (fxVal & 0x0f);
                logger()->info(L4CXX_LOCATION, "Row %1%: Break pattern to row %2%", state().row, int(m_breakRow));
            }
        }
    }
    catch( boost::exception& )
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(boost::current_exception_diagnostic_information()));
    }
    catch( ... )
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Unknown exception"));
    }
}

ModPattern* ModModule::getPattern(size_t idx) const
{
    if( idx >= m_patterns.size() )
    {
        return nullptr;
    }
    return m_patterns[idx];
}

light4cxx::Logger* ModModule::logger()
{
    return light4cxx::Logger::get(AbstractModule::logger()->name() + ".mod");
}
}
}
