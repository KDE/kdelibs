/**************************************************************************

    songlist.cc  - class SongList, which holds a list of songs (collection)
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
#include "songlist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

SongList::SongList(void)
{
    list=NULL;
    last=NULL;
    active=NULL;
    ntotal=0;
}

SongList::SongList(SongList &src)
{
    list=last=active=NULL;
    ntotal=0;
    src.iteratorStart();
    while (!src.iteratorAtEnd())
    {
        AddSong(src.getIteratorName());
        src.iteratorNext();
    }
    if (src.active!=NULL) active=getSongid(src.active->id);
}

SongList::~SongList()
{
    clean();
    Song *ptr=list;
    active=last=NULL;
    ntotal=0;
    
    while (ptr!=NULL)
    {
        list=ptr->next;
        delete ptr->name;
        delete ptr;
        ptr=list;
    }
    
}

SongList::Song *SongList::getSongid(int id)
{
    Song *ptr=list;
    while ((ptr!=NULL)&&(ptr->id!=id))
        ptr=ptr->next;
    
    return ptr;
}

int SongList::AddSong(const char *song)
{
    if (last==NULL)
    {
        last=new Song;
        list=last;
    }
    else
    {
        last->next=new Song;
        last=last->next;
    }
    last->name=new char[strlen(song)+1];
    strcpy(last->name,song);
    last->id= ++ntotal;
    last->next=NULL;
    if (active==NULL) active=last;
    return last->id;
}

void SongList::DelSong(int id)
{
    Song *ptr;
    
    if (list==NULL) return;
    if (id==1)
    {
        if (last->id==1)
        {
            list=last=active=NULL;
            ntotal=0;
        }
        else
        {
            ptr=list;
            if (active->id==1) active=list->next;
            list=list->next;
            delete ptr->name;
            delete ptr;
            ntotal--;
            
            regenerateid(list,1);
        }
        return;
    }
    Song *ptr_prev=getSongid(id-1);
    ptr=ptr_prev->next;
    if (last->id==id) last=ptr_prev;
    if (active->id==id) 
        if (active->next!=NULL) active=active->next;
        else active=ptr_prev;
    
    ntotal--;
    ptr_prev->next=ptr->next;
    delete ptr->name;
    delete ptr;
    regenerateid(ptr_prev->next,id);
    
}

void SongList::regenerateid(Song *song,int id)
{
    Song *tmp=song;
    int i=id;
    while (tmp!=NULL)
    {
        tmp->id=i++;
        tmp=tmp->next;
    }
    ntotal=i-1;
}

void SongList::setActiveSong(int id)
{
    Song *tmp=getSongid(id);
    if (tmp!=NULL) active=tmp;
}

char *SongList::getName(int id)
{
    Song *tmp=getSongid(id);
    if (tmp!=NULL) return tmp->name;
    return NULL;
}

/*
void SongList::saveList(FILE *fh)
{
    Song *ptr=list;
    while (ptr!=NULL)
    {
        fputs(fh,ptr->name);
        ptr=ptr->next;
    }
}
*/

void SongList::iteratorStart(void)
{
    it=list;
}

void SongList::iteratorNext(void)
{
    if (it!=NULL) it=it->next;
}

int SongList::getIteratorID(void)
{
    if (it==NULL) return -1;
    return it->id;
}

char *SongList::getIteratorName(void)
{
    if (it==NULL) return NULL;
    return it->name;
}


void SongList::clean(void)
{
    Song *tmp=list;
    active=last=NULL;
    ntotal=0;
    
    while (tmp!=NULL)
    {
        list=tmp->next;
        delete tmp->name;
        delete tmp;
        tmp=list;
    }
}

void SongList::copy(SongList &src)
{
    clean();
    src.iteratorStart();
    while (!src.iteratorAtEnd())
    {
        AddSong(src.getIteratorName());
        src.iteratorNext();
    }
    if (src.active!=NULL) active=getSongid(src.active->id);
}

int SongList::next(void)
{
    if (list==NULL) {active=NULL;return 0;};
    if (active!=NULL) active=active->next;
    if (active==NULL) 
    {
        Song *tmp=list;
        while (tmp->next!=NULL) tmp=tmp->next;
        active=tmp;
        return 0;
    }
    return 1;
}


void SongList::previous(void)
{
    if (list==NULL) {active=NULL;return;};
    Song *tmp=list;
    while ((tmp->next!=NULL)&&(tmp->next->id!=active->id)) tmp=tmp->next;
    if (tmp->next==NULL) {active=list;return;};
    active=tmp;
}
