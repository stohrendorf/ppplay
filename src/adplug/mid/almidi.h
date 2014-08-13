#ifndef PPP_ALMIDI_H
#define PPP_ALMIDI_H

#include "dualchips.h"
#include "stream/stream.h"

namespace ppp {

class EMidi
{
private:
    struct Track;

    DualChips m_chips{};
    Track *m_trackPtr = nullptr;
    uint16_t m_numTracks = 0;

    bool m_loop = false;

    int16_t m_division = 96;
    int  m_tick    = 0;
    int  m_beat    = 1;
    int  m_measure = 1;
    int  m_beatsPerMeasure = 0;
    int  m_ticksPerBeat = 0;
    int  m_timeBase = 0;

    unsigned long m_positionInTicks = 0;

    int  m_context = 0;

    int m_activeTracks = 0;
    int m_totalVolume = 255;

    int m_tempo = 120;

    size_t m_ticksPerSecond = 0;

    std::array<int,16> m_channelVolume{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

    void resetTracks();
    void advanceTick();
    void metaEvent(Track* Track);
    bool interpretControllerInfo(Track *Track, bool TimeSet, int channel, int c1, int c2);
    void setChannelVolume(int channel, int volume);
    void allNotesOff();
    void sendChannelVolumes();
    void reset();
    void setVolume(int volume);
    void setTempo(int tempo);
    void initEmidi();

public:
    EMidi(Stream& stream);
    ~EMidi();

    bool serviceRoutine();
    size_t ticksPerSecond() const noexcept
    {
        return m_ticksPerSecond;
    }
    void read(std::array<int16_t,4>* data) {
        m_chips.read(data);
    }
};

}

#endif
