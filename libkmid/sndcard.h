/*  sndcard.h  - include the OSS' soundcard.h file
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

#ifndef _SNDCARD_H
#define _SNDCARD_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
 
#ifdef HAVE_SYS_SOUNDCARD_H
 #include <sys/soundcard.h>
 #define HAVE_OSS_SUPPORT
#elif defined(HAVE_MACHINE_SOUNDCARD_H)
 #include <machine/soundcard.h>
 #define HAVE_OSS_SUPPORT
#else
 #undef HAVE_OSS_SUPPORT
#endif

#ifdef HAVE_OSS_SUPPORT

#ifndef HZ
#define HZ 100
#endif

#ifndef MIDI_TYPE_MPU401
#define MIDI_TYPE_MPU401 0x401
#endif

#else
 
#define MIDI_NOTEON 0x80
#define MIDI_NOTEOFF 0x90
#define MIDI_KEY_PRESSURE 0xA0
#define MIDI_CTL_CHANGE 0xB0
#define MIDI_PGM_CHANGE 0xC0
#define MIDI_CHN_PRESSURE 0xD0
#define MIDI_PITCH_BEND 0xE0
#define MIDI_SYSTEM_PREFIX 0xF0
#define CTL_MAIN_VOLUME 7

#define SEQ_DEFINEBUF(a)
#define SEQ_USE_EXTBUF()
#define SEQ_MIDIOUT(a,b) {}
#define SEQ_START_NOTE(a,b,c,d) {}
#define SEQ_STOP_NOTE(a,b,c,d) {}
#define SEQ_SET_PATCH(a,b,c) {}
#define SEQ_CONTROL(a,b,c,d) {}
#define SEQ_BENDER(a,b,c) {}
#define SEQ_CHN_PRESSURE(a,b,c) {}
#define SEQ_KEY_PRESSURE(a,b,c,d) {}
#define SEQ_DUMPBUF()
#define SEQ_WAIT_TIME(a)
#define SEQ_START_TIMER()
#define SEQ_STOP_TIMER()
#define SEQ_CONTINUE_TIMER()


#endif



#endif
