/**************************************************************************

    midispec.h  - Some definitions to make more readable the code 
    Copyright (C) 1997  Antonio Larrosa Jimenez

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Send comments and bug fixes to antlarr@arrakis.es
    or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain

***************************************************************************/ 
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
/*                                    sf=sharps/flats (-7=7 flats, 0=key of C,^M
                                    7=7 sharps)
                                    mi=major/minor (0=major, 1=minor)
*/                              
 
#define ME_END_OF_TRACK 0x2F
