/**************************************************************************

    slman.cc  - SongList Manager, which holds a set of collections (SongLists)
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
#include "slman.h"
#include <stdlib.h>
#include <string.h>
#include "../version.h"

SLManager::SLManager()
{
    list=NULL;
    ntotal=0;
    tempsl=NULL;
    createTemporaryCollection();
}

SLManager::SLManager(SLManager &src)
{
    list=NULL;
    ntotal=0;
    tempsl=NULL;
    SongListNode *srcSL=src.list;
    SongList *tmpSL;
    int i;
    while (srcSL!=NULL)
    {
        i=createCollection(srcSL->name);  
        tmpSL=getCollection(i);
        if (tmpSL!=NULL)
            tmpSL->copy(*srcSL->SL);
        srcSL=srcSL->next;
    }
    if (src.tempsl!=NULL) tempsl=new SongList(*src.tempsl);
    else tempsl=NULL;
}

SLManager::~SLManager()
{
    SongListNode *ptr;
    ptr=list;
    while (ptr!=NULL)
    {
        list=ptr->next;
        if (ptr->SL!=NULL) delete ptr->SL;
        if (ptr->name!=NULL) delete ptr->name;
        delete ptr;
        ptr=list;
    }
    if (tempsl!=NULL) delete tempsl;
    ntotal=0;
}

int SLManager::createCollection(const char *name)
{
    SongListNode *ptr;
    if (nameUsed(name))
    {
        printf("Name '%s' is already used\n",name);
        return -1;
    }
    if (list==NULL)
    {
        list=new SongListNode;
        list->id=1;
        ntotal=1;
        ptr=list;
    }
    else
    {
        ptr=list;
        while (ptr->next!=NULL) ptr=ptr->next;
        ptr->next=new SongListNode;
        ptr=ptr->next;
        
        ptr->id= ++ntotal;
    }
    ptr->SL=new SongList;
    ptr->next=NULL;   
    if (name!=NULL)
    {
        ptr->name=new char[strlen(name)+1];
        strcpy(ptr->name,name);
    }
    else
   {
   ptr->name=getNotUsedName();
   }
return ptr->id;
}

char *SLManager::getNotUsedName(void)
{
    char *trythis;
    trythis=new char[100];
    strcpy(trythis,"No Name");
    int tries=1;
    int success=0;
    while (!success)
    {
        if (nameUsed(trythis))  sprintf(trythis,"No Name - %d",++tries);
        else
            success=1;
    }
    return trythis;
}

int SLManager::nameUsed(const char *name)
{
    /*
    SongListNode *ptr=list;
    int used=0;
    while ((!used)&&(ptr!=NULL))
    {
        if (strcmp(ptr->name,name)==0) used=1;
        ptr=ptr->next;
    };
    return used;
    */
    if (getCollection(name)==NULL) return 0;
    return 1;
}

void SLManager::deleteCollection(int id)
{
    if (list==NULL) return;
    SongListNode *ptr=list;
    SongListNode *ptr2;
    if (id==1) list=list->next;
    else
    {
        ptr2=list;
        while ((ptr!=NULL)&&(ptr->id!=id)) 
        {
            ptr2=ptr;
            ptr=ptr->next;
        }
        if (ptr==NULL) 
        {
            printf("Trying to delete a not used id\n");
            return;
        }
        ptr2->next=ptr->next;
    }
    ptr2=ptr->next;
    delete ptr->SL;
    delete ptr->name;
    delete ptr;
    regenerateid(ptr2,id);
    
}

void SLManager::regenerateid(SongListNode *sl,int id)
{
    SongListNode *tmp=sl;
    int i=id;
    while (tmp!=NULL)
    {
        tmp->id=i++;
        tmp=tmp->next;
    }
    ntotal=i-1;
}

void SLManager::changeCollectionName(int id,const char *newname)
{
    if (id<1) return;
    if (nameUsed(newname))
    {
        printf("Cannot change name, '%s' is already used\n",newname);
        return;
    }
    SongListNode *ptr=list;
    while ((ptr!=NULL)&&(ptr->id!=id)) ptr=ptr->next;
    if (ptr==NULL) return;
    
    delete ptr->name;
    ptr->name=new char[strlen(newname)+1];
    strcpy(ptr->name,newname);
}

SongList *SLManager::getCollection(int id)
{
    if (id==0) return tempsl;
    
    SongListNode *ptr=list;
    while ((ptr!=NULL)&&(ptr->id!=id)) ptr=ptr->next;
    
    if (ptr==NULL) return NULL;
    return ptr->SL;
}

SongList *SLManager::getCollection(const char *name)
{
    SongListNode *ptr=list;
    while ((ptr!=NULL)&&(strcmp(ptr->name,name)!=0)) ptr=ptr->next;
    
    if (ptr==NULL) return NULL;
    return ptr->SL;
}

const char *SLManager::getCollectionName(int id)
{
    if (id==0) return "Temporary Collection";
    SongListNode *ptr=list;
    while ((ptr!=NULL)&&(ptr->id!=id)) ptr=ptr->next;
    
    if (ptr==NULL) return NULL;
    return ptr->name;
}

void SLManager::loadConfig(const char *filename)
{
#ifdef GENERAL_DEBUG_MESSAGES
    printf("Loading collections\n");
#endif
    FILE *fh=fopen(filename,"rt");
    if (fh==NULL)
    {
        printf("Collections cannot be loaded\n(File %s doesn't exist or can't be opened)\n",filename);
        return;
    }
    char s[300];
    SongList *sl=NULL;
    int activeid=0;
    while (!feof(fh))
    {
        fgets(s,299,fh);
        if ((strlen(s)>0)&&(s[strlen(s)-1]==10)) s[strlen(s)-1]=0;
        switch (s[0])
        {
        case (0)   : break;
        case (10)  : break;
        case ('=') : 
            {
                if (sl!=NULL) sl->setActiveSong(activeid);
                int id=createCollection(&s[1]);
                sl=getCollection(id);
                fgets(s,299,fh);
                activeid=atoi(s);
            }
            break;
        default : 
            {
            if (sl!=NULL) sl->AddSong((const char *)s);
            }
        }
    }
    if (sl!=NULL) sl->setActiveSong(activeid);
    
    fclose(fh);
}

void SLManager::saveConfig(const char *filename)
{
    SongListNode *ptr=list;
    FILE *fh=fopen(filename,"wt");
    if (fh==NULL)
    {
        printf("Collections couldn't be saved\n");
        return;
    }
    char s[300];
    SongList *sl;
    while (ptr!=NULL)
    {
        sprintf(s,"=%s\n",ptr->name);
        fputs(s,fh);
        
        sl=ptr->SL;
        sprintf(s,"%d\n",sl->getActiveSongID());   
        fputs(s,fh);
        
        sl->iteratorStart();
        while (!sl->iteratorAtEnd())
        {
            sprintf(s,"%s\n",sl->getIteratorName());
            fputs(s,fh);
            sl->iteratorNext();
        }
        //   ptr->SL->saveList(fh);
        fputs("\n",fh);
        
        ptr=ptr->next;
    }
    fclose(fh);
    
}

SongList *SLManager::createTemporaryCollection(void)
{
    if (tempsl==NULL)
        tempsl=new SongList();
    else
        tempsl->clean();
    
    return tempsl;
}
