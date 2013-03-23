#ifndef PPPLAY_STANDARDFXDESC_H
#define PPPLAY_STANDARDFXDESC_H

#include "ppplay_core_export.h"

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
	extern PPPLAY_CORE_EXPORT const char* NullFx;
	
	extern PPPLAY_CORE_EXPORT const char* SetVolume;
	extern PPPLAY_CORE_EXPORT const char* SetEnvelopePos;
	
	extern PPPLAY_CORE_EXPORT const char* SlowVolSlideDown;
	extern PPPLAY_CORE_EXPORT const char* VolSlideDown;
	extern PPPLAY_CORE_EXPORT const char* FastVolSlideDown;
	
	extern PPPLAY_CORE_EXPORT const char* SlowVolSlideUp;
	extern PPPLAY_CORE_EXPORT const char* VolSlideUp;
	extern PPPLAY_CORE_EXPORT const char* FastVolSlideUp;

	extern PPPLAY_CORE_EXPORT const char* SlowPitchSlideDown;
	extern PPPLAY_CORE_EXPORT const char* PitchSlideDown;
	extern PPPLAY_CORE_EXPORT const char* FastPitchSlideDown;
	
	extern PPPLAY_CORE_EXPORT const char* SlowPitchSlideUp;
	extern PPPLAY_CORE_EXPORT const char* PitchSlideUp;
	extern PPPLAY_CORE_EXPORT const char* FastPitchSlideUp;
	
	extern PPPLAY_CORE_EXPORT const char* PortaVolSlide;
	extern PPPLAY_CORE_EXPORT const char* VibVolSlide;
	extern PPPLAY_CORE_EXPORT const char* Porta;
	extern PPPLAY_CORE_EXPORT const char* Vibrato;
	extern PPPLAY_CORE_EXPORT const char* FineVibrato;
	extern PPPLAY_CORE_EXPORT const char* Tremolo;
	extern PPPLAY_CORE_EXPORT const char* NoteCut;
	extern PPPLAY_CORE_EXPORT const char* NoteDelay;
	extern PPPLAY_CORE_EXPORT const char* KeyOff;
	extern PPPLAY_CORE_EXPORT const char* SetFinetune;
	extern PPPLAY_CORE_EXPORT const char* SetVibWaveform;
	extern PPPLAY_CORE_EXPORT const char* SetTremWaveform;
	extern PPPLAY_CORE_EXPORT const char* Retrigger;
	extern PPPLAY_CORE_EXPORT const char* Offset;
	extern PPPLAY_CORE_EXPORT const char* Tremor;
	extern PPPLAY_CORE_EXPORT const char* SetTempo;
	extern PPPLAY_CORE_EXPORT const char* SetSpeed;
	extern PPPLAY_CORE_EXPORT const char* Arpeggio;
	extern PPPLAY_CORE_EXPORT const char* Glissando;
	extern PPPLAY_CORE_EXPORT const char* StereoControl;
	extern PPPLAY_CORE_EXPORT const char* GlobalVolume;
	extern PPPLAY_CORE_EXPORT const char* GlobalVolSlideDown;
	extern PPPLAY_CORE_EXPORT const char* GlobalVolSlideUp;
	
	extern PPPLAY_CORE_EXPORT const char* JumpOrder;
	extern PPPLAY_CORE_EXPORT const char* PatternBreak;
	extern PPPLAY_CORE_EXPORT const char* PatternLoop;
	extern PPPLAY_CORE_EXPORT const char* PatternDelay;
	extern PPPLAY_CORE_EXPORT const char* SetPanPos;
	extern PPPLAY_CORE_EXPORT const char* PanSlideLeft;
	extern PPPLAY_CORE_EXPORT const char* PanSlideRight;
}
}

#endif