/**************************************************************************

    midfile.h  - function which reads a midi file,and creates the track classes
    Copyright (C) 1997,98  Antonio Larrosa Jimenez

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
#ifndef _MIDFILE_H
#define _MIDFILE_H

#include "dattypes.h"
#include "track.h"
#include <stdio.h>

struct midifileinfo
{
int format;
int ntracks;
int ticksPerCuarterNote;


ulong ticksTotal;
double millisecsTotal;
ulong ticksPlayed;

int patchesUsed[256]; // 0 - it's not used
                     // 1 - it's used only once
                     // 2 - if it's used twice
                     // 3 - if it's used three times
                     // and so on
};

double tempoToMetronomeTempo(ulong x);

track **readMidiFile(char *name,midifileinfo *info,int &ok);

void parsePatchesUsed(track **Tracks,midifileinfo *info,int gm);

#endif
