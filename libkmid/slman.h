/*  slman.h  - SongList Manager, which holds a set of collections (SongLists) 
    Copyright (C) 1997,98,99,2000  Antonio Larrosa Jimenez

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
#ifndef SLMAN_H
#define SLMAN_H

#include "songlist.h"

class SLManager
{
protected:
int ntotal;

struct SongListNode
   {
   int id;
   char *name;
   SongList *SL;
   
   SongListNode *next;
   };
SongListNode *list;
SongList *tempsl;

char *getNotUsedName(void);
int nameUsed(const char *name);
void regenerateid(SongListNode *sl,int id);

public:
SLManager();
SLManager(SLManager &src);  // Copy constructor
~SLManager();

int createCollection(const char *name=NULL); // Returns the id associated to the 
                           // new collection
                           // -1 if name already is used and Collection was
                           //  not created

void deleteCollection(int id);
void changeCollectionName(int id, const char *newname);

SongList *getCollection(int id);
SongList *getCollection(const char *name);
const char *getCollectionName(int id);

int numberOfCollections(void) {return ntotal;};

void loadConfig(const char *filename);
void saveConfig(const char *filename);

SongList *createTemporaryCollection(void);
SongList *getTemporaryCollection(void) {return tempsl;};

};

#endif
