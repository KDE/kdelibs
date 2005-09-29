/*  track.h  - class track, which has a midi file track and its events
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
#ifndef _TRACK_H
#define _TRACK_H

#include <stdio.h>
#include <libkmid/dattypes.h>

/**
 * An structure that represents a MIDI event.
 *
 * @short Represents a MIDI event
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */ 
struct MidiEvent
{
  /**
   * MIDI Command
   * 
   * Caution, if a command doesn't use a variable, it may contain garbage.
   */
  uchar	command;

  /**
   * Channel
   */
  uchar   chn;

  /**
   * Note
   */
  uchar   note;

  /**
   * Velocity
   */
  uchar   vel;

  /**
   * Patch (if command was a change patch command)
   */
  uchar   patch;

  /**
   * Patch (if command was a controller command)
   */
  uchar   ctl;

  /**
   * Data 1
   */
  uchar   d1;

  /**
   * Data 2
   */
  uchar   d2;

  /**
   * Data 3
   */
  uchar   d3;

  /**
   * Data 4
   */
  uchar   d4;

  /**
   * Data 5
   */
  uchar   d5;

  /**
   * Data 6
   */
  uchar   d6;

  /**
   * Length of the generic data variable
   */
  ulong	length;

  /**
   * The data for commands like text, sysex, etc.
   */
  uchar  *data;

};

/**
 * Stores a MIDI track. This can be thought of as a list of MIDI events.
 *
 * The data types used to store the track is similar to how events are
 * stored on a MIDI file, but used in a way that allows for faster parses.
 * 
 * This class is used on MidiPlayer::loadSong() to load the song and
 * later play it with MidiPlayer::play().
 *
 * @short Stores a MIDI track with a simple API
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */
class MidiTrack
{
  private:
    class MidiTrackPrivate;
    MidiTrackPrivate *d;

    int	id;

    ulong size;
    uchar *data;
    uchar *ptrdata;

    bool  note[16][128]; // Notes that are set on or off by this track
    ulong current_ticks; // Total number of ticks since beginning of song
    ulong delta_ticks;   // Delta ticks from previous event to next event
    ulong wait_ticks;    // Wait ticks from previous event in other track
    // to next event in this track

    ulong currentpos; // Some songs don't have a endoftrack event, so
    // we have to see when currentpos > size
    int	endoftrack;

    ulong readVariableLengthValue(void);

    uchar lastcommand;  // This is to run light without overbyte :-)


    double current_time; // in ms.
    double time_at_previous_tempochange;  // in ms.
    double ticks_from_previous_tempochange; // in ticks	
    //	double	time_to_next_event;  // in ms.
    double	time_at_next_event;  // in ms.
    int	tPCN;
    ulong	tempo;

    int	power2to(int i);

  public:
    /**
     * Constructor. 
     * @param file the file to read the track from. It should be ready at the
     * start of a track. MidiTrack reads just that track and the file is left at
     * the end of this track).
     * @param tpcn the ticks per cuarter note used in this file.
     * @param Id the ID for this track.
     */ 
    MidiTrack(FILE *file,int tpcn,int Id);

    /**
     * Destructor
     */
    ~MidiTrack();

    /**
     * Makes the iterator advance the given number of ticks.
     *
     * @return 0 if OK, and 1 if you didn't handle this track well and you
     * forgot to take an event (thing that will never happen if you use
     * MidiPlayer::play() ).
     */ 
    int	ticksPassed (ulong ticks); 

    /**
     * Makes the iterator advance the given number of milliseconds.
     *
     * @return 0 if OK, and 1 if you didn't handle this track well and you
     * forgot to take an event (thing that will never happen if you use
     * MidiPlayer::play() ).
     */
    int	msPassed    (ulong ms);

    /**
     * Returns the current millisecond which the iterator is at.
     */
    int	currentMs   (double ms);

    /**
     * Returns the number of ticks left for the next event.
     */
    ulong	waitTicks   (void) { return wait_ticks; };

    //	ulong   waitMs (void) {return time_to_next_event;};

    /**
     * Returns the absolute number of milliseconds of the next event.
     */ 
    double   absMsOfNextEvent (void) { return time_at_next_event; };

    /**
     * Change the tempo of the song.
     */
    void 	changeTempo(ulong t);

    /**
     * Reads the event at the iterator position, and puts it on the structure
     * pointed to by @p ev.
     */
    void	readEvent(MidiEvent *ev);

    /**
     * Initializes the iterator.
     */
    void	init(void);

    /**
     * Clears the internal variables.
     */
    void	clear(void);

};

#endif
