#ifndef MIDIDATA_H
#define MIDIDATA_H
/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999, 2000, 2001 Simon Peter, <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * FM instrument definitions below borrowed from the Allegro library by
 * Phil Hassey, <philhassey@hotmail.com> - see "adplug/players/mid.cpp"
 * for further acknowledgements.
 */

#include <array>
#include <cstdint>

const std::array<std::array<uint8_t, 14>, 128> midi_fm_instruments = { {
                                                                         /* This set of GM instrument patches was provided by Jorrit Rouwe... */
                                                                         { { 0x21, 0x21, 0x8f, 0x0c, 0xf2, 0xf2, 0x45,
                                                                             0x76, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Acoustic Grand */
                                                                         { { 0x31, 0x21, 0x4b, 0x09, 0xf2, 0xf2, 0x54,
                                                                             0x56, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Bright Acoustic */
                                                                         { { 0x31, 0x21, 0x49, 0x09, 0xf2, 0xf2, 0x55,
                                                                             0x76, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Electric Grand */
                                                                         { { 0xb1, 0x61, 0x0e, 0x09, 0xf2, 0xf3, 0x3b,
                                                                             0x0b, 0x00, 0x00, 0x06, 0, 0,
                                                                             0 } }, /* Honky-Tonk */
                                                                         { { 0x01, 0x21, 0x57, 0x09, 0xf1, 0xf1, 0x38,
                                                                             0x28, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Electric Piano 1 */
                                                                         { { 0x01, 0x21, 0x93, 0x09, 0xf1, 0xf1, 0x38,
                                                                             0x28, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Electric Piano 2 */
                                                                         { { 0x21, 0x36, 0x80, 0x17, 0xa2, 0xf1, 0x01,
                                                                             0xd5, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Harpsichord */
                                                                         { { 0x01, 0x01, 0x92, 0x09, 0xc2, 0xc2, 0xa8,
                                                                             0x58, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Clav */
                                                                         { { 0x0c, 0x81, 0x5c, 0x09, 0xf6, 0xf3, 0x54,
                                                                             0xb5, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Celesta */
                                                                         { { 0x07, 0x11, 0x97, 0x89, 0xf6, 0xf5, 0x32,
                                                                             0x11, 0x00, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Glockenspiel */
                                                                         { { 0x17, 0x01, 0x21, 0x09, 0x56, 0xf6, 0x04,
                                                                             0x04, 0x00, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Music Box */
                                                                         { { 0x18, 0x81, 0x62, 0x09, 0xf3, 0xf2, 0xe6,
                                                                             0xf6, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Vibraphone */
                                                                         { { 0x18, 0x21, 0x23, 0x09, 0xf7, 0xe5, 0x55,
                                                                             0xd8, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Marimba */
                                                                         { { 0x15, 0x01, 0x91, 0x09, 0xf6, 0xf6, 0xa6,
                                                                             0xe6, 0x00, 0x00, 0x04, 0, 0,
                                                                             0 } }, /* Xylophone */
                                                                         { { 0x45, 0x81, 0x59, 0x89, 0xd3, 0xa3, 0x82,
                                                                             0xe3, 0x00, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Tubular Bells */
                                                                         { { 0x03, 0x81, 0x49, 0x89, 0x74, 0xb3, 0x55,
                                                                             0x05, 0x01, 0x00, 0x04, 0, 0,
                                                                             0 } }, /* Dulcimer */
                                                                         { { 0x71, 0x31, 0x92, 0x09, 0xf6, 0xf1, 0x14,
                                                                             0x07, 0x00, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Drawbar Organ */
                                                                         { { 0x72, 0x30, 0x14, 0x09, 0xc7, 0xc7, 0x58,
                                                                             0x08, 0x00, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Percussive Organ */
                                                                         { { 0x70, 0xb1, 0x44, 0x09, 0xaa, 0x8a, 0x18,
                                                                             0x08, 0x00, 0x00, 0x04, 0, 0,
                                                                             0 } }, /* Rock Organ */
                                                                         { { 0x23, 0xb1, 0x93, 0x09, 0x97, 0x55, 0x23,
                                                                             0x14, 0x01, 0x00, 0x04, 0, 0,
                                                                             0 } }, /* Church Organ */
                                                                         { { 0x61, 0xb1, 0x13, 0x89, 0x97, 0x55, 0x04,
                                                                             0x04, 0x01, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Reed Organ */
                                                                         { { 0x24, 0xb1, 0x48, 0x09, 0x98, 0x46, 0x2a,
                                                                             0x1a, 0x01, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Accoridan */
                                                                         { { 0x61, 0x21, 0x13, 0x09, 0x91, 0x61, 0x06,
                                                                             0x07, 0x01, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Harmonica */
                                                                         { { 0x21, 0xa1, 0x13, 0x92, 0x71, 0x61, 0x06,
                                                                             0x07, 0x00, 0x00, 0x06, 0, 0,
                                                                             0 } }, /* Tango Accordian */
                                                                         { { 0x02, 0x41, 0x9c, 0x89, 0xf3, 0xf3, 0x94,
                                                                             0xc8, 0x01, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Acoustic Guitar(nylon) */
                                                                         { { 0x03, 0x11, 0x54, 0x09, 0xf3, 0xf1, 0x9a,
                                                                             0xe7, 0x01, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Acoustic Guitar(steel) */
                                                                         { { 0x23, 0x21, 0x5f, 0x09, 0xf1, 0xf2, 0x3a,
                                                                             0xf8, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Electric Guitar(jazz) */
                                                                         { { 0x03, 0x21, 0x87, 0x89, 0xf6, 0xf3, 0x22,
                                                                             0xf8, 0x01, 0x00, 0x06, 0, 0,
                                                                             0 } }, /* Electric Guitar(clean) */
                                                                         { { 0x03, 0x21, 0x47, 0x09, 0xf9, 0xf6, 0x54,
                                                                             0x3a, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Electric Guitar(muted) */
                                                                         { { 0x23, 0x21, 0x4a, 0x0e, 0x91, 0x84, 0x41,
                                                                             0x19, 0x01, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Overdriven Guitar */
                                                                         { { 0x23, 0x21, 0x4a, 0x09, 0x95, 0x94, 0x19,
                                                                             0x19, 0x01, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Distortion Guitar */
                                                                         { { 0x09, 0x84, 0xa1, 0x89, 0x20, 0xd1, 0x4f,
                                                                             0xf8, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Guitar Harmonics */
                                                                         { { 0x21, 0xa2, 0x1e, 0x09, 0x94, 0xc3, 0x06,
                                                                             0xa6, 0x00, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Acoustic Bass */
                                                                         { { 0x31, 0x31, 0x12, 0x09, 0xf1, 0xf1, 0x28,
                                                                             0x18, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Electric Bass(finger) */
                                                                         { { 0x31, 0x31, 0x8d, 0x09, 0xf1, 0xf1, 0xe8,
                                                                             0x78, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Electric Bass(pick) */
                                                                         { { 0x31, 0x32, 0x5b, 0x09, 0x51, 0x71, 0x28,
                                                                             0x48, 0x00, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Fretless Bass */
                                                                         { { 0x01, 0x21, 0x8b, 0x49, 0xa1, 0xf2, 0x9a,
                                                                             0xdf, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Slap Bass 1 */
                                                                         { { 0x21, 0x21, 0x8b, 0x11, 0xa2, 0xa1, 0x16,
                                                                             0xdf, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Slap Bass 2 */
                                                                         { { 0x31, 0x31, 0x8b, 0x09, 0xf4, 0xf1, 0xe8,
                                                                             0x78, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Synth Bass 1 */
                                                                         { { 0x31, 0x31, 0x12, 0x09, 0xf1, 0xf1, 0x28,
                                                                             0x18, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Synth Bass 2 */
                                                                         { { 0x31, 0x21, 0x15, 0x09, 0xdd, 0x56, 0x13,
                                                                             0x26, 0x01, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Violin */
                                                                         { { 0x31, 0x21, 0x16, 0x09, 0xdd, 0x66, 0x13,
                                                                             0x06, 0x01, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Viola */
                                                                         { { 0x71, 0x31, 0x49, 0x09, 0xd1, 0x61, 0x1c,
                                                                             0x0c, 0x01, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Cello */
                                                                         { { 0x21, 0x23, 0x4d, 0x89, 0x71, 0x72, 0x12,
                                                                             0x06, 0x01, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Contrabass */
                                                                         { { 0xf1, 0xe1, 0x40, 0x09, 0xf1, 0x6f, 0x21,
                                                                             0x16, 0x01, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Tremolo Strings */
                                                                         { { 0x02, 0x01, 0x1a, 0x89, 0xf5, 0x85, 0x75,
                                                                             0x35, 0x01, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Pizzicato Strings */
                                                                         { { 0x02, 0x01, 0x1d, 0x89, 0xf5, 0xf3, 0x75,
                                                                             0xf4, 0x01, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Orchestral Strings */
                                                                         { { 0x10, 0x11, 0x41, 0x09, 0xf5, 0xf2, 0x05,
                                                                             0xc3, 0x01, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Timpani */
                                                                         { { 0x21, 0xa2, 0x9b, 0x0a, 0xb1, 0x72, 0x25,
                                                                             0x08, 0x01, 0x00, 0x0e, 0, 0,
                                                                             0 } }, /* String Ensemble 1 */
                                                                         { { 0xa1, 0x21, 0x98, 0x09, 0x7f, 0x3f, 0x03,
                                                                             0x07, 0x01, 0x01, 0x00, 0, 0,
                                                                             0 } }, /* String Ensemble 2 */
                                                                         { { 0xa1, 0x61, 0x93, 0x09, 0xc1, 0x4f, 0x12,
                                                                             0x05, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* SynthStrings 1 */
                                                                         { { 0x21, 0x61, 0x18, 0x09, 0xc1, 0x4f, 0x22,
                                                                             0x05, 0x00, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* SynthStrings 2 */
                                                                         { { 0x31, 0x72, 0x5b, 0x8c, 0xf4, 0x8a, 0x15,
                                                                             0x05, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Choir Aahs */
                                                                         { { 0xa1, 0x61, 0x90, 0x09, 0x74, 0x71, 0x39,
                                                                             0x67, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Voice Oohs */
                                                                         { { 0x71, 0x72, 0x57, 0x09, 0x54, 0x7a, 0x05,
                                                                             0x05, 0x00, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Synth Voice */
                                                                         { { 0x90, 0x41, 0x00, 0x09, 0x54, 0xa5, 0x63,
                                                                             0x45, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Orchestra Hit */
                                                                         { { 0x21, 0x21, 0x92, 0x0a, 0x85, 0x8f, 0x17,
                                                                             0x09, 0x00, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Trumpet */
                                                                         { { 0x21, 0x21, 0x94, 0x0e, 0x75, 0x8f, 0x17,
                                                                             0x09, 0x00, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Trombone */
                                                                         { { 0x21, 0x61, 0x94, 0x09, 0x76, 0x82, 0x15,
                                                                             0x37, 0x00, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Tuba */
                                                                         { { 0x31, 0x21, 0x43, 0x09, 0x9e, 0x62, 0x17,
                                                                             0x2c, 0x01, 0x01, 0x02, 0, 0,
                                                                             0 } }, /* Muted Trumpet */
                                                                         { { 0x21, 0x21, 0x9b, 0x09, 0x61, 0x7f, 0x6a,
                                                                             0x0a, 0x00, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* French Horn */
                                                                         { { 0x61, 0x22, 0x8a, 0x0f, 0x75, 0x74, 0x1f,
                                                                             0x0f, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Brass Section */
                                                                         { { 0xa1, 0x21, 0x86, 0x8c, 0x72, 0x71, 0x55,
                                                                             0x18, 0x01, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* SynthBrass 1 */
                                                                         { { 0x21, 0x21, 0x4d, 0x09, 0x54, 0xa6, 0x3c,
                                                                             0x1c, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* SynthBrass 2 */
                                                                         { { 0x31, 0x61, 0x8f, 0x09, 0x93, 0x72, 0x02,
                                                                             0x0b, 0x01, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Soprano Sax */
                                                                         { { 0x31, 0x61, 0x8e, 0x09, 0x93, 0x72, 0x03,
                                                                             0x09, 0x01, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Alto Sax */
                                                                         { { 0x31, 0x61, 0x91, 0x09, 0x93, 0x82, 0x03,
                                                                             0x09, 0x01, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Tenor Sax */
                                                                         { { 0x31, 0x61, 0x8e, 0x09, 0x93, 0x72, 0x0f,
                                                                             0x0f, 0x01, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Baritone Sax */
                                                                         { { 0x21, 0x21, 0x4b, 0x09, 0xaa, 0x8f, 0x16,
                                                                             0x0a, 0x01, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Oboe */
                                                                         { { 0x31, 0x21, 0x90, 0x09, 0x7e, 0x8b, 0x17,
                                                                             0x0c, 0x01, 0x01, 0x06, 0, 0,
                                                                             0 } }, /* English Horn */
                                                                         { { 0x31, 0x32, 0x81, 0x09, 0x75, 0x61, 0x19,
                                                                             0x19, 0x01, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Bassoon */
                                                                         { { 0x32, 0x21, 0x90, 0x09, 0x9b, 0x72, 0x21,
                                                                             0x17, 0x00, 0x00, 0x04, 0, 0,
                                                                             0 } }, /* Clarinet */
                                                                         { { 0xe1, 0xe1, 0x1f, 0x09, 0x85, 0x65, 0x5f,
                                                                             0x1a, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Piccolo */
                                                                         { { 0xe1, 0xe1, 0x46, 0x09, 0x88, 0x65, 0x5f,
                                                                             0x1a, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Flute */
                                                                         { { 0xa1, 0x21, 0x9c, 0x09, 0x75, 0x75, 0x1f,
                                                                             0x0a, 0x00, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Recorder */
                                                                         { { 0x31, 0x21, 0x8b, 0x09, 0x84, 0x65, 0x58,
                                                                             0x1a, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Pan Flute */
                                                                         { { 0xe1, 0xa1, 0x4c, 0x09, 0x66, 0x65, 0x56,
                                                                             0x26, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Blown Bottle */
                                                                         { { 0x62, 0xa1, 0xcb, 0x09, 0x76, 0x55, 0x46,
                                                                             0x36, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Skakuhachi */
                                                                         { { 0x62, 0xa1, 0xa2, 0x09, 0x57, 0x56, 0x07,
                                                                             0x07, 0x00, 0x00, 0x0b, 0, 0,
                                                                             0 } }, /* Whistle */
                                                                         { { 0x62, 0xa1, 0x9c, 0x09, 0x77, 0x76, 0x07,
                                                                             0x07, 0x00, 0x00, 0x0b, 0, 0,
                                                                             0 } }, /* Ocarina */
                                                                         { { 0x22, 0x21, 0x59, 0x09, 0xff, 0xff, 0x03,
                                                                             0x0f, 0x02, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Lead 1 (square) */
                                                                         { { 0x21, 0x21, 0x0e, 0x09, 0xff, 0xff, 0x0f,
                                                                             0x0f, 0x01, 0x01, 0x00, 0, 0,
                                                                             0 } }, /* Lead 2 (sawtooth) */
                                                                         { { 0x22, 0x21, 0x46, 0x89, 0x86, 0x64, 0x55,
                                                                             0x18, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Lead 3 (calliope) */
                                                                         { { 0x21, 0xa1, 0x45, 0x09, 0x66, 0x96, 0x12,
                                                                             0x0a, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Lead 4 (chiff) */
                                                                         { { 0x21, 0x22, 0x8b, 0x09, 0x92, 0x91, 0x2a,
                                                                             0x2a, 0x01, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Lead 5 (charang) */
                                                                         { { 0xa2, 0x61, 0x9e, 0x49, 0xdf, 0x6f, 0x05,
                                                                             0x07, 0x00, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Lead 6 (voice) */
                                                                         { { 0x20, 0x60, 0x1a, 0x09, 0xef, 0x8f, 0x01,
                                                                             0x06, 0x00, 0x02, 0x00, 0, 0,
                                                                             0 } }, /* Lead 7 (fifths) */
                                                                         { { 0x21, 0x21, 0x8f, 0x86, 0xf1, 0xf4, 0x29,
                                                                             0x09, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Lead 8 (bass+lead) */
                                                                         { { 0x77, 0xa1, 0xa5, 0x09, 0x53, 0xa0, 0x94,
                                                                             0x05, 0x00, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* Pad 1 (new age) */
                                                                         { { 0x61, 0xb1, 0x1f, 0x89, 0xa8, 0x25, 0x11,
                                                                             0x03, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Pad 2 (warm) */
                                                                         { { 0x61, 0x61, 0x17, 0x09, 0x91, 0x55, 0x34,
                                                                             0x16, 0x00, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Pad 3 (polysynth) */
                                                                         { { 0x71, 0x72, 0x5d, 0x09, 0x54, 0x6a, 0x01,
                                                                             0x03, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Pad 4 (choir) */
                                                                         { { 0x21, 0xa2, 0x97, 0x09, 0x21, 0x42, 0x43,
                                                                             0x35, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Pad 5 (bowed) */
                                                                         { { 0xa1, 0x21, 0x1c, 0x09, 0xa1, 0x31, 0x77,
                                                                             0x47, 0x01, 0x01, 0x00, 0, 0,
                                                                             0 } }, /* Pad 6 (metallic) */
                                                                         { { 0x21, 0x61, 0x89, 0x0c, 0x11, 0x42, 0x33,
                                                                             0x25, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Pad 7 (halo) */
                                                                         { { 0xa1, 0x21, 0x15, 0x09, 0x11, 0xcf, 0x47,
                                                                             0x07, 0x01, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Pad 8 (sweep) */
                                                                         { { 0x3a, 0x51, 0xce, 0x09, 0xf8, 0x86, 0xf6,
                                                                             0x02, 0x00, 0x00, 0x02, 0, 0,
                                                                             0 } }, /* FX 1 (rain) */
                                                                         { { 0x21, 0x21, 0x15, 0x09, 0x21, 0x41, 0x23,
                                                                             0x13, 0x01, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* FX 2 (soundtrack) */
                                                                         { { 0x06, 0x01, 0x5b, 0x09, 0x74, 0xa5, 0x95,
                                                                             0x72, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* FX 3 (crystal) */
                                                                         { { 0x22, 0x61, 0x92, 0x8c, 0xb1, 0xf2, 0x81,
                                                                             0x26, 0x00, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* FX 4 (atmosphere) */
                                                                         { { 0x41, 0x42, 0x4d, 0x09, 0xf1, 0xf2, 0x51,
                                                                             0xf5, 0x01, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* FX 5 (brightness) */
                                                                         { { 0x61, 0xa3, 0x94, 0x89, 0x11, 0x11, 0x51,
                                                                             0x13, 0x01, 0x00, 0x06, 0, 0,
                                                                             0 } }, /* FX 6 (goblins) */
                                                                         { { 0x61, 0xa1, 0x8c, 0x89, 0x11, 0x1d, 0x31,
                                                                             0x03, 0x00, 0x00, 0x06, 0, 0,
                                                                             0 } }, /* FX 7 (echoes) */
                                                                         { { 0xa4, 0x61, 0x4c, 0x09, 0xf3, 0x81, 0x73,
                                                                             0x23, 0x01, 0x00, 0x04, 0, 0,
                                                                             0 } }, /* FX 8 (sci-fi) */
                                                                         { { 0x02, 0x07, 0x85, 0x0c, 0xd2, 0xf2, 0x53,
                                                                             0xf6, 0x00, 0x01, 0x00, 0, 0,
                                                                             0 } }, /* Sitar */
                                                                         { { 0x11, 0x13, 0x0c, 0x89, 0xa3, 0xa2, 0x11,
                                                                             0xe5, 0x01, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Banjo */
                                                                         { { 0x11, 0x11, 0x06, 0x09, 0xf6, 0xf2, 0x41,
                                                                             0xe6, 0x01, 0x02, 0x04, 0, 0,
                                                                             0 } }, /* Shamisen */
                                                                         { { 0x93, 0x91, 0x91, 0x09, 0xd4, 0xeb, 0x32,
                                                                             0x11, 0x00, 0x01, 0x08, 0, 0,
                                                                             0 } }, /* Koto */
                                                                         { { 0x04, 0x01, 0x4f, 0x09, 0xfa, 0xc2, 0x56,
                                                                             0x05, 0x00, 0x00, 0x0c, 0, 0,
                                                                             0 } }, /* Kalimba */
                                                                         { { 0x21, 0x22, 0x49, 0x09, 0x7c, 0x6f, 0x20,
                                                                             0x0c, 0x00, 0x01, 0x06, 0, 0,
                                                                             0 } }, /* Bagpipe */
                                                                         { { 0x31, 0x21, 0x85, 0x09, 0xdd, 0x56, 0x33,
                                                                             0x16, 0x01, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Fiddle */
                                                                         { { 0x20, 0x21, 0x04, 0x8a, 0xda, 0x8f, 0x05,
                                                                             0x0b, 0x02, 0x00, 0x06, 0, 0,
                                                                             0 } }, /* Shanai */
                                                                         { { 0x05, 0x03, 0x6a, 0x89, 0xf1, 0xc3, 0xe5,
                                                                             0xe5, 0x00, 0x00, 0x06, 0, 0,
                                                                             0 } }, /* Tinkle Bell */
                                                                         { { 0x07, 0x02, 0x15, 0x09, 0xec, 0xf8, 0x26,
                                                                             0x16, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Agogo */
                                                                         { { 0x05, 0x01, 0x9d, 0x09, 0x67, 0xdf, 0x35,
                                                                             0x05, 0x00, 0x00, 0x08, 0, 0,
                                                                             0 } }, /* Steel Drums */
                                                                         { { 0x18, 0x12, 0x96, 0x09, 0xfa, 0xf8, 0x28,
                                                                             0xe5, 0x00, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Woodblock */
                                                                         { { 0x10, 0x00, 0x86, 0x0c, 0xa8, 0xfa, 0x07,
                                                                             0x03, 0x00, 0x00, 0x06, 0, 0,
                                                                             0 } }, /* Taiko Drum */
                                                                         { { 0x11, 0x10, 0x41, 0x0c, 0xf8, 0xf3, 0x47,
                                                                             0x03, 0x02, 0x00, 0x04, 0, 0,
                                                                             0 } }, /* Melodic Tom */
                                                                         { { 0x01, 0x10, 0x8e, 0x09, 0xf1, 0xf3, 0x06,
                                                                             0x02, 0x02, 0x00, 0x0e, 0, 0,
                                                                             0 } }, /* Synth Drum */
                                                                         { { 0x0e, 0xc0, 0x00, 0x09, 0x1f, 0x1f, 0x00,
                                                                             0xff, 0x00, 0x03, 0x0e, 0, 0,
                                                                             0 } }, /* Reverse Cymbal */
                                                                         { { 0x06, 0x03, 0x80, 0x91, 0xf8, 0x56, 0x24,
                                                                             0x84, 0x00, 0x02, 0x0e, 0, 0,
                                                                             0 } }, /* Guitar Fret Noise */
                                                                         { { 0x0e, 0xd0, 0x00, 0x0e, 0xf8, 0x34, 0x00,
                                                                             0x04, 0x00, 0x03, 0x0e, 0, 0,
                                                                             0 } }, /* Breath Noise */
                                                                         { { 0x0e, 0xc0, 0x00, 0x09, 0xf6, 0x1f, 0x00,
                                                                             0x02, 0x00, 0x03, 0x0e, 0, 0,
                                                                             0 } }, /* Seashore */
                                                                         { { 0xd5, 0xda, 0x95, 0x49, 0x37, 0x56, 0xa3,
                                                                             0x37, 0x00, 0x00, 0x00, 0, 0,
                                                                             0 } }, /* Bird Tweet */
                                                                         { { 0x35, 0x14, 0x5c, 0x11, 0xb2, 0xf4, 0x61,
                                                                             0x15, 0x02, 0x00, 0x0a, 0, 0,
                                                                             0 } }, /* Telephone ring */
                                                                         { { 0x0e, 0xd0, 0x00, 0x09, 0xf6, 0x4f, 0x00,
                                                                             0xf5, 0x00, 0x03, 0x0e, 0, 0,
                                                                             0 } }, /* Helicopter */
                                                                         { { 0x26, 0xe4, 0x00, 0x09, 0xff, 0x12, 0x01,
                                                                             0x16, 0x00, 0x01, 0x0e, 0, 0,
                                                                             0 } }, /* Applause */
                                                                         { { 0x00, 0x00, 0x00, 0x09, 0xf3, 0xf6, 0xf0,
                                                                             0xc9, 0x00, 0x02, 0x0e, 0, 0,
                                                                             0 } }  /* Gunshot */
                                                                       } };

/* logarithmic relationship between midi and FM volumes */
const std::array<uint8_t, 128> my_midi_fm_vol_table = {
  { 0, 11, 16, 19, 22, 25, 27, 29, 32, 33, 35, 37, 39, 40, 42, 43, 45, 46, 48,
    49, 50, 51, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 64, 64, 65, 66, 67, 68,
    69, 70, 71, 72, 73, 74, 75, 75, 76, 77, 78, 79, 80, 80, 81, 82, 83, 83, 84,
    85, 86, 86, 87, 88, 89, 89, 90, 91, 91, 92, 93, 93, 94, 95, 96, 96, 97, 97,
    98, 99, 99, 100, 101, 101, 102, 103, 103, 104, 104, 105, 106, 106, 107, 107,
    108, 109, 109, 110, 110, 111, 112, 112, 113, 113, 114, 114, 115, 115, 116,
    117, 117, 118, 118, 119, 119, 120, 120, 121, 121, 122, 122, 123, 123, 124,
    124, 125, 125, 126, 126, 127 }
};

#endif
