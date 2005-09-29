/**************************************************************************

    voiceman.cc - The VoiceManager class handles a set of voices for synths
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

#include "voiceman.h"
#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

VoiceManager::VoiceManager(int totalvoices)
{
  nvoices=totalvoices;

  FirstVoice=new voice;
  FirstVoice->id=0;
  FirstVoice->channel=0;
  FirstVoice->note=0;
  FirstVoice->used=0;
  FirstVoice->prev=NULL;

  voice *ptrb=FirstVoice;
  voice *ptr=NULL;
  int i;
  for (i=1;i<nvoices;i++)
  {
    ptr=new voice;
    ptrb->next=ptr;
    ptr->id=i;
    ptr->channel=0;
    ptr->note=0;
    ptr->used=0;
    ptr->prev=ptrb;    
    ptrb=ptr;
  }
  LastVoice=ptr;
  LastVoice->next=NULL;
  LastnotusedVoice=LastVoice;

  VoiceList=new voice *[nvoices];
  ptr=FirstVoice;
  for (i=0;i<nvoices;i++)
  {
    VoiceList[i]=ptr;
    ptr=ptr->next; 
  }
  searcher_aid=new voice;
}

VoiceManager::~VoiceManager()
{
  voice *ptr=FirstVoice;
  voice *ptr2;
  while (ptr!=NULL)
  {
    ptr2=ptr->next;
    delete ptr;
    ptr=ptr2;
  }
  FirstVoice=NULL;
  LastVoice=NULL;
  LastnotusedVoice=NULL;

  if (VoiceList!=NULL) 
  {
    delete VoiceList;
    VoiceList=NULL;
  }

  delete searcher_aid;
}

void VoiceManager::clearLists(void)
{
#ifdef VOICEMANDEBUG
  printf("voicemanager::cleanLists\n");
#endif
  voice *ptr=FirstVoice;
  voice *ptr2=FirstVoice;
  while (ptr!=NULL)
  {
    ptr->used=0;
    ptr2=ptr;
    ptr=ptr->next;
  }
  LastVoice=ptr2;
  LastnotusedVoice=ptr2;

}

int VoiceManager::allocateVoice(int chn,int key)
{
  // First, we take the allocated voice out of the first place of the list
  if ((LastnotusedVoice!=NULL)&&(LastnotusedVoice->id==FirstVoice->id))
  {
#ifdef VOICEMANDEBUG
    printf("Used last voice !\n");
#endif
    LastnotusedVoice=NULL;
  }
  voice *newvoice=FirstVoice;
  FirstVoice=FirstVoice->next;
  FirstVoice->prev=NULL;

#ifdef VOICEMANDEBUG
  printf("Allocating id :%d\n",newvoice->id);
#endif
  // then we put the allocated voice at the end of the list
  LastVoice->next=newvoice;
  newvoice->prev=LastVoice;
  LastVoice=newvoice;
  LastVoice->next=NULL;

  newvoice->channel=chn;
  newvoice->note=key;

#ifdef VOICEMANDEBUG
  if (newvoice->used==1) 
  {
    printf("Replacing voice : %d\n",newvoice->id);
  }
#endif
  newvoice->used=1;

  //dispStat();
  return newvoice->id;
}

void VoiceManager::deallocateVoice(int id)
{
  voice *delvoice=VoiceList[id];
#ifdef VOICEMANDEBUG
  printf("Deallocating id :%d\n",id);
#endif
  if (delvoice->id==LastVoice->id)
  {
    LastVoice=delvoice->prev;
    LastVoice->next=NULL;

    if (LastnotusedVoice==NULL)
    {
      delvoice->next=FirstVoice;
      FirstVoice->prev=delvoice;
      FirstVoice=delvoice;
      FirstVoice->prev=NULL;
      LastnotusedVoice=FirstVoice;
    }
    else
    {
      if (LastnotusedVoice->next==NULL)
      {
	LastnotusedVoice->next=delvoice;
	delvoice->prev=LastnotusedVoice;
	delvoice->next=NULL;
	LastnotusedVoice=delvoice;
	LastVoice=delvoice;
      }
      else
      {
	delvoice->next=LastnotusedVoice->next;
	delvoice->next->prev=delvoice;
	delvoice->prev=LastnotusedVoice;
	LastnotusedVoice->next=delvoice;
	LastnotusedVoice=delvoice;
      }
    }
  }
  else
  {
    if (delvoice->prev!=NULL)
    {
      delvoice->prev->next=delvoice->next;
      delvoice->next->prev=delvoice->prev;
      if (LastnotusedVoice==NULL)
      {
	delvoice->next=FirstVoice;
	FirstVoice->prev=delvoice;
	FirstVoice=delvoice;
	FirstVoice->prev=NULL;
	LastnotusedVoice=FirstVoice;                                                    }
      else
      {
	if (LastnotusedVoice->next==NULL)
	{
	  LastnotusedVoice->next=delvoice;
	  delvoice->prev=LastnotusedVoice;
	  delvoice->next=NULL;
	  LastnotusedVoice=delvoice;
	  LastVoice=delvoice;
	}
	else
	{
	  delvoice->next=LastnotusedVoice->next;
	  delvoice->next->prev=delvoice;
	  delvoice->prev=LastnotusedVoice;
	  LastnotusedVoice->next=delvoice;
	  LastnotusedVoice=delvoice;
	}
      }
    }
  }
  delvoice->used=0;

  //  dispStat();
}

void VoiceManager::initSearch(void)
{
  searcher=searcher_aid;
  searcher_aid->prev=LastVoice;
}

int VoiceManager::search(int chn)
{
  if (searcher==NULL) return -1;
  searcher=searcher->prev;

  while (searcher!=NULL)
  {
    if (searcher->used==0) return -1;
    if (searcher->channel==chn) 
    {	
      return searcher->id;
    }
    searcher=searcher->prev;
  }
  return -1;
}

int VoiceManager::search(int chn,int note)
{
  if (searcher==NULL) return -1;
  searcher=searcher->prev;
  while ((searcher!=NULL))
  {
    if (searcher->used==0) return -1;
    if ((searcher->channel==chn)&&(searcher->note==note)) 
    {
      return searcher->id;
    }
    searcher=searcher->prev;
  }
  return -1;
}

/*
void VoiceManager::dispStat(void)
{
#ifdef VOICEMANDEBUG
  printf("Stats\n");
  voice *ptr=FirstVoice;
  while (ptr!=NULL)
  {
    printf("Voice %d is %s\n",ptr->id,(ptr->used==0)?("off"):("on"));
    ptr=ptr->next;
  }
  if (LastnotusedVoice!=NULL) printf("LnuV = %d\n",LastnotusedVoice->id);
#endif
}
*/
