#ifndef PPP_ALMIDI_H
#define PPP_ALMIDI_H

#include "multichips.h"
#include "stream/stream.h"

namespace ppp {

class EMidi
{
    DISABLE_COPY(EMidi)
private:
    struct Track;

    MultiChips m_chips;
    std::vector<Track> m_tracks;

    bool m_loop = false;

    int16_t m_division = 96;

    struct SongContext;

    struct Timing {
        int  tick    = 0;
        int  beat    = 1;
        int  measure = 1;
        int  beatsPerMeasure = 4;
        int  ticksPerBeat = 0;
        int  timeBase = 4;
    };

    Timing m_timing{};

    unsigned long m_positionInTicks = 0;

    int  m_context = 0;

    int m_activeTracks = 0;
    int m_totalVolume = 255;

    int m_tempo = 120;

    size_t m_ticksPerSecond = 0;

    std::array<int,16> m_channelVolume{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

    enum class Format {
        PlainMidi,
        IdMus
    };

    Format m_format = Format::PlainMidi;

    void resetTracks();
    void advanceTick();
    void metaEvent(Track& track);
    bool interpretControllerInfo(Track& track, bool timeSet, int channel, uint8_t c1, uint8_t c2);
    void setChannelVolume(int channel, int volume);
    void allNotesOff();
    void sendChannelVolumes();
    void reset();
    void setVolume(int volume);
    void setTempo(int tempo);
    void initEmidi();
    bool tryLoadMidi(Stream &stream);
    bool tryLoadMus(Stream &stream);
    bool serviceRoutineMidi();
    bool serviceRoutineMus();

public:
    EMidi(Stream& stream, size_t chipCount, bool stereo);
    ~EMidi();

    bool serviceRoutine();
    size_t ticksPerSecond() const noexcept {
        return m_ticksPerSecond;
    }
    void read(std::array<int16_t,4>* data) {
        m_chips.read(data);
    }
};

}

#endif
