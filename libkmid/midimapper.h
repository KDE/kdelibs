/**************************************************************************

    midimapper.h  - The midi mapper object
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
#ifndef _MIDIMAPPER_H
#define _MIDIMAPPER_H

#include <stdio.h>
#include "dattypes.h"

struct Keymap
{
	char name[30];
	uchar key[128];
	struct Keymap *next;
};

class MidiMapper
{
private:
	int	ok;
	uchar	channel[16]; 
	Keymap *channelKeymap[16]; // pointer to the keymap to use for a channel
				// this is to make it faster
				// The index is with the real channel (already mapped)
	int	channelPatchForced[16]; // if -1 the channel doesn't have
				// a forced patch, else indicates the patch to
				// force in a channel

	uchar	patchmap[128];
	Keymap *patchKeymap[128]; // Same as channelKeymap
   
	Keymap *keymaps; // Real linked list of keymaps used around the program


	char *	filename; // Stores the name of the file from which the map
			// was loaded

	int mapExpressionToVolumeEvents; // Simulate expression events with volume events
	int mapPitchBender;  // Use or not use the next variable
	int PitchBenderRatio; // Indicates the ratio between the standard and the Synth's Pitch Bender
			//	engine. The number sent to the synth is multiplied by this and dividied
			//	by 4096. Thus if PitchBenderRatio is 4096, the synth's pitch bender works as the standard

        void getValue(char *s,char *v);
        void removeSpaces(char *s);
        int  countWords(char *s);
        void getWord(char *t,char *s,int w); // get from s the word in position
                                             //  w and put it in t


        void DeallocateMaps(void);
        Keymap *createKeymap(char *name,uchar use_same_note=0,uchar note=0);
	void readPatchmap(FILE *fh);
	void readKeymap(FILE *fh,char *first_line);
	void readChannelmap(FILE *fh);
	void readOptions(FILE *fh);

        void AddKeymap(Keymap *newkm);
	Keymap *GiveMeKeymap(char *n);

public:
	MidiMapper(const char *name);
	~MidiMapper();


	void LoadFile(const char *name);	
	int  OK(void) {return ok;};


	uchar Channel(uchar chn) { return channel[chn];};
	uchar Patch(uchar chn,uchar pgm);
	uchar Key(uchar chn,uchar pgm, uchar note);
	void  PitchBender(uchar chn,uchar &lsb,uchar &msb);
        void  Controller(uchar chn,uchar &ctl,uchar &v);

	char *getFilename(void);

};
#endif
