/**************************************************************************

    notearray.h  - NoteArray class, which holds an array of notes
    Copyright (C) 1998  Antonio Larrosa Jimenez

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

#ifndef NOTEARRAY_H
#define NOTEARRAY_H

// This class holds a resizeable array of noteon/off events, it can increase it size, but
// it doesn't decreases it until destruction :-)
#include "dattypes.h"

struct noteCmd {
    ulong ms; // ms from beginning of song
    int chn;
    int cmd; // 0 note off, 1 note on, 2 change patch
    int note; // if cmd==2 then the patch is stored in "note"
};

class NoteArray
{
    noteCmd *data;
    ulong totalAllocated;

    ulong last;
    noteCmd *lastAdded;

    noteCmd *it; // the iterator
    
    noteCmd *pointerTo(ulong pos);
    
public:
    NoteArray(void);
    ~NoteArray();

    void at(ulong pos, ulong ms,int chn,int cmd,int note);
    void at(ulong pos, noteCmd s);
    noteCmd at(int pos);

    void add(ulong ms,int chn,int cmd,int note);

    void iteratorBegin(void) { it=data; };
    noteCmd *get(void)       { return it; };
    void next(void);
    void moveIteratorTo(ulong ms,int *pgm=NULL);
    // call next() until the next event is over ms milliseconds
    // and puts in pgm[16] the instruments used at this moment

};

#endif
