/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef S3MBASE_H
#define S3MBASE_H

#include "genmod/genbase.h"

#include <cstdint>

/**
 * @defgroup S3mMod ScreamTracker 3 module definitions
 * @brief This module contains the ScreamTracker 3 Module Classes
 * @remarks I am very proud that this should be the @b most @b compatible playing
 * routine ever.
 */

/**
 * @file
 * @ingroup S3mMod
 * @brief This file contains the base definitions for S3M Modules
 */

namespace ppp {
/**
 * @namespace ppp::s3m
 * @ingroup S3mMod
 * @brief This namespace contains some consts that do not need to be public
 */
namespace s3m {

/**
 * @brief Some default values...
 * @ingroup S3mMod
 */
enum : uint8_t {
	s3mEmptyNote    = 0xff, //!< @brief Empty note
	s3mKeyOffNote   = 0xfe, //!< @brief Key off note
	s3mEmptyInstr   = 0x00, //!< @brief Empty sample
	s3mEmptyVolume  = 0xff, //!< @brief Empty volume
	s3mEmptyCommand = 0x00, //!< @brief Empty command
	s3mOrderEnd     = 0xff, //!< @brief "--" Marker (End of Song)
	s3mOrderSkip    = 0xfe  //!< @brief "++" Marker (Skip this order)
};

/**
 * @brief Tracker IDs
 * @ingroup S3mMod
 */
enum : uint8_t {
	s3mTIdScreamTracker = 0x01, //!< @brief Scream Tracker 3
	s3mTIdImagoOrpheus = 0x02, //!< @brief Imago Orpheus
	s3mTIdImpulseTracker = 0x03, //!< @brief Impulse Tracker
	s3mTIdSchismTracker = 0x04, //!< @brief Schism Tracker
	s3mTIdOpenMPT = 0x05 //!< @brief OpenMPT
};

/**
 * @brief Commands/effects
 * @ingroup S3mMod
 * @remarks @e FX @e Data @e Share means that if the value is 0x00, the last non-zero value is used. A special case is
 * ::s3mFxVibrato (and others), where each nibble is treated separately. If there is @e own @e field mentioned, that
 * means that this effect has its own backup variable for saving the last used effect value. See ::s3mFxVibVolSlide for
 * example.
 */
enum : uint8_t {
	/**
	 * @brief (A) Set speed
	 * @note Global
	 * @remarks Ignored if 0
	 */
	s3mFxSpeed        =  1,
	/**
	 * @brief (B) Jump to order
	 * @note Global
	 * @remarks Restarts current order/pattern if out of range
	 */
	s3mFxJumpOrder    =  2,
	/**
	 * @brief (C) Break to next pattern, row in FX Data
	 * @note Global
	 * @remarks Ignored if out of range
	 */
	s3mFxBreakPat     =  3,
	/**
	 * @brief (D) Volume slide
	 * @note FX Data Share
	 * @see s3mFlag300Slides
	 * @details
	 * Testing some cases with STv3.21 this came out with the effect values:
	 * @li @c 0x01 to @c 0x0f is a @e Slide @e Down
	 * @li @c 0x10 to @c 0xf0 is a @e Slide @e Up
	 * @li @c 0xf1 to @c 0xfe is a @e Fine @e Slide @e Down
	 * @li @c 0x1f to @c 0xff is a @e Fine @e Slide @e Up
	 * @li All others are considered @e Slide @e Downs with the low-nibble value
	 *
	 * Pseudo-code:
	 * @code
	 * if(highNibble(fxVal)==0x00)
	 *   slideDown(lowNibble(fxVal));
	 * else if(lowNibble(fxVal)==0x00)
	 *   slideUp(highNibble(fxVal));
	 * else if(highNibble(fxVal)==0x0f)
	 *   fineSlideDown(highNibble(fxVal));
	 * else if(lowNibble(fxVal)==0x0f)
	 *   fineSlideUp(highNibble(fxVal));
	 * else
	 *   slideDown(lowNibble(fxVal));
	 * @endcode
	 * @see S3mChannel::doVolumeFx
	 */
	s3mFxVolSlide     =  4,
	//! @brief (E) Pitch slide down
	//! @note FX Data Share
	s3mFxPitchDown    =  5,
	//! @brief (F) Pitch slide up
	//! @note FX Data Share
	s3mFxPitchUp      =  6,
	//! @brief (G) Pitch slide to note
	//! @note FX Data Share (own field)
	//! @see S3mChannel::aLastPortaFx
	s3mFxPorta        =  7,
	//! @brief (H) Vibrato
	//! @note FX Data Share (Nibbles, own field)
	//! @see S3mChannel::aLastVibratoFx
	s3mFxVibrato      =  8,
	//! @brief (I) Tremor
	//! @note FX Data Share
	//! @note Continuous over rows
	//! @remarks Accepts nibbles greater than speed
	//! @remarks 1 is added to the nibbles, thus i.e. @c I11 is 2 ticks on and 2 ticks off
	s3mFxTremor       =  9,
	//! @brief (J) Arpeggio
	//! @note FX Data Share (Nibbles)
	s3mFxArpeggio     = 10,
	//! @brief (K) Continue Vibrato, add Volume slide
	//! @note FX Data Share
	//! @note If a value of 0 is given, it uses the value of the last vibrato FX
	//! @note If one of the nibbles is 0x0f, this FX is ignored
	s3mFxVibVolSlide  = 11,
	//! @brief (L) Continue Pitch slide, add Volume slide
	//! @note FX Data Share
	//! @note If a value of 0 is given, it uses the value of the last porta FX
	//! @note If one of the nibbles is 0x0f, this FX is ignored
	s3mFxPortaVolSlide = 12,
	//! @brief (M) Set channel volume
	//! @remarks Originally not supported
	s3mFxChanVolume   = 13,
	//! @brief (N) Channel volume slide
	//! @note FX Data Share
	//! @remarks Originally not supported, and it won't be...
	s3mFxChanVolSlide = 14,
	//! @brief (O) Set sample offset
	//! @note FX Data Share
	//! @remarks Does not play if value out of range
	//! @remarks Ignored if not combined with a note
	s3mFxOffset       = 15,
	//! @brief (P) Panning slide
	//! @note FX Data Share
	//! @remarks Originally not supported
	s3mFxPanSlide     = 16,
	//! @brief (Q) Retrigger
	//! @note FX Data Share
	//! @remarks Accepts Values greater than Speed
	//! @note Continuous over rows
	s3mFxRetrig       = 17,
	//! @brief (R) Tremolo
	//! @see s3mFxVibrato
	s3mFxTremolo      = 18,
	//! @brief (S) Special FX
	s3mFxSpecial      = 19,
	//! @brief (T) Set tempo
	//! @note Global
	//! @remarks Ignored if not within @c 0x21 to @c 0xff
	s3mFxTempo        = 20,
	//! @brief (U) Fine vibrato
	//! @note FX Data Share (Nibbles)
	s3mFxFineVibrato  = 21,
	//! @brief (V) Set global volume
	//! @note Global
	//! @remarks Does not affect already playing notes, and is applied @e after tick 0
	s3mFxGlobalVol    = 22,
	//! @brief (W) Global volume slide
	//! @note Global
	//! @note FX Data Share
	//! @remarks Originally not supported, and it won't be...
	s3mFxGlobVolSlide = 23,
	//! @brief (X) Set panning (0x00..0x80)
	//! @remarks Originally not supported
	//! @note @c 0xa4 = Surround sound (that is practically Center Panning with negated Right sample values)
	s3mFxSetPanning   = 24,
	//! @brief (Y) Panbrello
	//! @remarks Even not originally supported, I've found an S3M with it (probably a converted IT), but it won't be implemented...
	s3mFxPanbrello    = 25,
	//! @brief (Z) Resonance filter
	//! @remarks Not supported
	s3mFxResonance    = 26
};

/**
 * @brief Special Commands/effects
 * @ingroup S3mMod
 * @see s3mFxSpecial
 */
enum : uint8_t {
	//! @brief Special FX: Set filter
	//! @remarks Not supported
	s3mSFxSetFilter    = 0x00,
	//! @brief Special FX: Set Glissando control
	//! @remarks Not supported
	s3mSFxSetGlissando = 0x01,
	//! @brief Special FX: Set Finetune
	//! @todo Implementation needed
	//! @note Is this a per-channel or a per-sample effect?
	//! @see S3mChannel::doSpecialFx
	s3mSFxSetFinetune  = 0x02,
	//! @brief Special FX: Set Vibrato Waveform
	//! @note Only the low 2 bits are interesting, retriggering is @e never done
	s3mSFxSetVibWave   = 0x03,
	//! @brief Special FX: Set Tremolo Waveform
	//! @see ::s3mSFxSetVibWave
	s3mSFxSetTremWave  = 0x04,
	//! @brief Special FX: Set Panning position
	s3mSFxSetPan       = 0x08,
	//! @brief Special FX: Stereo control
	//! @remarks Originally not supported
	s3mSFxStereoCtrl   = 0x0a,
	//! @brief Special FX: Pattern loop
	//! @remarks Sets loop point after loop end point when finished
	//! @remarks Multiple loop end points on one row create an infinite loop.
	//! Due to practical reasons, the loop counter is set to 127.
	s3mSFxPatLoop      = 0x0b,
	//! @brief Special FX: Note cut
	//! @remarks Ignored if 0 or >=Speed
	//! @remarks There seem to be some modules out there with version tag "ScreamTracker v3.20"
	//! that use "SC0" (i.e. "Space Debris Remix"). The question is: How to handle that???
	//! And... from which tracker do they come from?
	s3mSFxNoteCut      = 0x0c,
	//! @brief Special FX: Note delay
	//! @remarks Don't play if 0 or >=Speed
	s3mSFxNoteDelay    = 0x0d,
	//! @brief Special FX: Pattern delay
	//! @note Global
	//! @remarks If multiple pattern delays on one row are specified, only the first one is used
	//! @remarks A value of 0 is ignored
	s3mSFxPatDelay     = 0x0e,
	//! @brief Special FX: Funk repeat
	//! @remarks Not supported
	s3mSFxFunkRpt      = 0x0f
};
} // namespace s3m
} // namespace ppp

#endif

