/**************************************************************************

    track.h  - class track, which has a midi file track and its events
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
#ifndef _TRACK_H
#define _TRACK_H

#include <stdio.h>
#include "dattypes.h"

struct Midi_event
{
	uchar	command;
        uchar   chn;
	uchar   note;
	uchar   vel;
	uchar   patch;
	uchar   ctl;
	uchar   d1;
	uchar   d2;
	uchar   d3;
	uchar   d4;
	uchar   d5;
	uchar   d6;
	ulong	length;
	uchar  *data;

// note that if a command doesn't use a variable it will contain garbage

};

class track
{
private:
	int	id;

	ulong	size;
	uchar  *data;
	uchar  *ptrdata;
	
	ulong	current_ticks; // Total number of ticks since beggining of song
	ulong    delta_ticks;   // Delta ticks from previous event to next event
	ulong    wait_ticks;    // Wait ticks from previous event in other track
					// to next event in this track

	ulong	currentpos; // Some songs don't have a endoftrack event, so
			// we have to see when currentpos > size
	int	endoftrack;

	ulong	readVariableLengthValue(void);

	uchar lastcommand;  // This is to run light without overbyte :-)


	double	current_time; // in ms.
	double	time_at_previous_tempochange;  // in ms.
	double	ticks_from_previous_tempochange; // in ticks	
//	double	time_to_next_event;  // in ms.
	double	time_at_next_event;  // in ms.
	int	tPCN;
	ulong	tempo;

	int	power2to(int i);

public:
	track(FILE *file,int tpcn,int Id);
	~track();

	int	ticksPassed (ulong ticks); // returns 0 if OK, and 1 if 
					// you didn't handle this track well
					// and you forgot to take an event
					// (never ?)
	int	msPassed    (ulong ms);
	int	currentMs   (double ms);

	ulong	waitTicks   (void)
			{return wait_ticks; };
//	ulong   waitMs (void) {return time_to_next_event;};
	double   absMsOfNextEvent (void);
	void 	changeTempo(ulong t);


        void	readEvent(Midi_event *ev);
	void	init(void);

};

#endif
