/**************************************************************************

    voiceman.h - The voiceManager class handles a set of voices for synths
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
#ifndef _VOICEMAN_H
#define _VOICEMAN_H

class voiceManager
{
private:
    int nvoices;

    struct voice
	{
        int     id;
	int	channel;
	int	note;
	int	used;
	
	struct voice *prev;
	struct voice *next;
	};
   voice *FirstVoice; // Points to the beginning of the voice list, that is, to
			// the older voice which is ready to be used
   voice *LastVoice; // Points to the last voice, I mean, the latest used
			// voice

   voice *LastnotusedVoice; // Points to the latest (list order) not used voice
			//, that is, to where deallocated voices will be moved

   voice **VoiceList; // Array with pointer to the voices, arranged by id for
			// faster search

   voice *searcher; // Variable used to search channels

   voice *searcher_aid; // An auxiliary variable for simpler searches
public:
   voiceManager(int totalvoices);
   ~voiceManager();

   int allocateVoice(int chn,int key);
   void deallocateVoice(int id);

   void initSearch(void); // call this function always before Search !!!
   int Search(int chn); //returns -1 if channel chn is not currently used
			// Continue searching for more voices which
			// use the channel chn
   int Search(int chn,int note); // The same but also looks for a note


   int Channel(int v) {return VoiceList[v]->channel;};
   int Note(int v) {return VoiceList[v]->note;};
   int Used(int v) {return VoiceList[v]->used;};

   void dispStat(void);

   void cleanLists(void);
};

#endif
