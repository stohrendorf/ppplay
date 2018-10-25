#ifndef PPPLAY_STANDARDFXDESC_H
#define PPPLAY_STANDARDFXDESC_H

namespace ppp
{
/**
 * @namespace ppp::fxdesc
 * @ingroup GenMod
 * @brief Contains standard fx descriptions
 * @see ppp::ChannelState::fxDesc
 */
namespace fxdesc
{
extern const char* NullFx;

extern const char* SetVolume;
extern const char* SetEnvelopePos;

extern const char* SlowVolSlideDown;
extern const char* VolSlideDown;
extern const char* FastVolSlideDown;

extern const char* SlowVolSlideUp;
extern const char* VolSlideUp;
extern const char* FastVolSlideUp;

extern const char* SlowPitchSlideDown;
extern const char* PitchSlideDown;
extern const char* FastPitchSlideDown;

extern const char* SlowPitchSlideUp;
extern const char* PitchSlideUp;
extern const char* FastPitchSlideUp;

extern const char* PortaVolSlide;
extern const char* VibVolSlide;
extern const char* Porta;
extern const char* Vibrato;
extern const char* FineVibrato;
extern const char* Tremolo;
extern const char* NoteCut;
extern const char* NoteDelay;
extern const char* KeyOff;
extern const char* Fadeout;
extern const char* SetFinetune;
extern const char* SetVibWaveform;
extern const char* SetTremWaveform;
extern const char* SetPanbrelloWaveform;
extern const char* Retrigger;
extern const char* Offset;
extern const char* Tremor;
extern const char* SetTempo;
extern const char* SetSpeed;
extern const char* Arpeggio;
extern const char* Glissando;
extern const char* StereoControl;
extern const char* GlobalVolume;
extern const char* GlobalVolSlideDown;
extern const char* GlobalVolSlideUp;
extern const char* ChannelVolume;
extern const char* ChannelVolSlideUp;
extern const char* ChannelVolSlideDown;

extern const char* JumpOrder;
extern const char* NNA;
extern const char* EnvelopEnable;
extern const char* EnvelopDisable;
extern const char* PatternBreak;
extern const char* PatternLoop;
extern const char* PatternDelay;
extern const char* SetPanPos;
extern const char* Panbrello;
extern const char* PanSlideLeft;
extern const char* PanSlideRight;
}
}

#endif
