/**************************************************************************

    player.h  - class player, which plays a set of tracks
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
#ifndef _PLAYER_H
#define _PLAYER_H

#include "dattypes.h"
#include "midfile.h"
#include "midiout.h"
#include "track.h"

struct SpecialEvent
{
	ulong	diffmilliseconds; //delta milliseconds from previous sp ev
	ulong   absmilliseconds; // milliseconds from beginning of song
        int     ticks; // ticks from begining of song
	int	type;// 1 Text, 2 Lyrics, 3 Change Tempo
		//     0 Nothing, end of linked list
	char	text[1024];
	int	tempo;   
	struct  SpecialEvent *next;
};


struct PlayerController
{
	volatile ulong	ticksTotal;
	volatile ulong	ticksPlayed;
	volatile double  millisecsPlayed;

	volatile int	tempo;

	volatile int	SPEVprocessed;
	volatile int	SPEVplayed;

	volatile int    OK; // when pause is released, the ui must know
			// when the player has opened the devices and it is
			// playing again then it put OK=1

	volatile int	playing; // if 1, the player is playing (or paused)
	volatile int	paused;  // if 1, the player is paused
	volatile int    moving;  // if 1, the player is moving the position
	volatile int	message; // set one of the following :

#define PLAYER_DOPAUSE	1
#define PLAYER_DOSTOP	2
#define PLAYER_SETPOS	4
#define PLAYER_HALT	8

	volatile int    error; //An error has ocurred(i.e. couldn't open device)
	volatile ulong	gotomsec;//milliseconds to go to,if player_setpos is set
	
	int		gm; // if 1 then song is GeneralMidi, if 0 then MT32

	volatile Midi_event	*ev;

};



class midiStat;

class player
{

midiOut *midi;
midifileinfo *info;
track **tracks;
SpecialEvent *spev;


int songLoaded;

void removeSong(void);

PlayerController *ctl;

void removeSpecialEvents(void);
void parseSpecialEvents(void);

public:


player(midiOut *midi_,PlayerController *pctl);
~player();

void loadSong(char *filename);

int isSongLoaded(void) {return songLoaded;};
SpecialEvent *takeSpecialEvents() {return spev;};
void writeSPEV(void);


void play(int calloutput,void output(void));


midifileinfo *Info(void) {return info;};

void SetPos(ulong gotomsec,midiStat *midistat);


/*
void pause(int i);
void stop(void);

void go_to(ulong ticks);
*/
};

#endif
