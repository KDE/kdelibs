/* This file is part of the KDE libraries
    Copyright (C) 1997 Christian Esken (esken@kde.org)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <unistd.h>
#include <string.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <kmisc.h>
extern "C" {
#include <mediatool.h>
}
#include "kaudio.h"
#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif


/******************************************************************************
 *
 * Function:	mystrdup()
 *
 * Task:	Duplicate a string, using freshly allocated memory.
 *		See "man strdup" for more information
 *
 * in:		s	Adress of string.
 * 
 * out:		char*	Adress of new created/copied string.
 *
 * Comment:	This is a drop-in replacement function for strdup().
 *		As the mentioned function is not POSIX, this is necessary
 *		for portabilty.
 *
 *****************************************************************************/
char *mystrdup(char *s)
{
  char *tmp = (char*)malloc(strlen(s)+1);
  if (tmp)
    strcpy(tmp,s);
  return tmp;
}

KAudio::KAudio()
{
  char		ServerId[256];
  char		KMServerCidFile[256];
  char		*tmpadr;
  FILE		*KMServerCidHandle;
  MediaCon	m;

  ServerContacted = false;
  WAVname         = NULL;
  autosync        = false;

  /*********************************************************************************
   * Read in audio player id (This is NOT a pid, but a communication connection id)
   *********************************************************************************/
  tmpadr= getenv("HOME");
  strcpy(KMServerCidFile,tmpadr);
  strcpy(KMServerCidFile+strlen(KMServerCidFile),"/.kaudioserver");
  KMServerCidHandle = fopen(KMServerCidFile,"r");
  if (KMServerCidHandle == NULL)
    {
      cerr << "PID could not get read.\n";
      return;
    }

  fscanf(KMServerCidHandle,"%s\n",ServerId);
  fclose (KMServerCidHandle);



  /************* connect audio player ******************************/
  MdConnect(atoi(ServerId), &m);
  if ( m.shm_adr == NULL )
    {
      cerr << "Could not find media master.\n";
      return;
    }
  /************* query for chunk adresses **************************/
  FnamChunk = (MdCh_FNAM*)FindChunkData(m.shm_adr, "FNAM");
  if (!FnamChunk)
    {
      cerr << "No FNAM chunk.\n";
      return;
    }
  IhdrChunk = (MdCh_IHDR*)FindChunkData(m.shm_adr, "IHDR");
  if (!IhdrChunk)
    {
      cerr << "No IHDR chunk.\n";
      return;
    }
  KeysChunk = (MdCh_KEYS*)FindChunkData(m.shm_adr, "KEYS");
  if (!KeysChunk)
    {
      cerr << "No KEYS chunk.\n";
      return;
    }
  StatChunk = (MdCh_STAT*)FindChunkData(m.shm_adr, "STAT");
  if (!StatChunk)
    {
      cerr << "No STAT chunk.\n";
      return;
    }


  MdConnectInit();

  ServerContacted = true;
}




bool KAudio::play()
{
  if (!ServerContacted)
    return false;

  EventCounterRaise( &(KeysChunk->play) ,1);
  KeysChunk->sync_id += 3; // sync helper (esp. for maudio)
  if (autosync)
    sync();
  return true;
}

bool KAudio::play(char *filename)
{
  if (!ServerContacted)
    return false;

  setFilename(filename);
  FileNameSet( FnamChunk, WAVname);
  return play();
}

bool KAudio::setFilename(char *filename)
{
  if (!ServerContacted)
    return false;
  if (WAVname  != NULL)
    free(WAVname);
  WAVname = mystrdup(filename);

  return true;
}

bool KAudio::stop()
{
  if (!ServerContacted)
    return false;

  EventCounterRaise(&(KeysChunk->stop) ,1);
  return true;
}

void KAudio::setAutosync(bool autosync)
{
  this->autosync = autosync;
}

void KAudio::sync()
{
  while ( StatChunk->sync_id != KeysChunk->sync_id )
    usleep(10*1000);
}

int KAudio::serverStatus()
{
  return !ServerContacted;
}
