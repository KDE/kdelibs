/*  notearray.h  - NoteArray class, which holds an array of notes
    This file is part of LibKMid 0.9.5
    Copyright (C) 1998,99,2000  Antonio Larrosa Jimenez
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
#ifndef NOTEARRAY_H
#define NOTEARRAY_H

#include <libkmid/dattypes.h>
#include <kdelibs_export.h>

/**
 * Holds a resizeable array of note on/off and patch change events. It can
 * increase it size, but it doesn't decreases (until destruction :-) )
 *
 * @short Stores an array of note on/off events
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */
class KMID_EXPORT NoteArray
{
  private:
    class NoteArrayPrivate;
    NoteArrayPrivate *d;

  public:
  struct noteCmd {
    /**
     * ms from beginning of song 
     */
    ulong ms; 

    /**
     * The channel
     */
    int chn;

    /**
     * 0 note off, 1 note on, 2 change patch
     */
    int cmd; 

    /**
     * The note.
     *
     * If cmd==2, then the patch is stored in "note"
     */
    int note; 
  };
  
  private:
  noteCmd *data;
  ulong totalAllocated;

  ulong last;
  noteCmd *lastAdded;

  /**
   * @internal
   * The iterator
   */
  noteCmd *it;

  noteCmd *pointerTo(ulong pos);

  public:
  /**
   * Constructor. Initializes internal variables.
   */
  NoteArray(void);
  /**
   * Destructor.
   */
  ~NoteArray();

  /**
   * Adds (or modifies) an event in the given position . 
   *
   * Note that this has nothing to do with what is being played, this just
   * modifies an internal array.
   */
  void at(ulong pos, ulong ms,int chn,int cmd,int note);

  /**
   * A convenience function, which differs from the above in the parameters
   * it accepts.
   */
  void at(ulong pos, noteCmd s);

  /**
   * Returns the note event at a given position.
   */ 
  noteCmd at(int pos);

  /**
   * Adds a note/patch event at a given millisecond.
   * 
   * Note: This method always appends at the end of the list.
   */ 
  void add(ulong ms,int chn,int cmd,int note);

  /**
   * Initializes the iterator.
   *
   * @see get()
   * @see next()
   */ 
  void iteratorBegin(void) { it=data; };

  /**
   * Get the command currently pointed to by the iterator.
   */
  noteCmd *get(void)       { return it; };

  /**
   * Advances the iterator to the next position.
   */ 
  void next(void);

  /**
   * Calls next() until the next event is over ms milliseconds
   * and puts in @p pgm[16] the instruments used at this moment.
   */
  void moveIteratorTo(ulong ms,int *pgm=NULL);
};

#endif
