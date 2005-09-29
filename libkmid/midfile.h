/*  midfile.h  - function which reads a midi file,and creates the track classes
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
#ifndef _MIDFILE_H
#define _MIDFILE_H

#include <libkmid/dattypes.h>
#include <libkmid/track.h>
#include <stdio.h>
#include <kdelibs_export.h>

/**
 * Contains all the information about a MIDI file.
 *
 * @short All the information about a MIDI file.
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */
struct MidiFileInfo
{
  /**
   * Format of MIDI file.
   */ 
  int format;

  /**
   * Number of tracks.
   */
  int ntracks;

  /**
   * Ticks per cuarter note.
   */
  int ticksPerCuarterNote;

  /**
   * Total number of MIDI ticks
   */ 
  ulong ticksTotal;

  /**
   * Total number of milliseconds
   */ 
  double millisecsTotal;

  ulong ticksPlayed;

  /**
   * Patches used in the MIDI file.
   *
   * In each position of the array it stores the number of times the
   * corresponding patch is used. So, if a MIDI file plays 782 notes
   * with a piano, patchesUsed[0] will store 782. In the same way, 
   * if it doesn't use the Music Box patch, patchesUsed[10] will be 0. 
   * 
   */
  int patchesUsed[256];

};

double KMID_EXPORT tempoToMetronomeTempo(ulong x);
double metronomeTempoToTempo(ulong x);

/**
 * Reads a midi file.
 * 
 * @param name the filename of the midi file to load.
 * @param info a pointer to the MidiFileInfo struct that will be
 *     filled with the information of the loaded file.
 * @param ok return status.
 * @return an array of MidiTrack objects with the contents of the file.
 */ 
MidiTrack **readMidiFile( const char *name, MidiFileInfo *info, int &ok);

void parseInfoData( MidiFileInfo *info, MidiTrack **tracks, float ratioTempo);

void parsePatchesUsed( MidiTrack **tracks, MidiFileInfo *info, int gm);

#endif
