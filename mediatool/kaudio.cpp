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
extern "C" {
#include <mediatool.h>
}
#include "kaudio.h"
#include "kaudio.moc"

#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif


#define maxFnameLen 256

KAudio::KAudio() : QObject()
{
  char		ServerId[256];
  char		KMServerCidFile[maxFnameLen];
  const char    kasFileName[]="/.kaudioserver";
  char		*tmpadr;
  FILE		*KMServerCidHandle;
  MediaCon	m;

  ServerContacted = false;
  WAVname         = 0L;
  autosync        = false;
  finishTimer     = 0;


  /*********************************************************************************
   * Read in audio player id (This is NOT a pid, but a communication connection id)
   *********************************************************************************/
  tmpadr= getenv("HOME");
  int homePathLen = strlen(tmpadr);

  if ( (homePathLen+strlen(kasFileName)+1 ) >= maxFnameLen ) {
    cerr <<  "HOME path too long.\n";
    return;
  }
  strcpy(KMServerCidFile,tmpadr);
  strcpy(KMServerCidFile+homePathLen,kasFileName);

  KMServerCidHandle = fopen(KMServerCidFile,"r");
  if (KMServerCidHandle == 0L)
    {
      cerr << "PID could not get read.\n";
      return;
    }

  fscanf(KMServerCidHandle,"%s\n",ServerId);
  fclose (KMServerCidHandle);



  /************* connect audio player ******************************/
  MdConnect(atoi(ServerId), &m);
  if ( m.shm_adr == 0L )
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

bool KAudio::play(QString& filename)
{
  if (!ServerContacted)
    return false;

  setFilename(filename);
  FileNameSet( FnamChunk, WAVname);
  return play();
}


bool KAudio::play(const char *filename)
{
  if (!ServerContacted)
    return false;

  setFilename(filename);
  FileNameSet( FnamChunk, WAVname);
  return play();
}

bool KAudio::setFilename(QString& filename)
{
  if (!ServerContacted)
    return false;
  if (WAVname  != 0L )
    ::free(WAVname);
  int fnlen= filename.length();
  WAVname = (char *)malloc(fnlen+1);
  strcpy(WAVname, (const char*)filename);

  return true;
}
bool KAudio::setFilename(const char *filename)
{
  if (!ServerContacted)
    return false;
  if (WAVname  != 0L )
    free(WAVname);

  char *myCopy = (char*)malloc(strlen(filename)+1);
  strcpy(myCopy,filename);
  WAVname = myCopy;

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

void KAudio::setSignals(bool sigs)
{
  if (sigs) {
    if (!finishTimer) {
      finishTimer = new QTimer(this);
      connect   ( finishTimer, SIGNAL(timeout()), this , SLOT(checkFinished()) );
      finishTimer->start(100);
      currentId = KeysChunk->sync_id;
    }
  }
  else {
    if (finishTimer) {
      disconnect( finishTimer );
      finishTimer->stop();
      delete finishTimer;
      finishTimer = 0;
    }
  }
}

void KAudio::checkFinished()
{
  if ( StatChunk->sync_id == currentId ) {
    // There is no point in emitting a signal now.
    // Either we played no sound after setSignals(true) or we signalled already
    return;
  }
  if ( StatChunk->sync_id == KeysChunk->sync_id ) {
    emit playFinished();
    // Copy the sync_id, so we see, that we have signalled the id already
    currentId = KeysChunk->sync_id;
  }
}
