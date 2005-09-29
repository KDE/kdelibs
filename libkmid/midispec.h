/*  midispec.h  - Some definitions to make the code more readable 
    This file is part of LibKMid 0.9.5
    Copyright (C) 1997,98,99,2000  Antonio Larrosa Jimenez
    LibKMid's homepage : http://www.arrakis.es/~rlarrosa/libkmid.html

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/ 

#ifndef _MIDISPEC_H
#define _MIDISPEC_H

#define META_EVENT 0xFF

#define ME_TRACK_SEQ_NUMBER	0x00
#define ME_TEXT			0x01
#define ME_COPYRIGHT		0x02
#define ME_SEQ_OR_TRACK_NAME	0x03
#define ME_TRACK_INSTR_NAME	0x04
#define ME_LYRIC		0x05
#define ME_MARKER		0x06
#define ME_CUE_POINT		0x07
#define ME_CHANNEL_PREFIX       0x20
#define ME_MIDI_PORT            0x21
#define ME_SET_TEMPO		0x51
#define ME_SMPTE_OFFSET         0x54
#define ME_TIME_SIGNATURE	0x58
#define ME_KEY_SIGNATURE	0x59
/* sf=sharps/flats (-7=7 flats, 0=key of C, 7=7 sharps)
   mi=major/minor (0=major, 1=minor)
*/                            
 
#define ME_END_OF_TRACK 0x2F


#define PERCUSSION_CHANNEL 9

#define KMID_EXTERNAL_MIDI 1
#define KMID_SYNTH         2
#define KMID_FM            3
#define KMID_GUS           4
#define KMID_AWE           5  //For future class aweOut
#define KMID_ALSA          6

#endif
