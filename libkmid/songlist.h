/**************************************************************************

    songlist.h  - class SongList, which holds a list of songs (collection) 
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
#ifndef SONGLIST_H
#define SONGLIST_H

#include <stdio.h>

class SongList
{
protected:
int ntotal;

struct Song
{
   int id;
   char *name;  // complete path and file name

   Song *next;
};

Song *list;
Song *last;
Song *active;

Song *it; // Iterator, just a helper variable to make easy (and fast) reading
         // all the list

Song *getSongid(int id);

void regenerateid(Song *song,int id);

public:
SongList(void);
SongList(SongList &src); // Copy constructor
~SongList();

int AddSong(const char *song); // Returns the id number assigned to the song
void DelSong(int id);

int NumberOfSongs(void)     { return ntotal; };

void setActiveSong(int id);
int getActiveSongID(void)   {return ((active!=NULL)? (active->id  ):(-1)); };
char *getActiveSongName(void) 
	{
	return ((active!=NULL)? (active->name):((char *)NULL)); 
	};

char *getName(int id); // Returns the name of the song with id id

void previous(void);
void next(void) { if (active!=NULL) active=active->next;};

void iteratorStart(void);
void iteratorNext(void);
int iteratorAtEnd (void) {return (it==NULL);};
int getIteratorID(void);
char *getIteratorName(void);

void clean(void); // Clean this list
void copy(SongList &src); // Makes this object a copy of src (really copied)
};

#endif
