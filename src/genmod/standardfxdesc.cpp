#include "standardfxdesc.h"

namespace ppp
{
namespace fxdesc
{
const char* NullFx = "      ";

const char* SetVolume = "StVol=";
const char* SetEnvelopePos = "EnvP \x1d";

const char* SlowVolSlideDown = "VSld \x19";
const char* VolSlideDown = "VSld \x1f";
const char* FastVolSlideDown = "VSld\x1f\x1f";

const char* SlowVolSlideUp = "VSld \x18";
const char* VolSlideUp = "VSld \x1e";
const char* FastVolSlideUp = "VSld\x1e\x1e";

const char* SlowPitchSlideDown = "Ptch \x19";
const char* PitchSlideDown = "Ptch \x1f";
const char* FastPitchSlideDown = "Ptch\x1f\x1f";

const char* SlowPitchSlideUp = "Ptch \x18";
const char* PitchSlideUp = "Ptch \x1e";
const char* FastPitchSlideUp = "Ptch\x1e\x1e";

const char* PortaVolSlide = "PrtVo\x12";
const char* VibVolSlide = "VibVo\xf7";
const char* Porta = "Porta\x12";
const char* Vibrato = "Vibr \xf7";
const char* FineVibrato = "FnVib\xf7";
const char* Tremolo = "Tremo\xec";
const char* NoteCut = "NCut \xd4";
const char* NoteDelay = "Delay\xc2";
const char* KeyOff = "KOff \xd4";
const char* Fadeout = "Fade \xd4";
const char* SetFinetune = "FTune\xe6";
const char* SetVibWaveform = "VWave\x9f";
const char* SetTremWaveform = "TWave\x9f";
const char* SetPanbrelloWaveform = "PWave\x9f";
const char* Retrigger = "Retr \xec";
const char* Offset = "Offs \xaa";
const char* Tremor = "Tremr\xec";
const char* SetTempo = "Tempo\x7f";
const char* SetSpeed = "Speed\x7f";
const char* Arpeggio = "Arp  \xf0";
const char* Glissando = "Gliss\xcd";
const char* StereoControl = "SCtrl\x1d";
const char* GlobalVolume = "GloVol";
const char* GlobalVolSlideDown = "GVSld\x1f";
const char* GlobalVolSlideUp = "GVSld\x1e";
const char* ChannelVolume = "ChnVol";
const char* ChannelVolSlideDown = "CVSld\x1f";
const char* ChannelVolSlideUp = "CVSld\x1e";

const char* JumpOrder = "JmOrd\x1a";
const char* NNA = "NNA  @";
const char* EnvelopEnable = "Env  +";
const char* EnvelopDisable = "Env  -";
const char* PatternBreak = "PBrk \xf6";
const char* PatternLoop = "PLoop\xe8";
const char* PatternDelay = "PDlay\xc2";
const char* SetPanPos = "StPan\x1d";
const char* Panbrello = "Panbr\x1d";
const char* PanSlideLeft = "PSld \x1b";
const char* PanSlideRight = "PSld \x1a";
}
}