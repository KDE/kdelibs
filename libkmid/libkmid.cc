/**************************************************************************
 
    libkmid.cc  - class KMidSimpleAPI that makes it easy to use libkmid
                        and a C wrapper.
    This file is part of LibKMid 0.9.5
    Copyright (C) 2000  Antonio Larrosa Jimenez
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

#include "libkmid.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>

#include "deviceman.h"
#include "player.h"
#include "midimapper.h"

struct kMidData kMid;

int KMidSimpleAPI::kMidInit(void)
{
  kMid.midi = new DeviceManager();
  if ( kMid.midi == 0L ) return 1;
  kMid.midi->initManager();
  if (!kMid.midi->ok()) return 1;

  kMid.pctlsmID=shmget(getpid(),sizeof(PlayerController),0600 | IPC_CREAT);
  if (kMid.pctlsmID==-1) return 1;
  kMid.pctl=(PlayerController *)shmat(kMid.pctlsmID,NULL,0);
  if (kMid.pctl==NULL) return 1;

  kMid.player=new MidiPlayer(kMid.midi,kMid.pctl);  
  if ( kMid.player == 0L ) 
  {
    delete kMid.midi;
    return 1;
  }

  kMid.player->setParseSong(false); 

  kMid.pctl->message=0;
  kMid.pctl->gm=1;
  kMid.pctl->error=0;
  kMid.pctl->ratioTempo=1.0;
  kMid.pctl->tempo=500000;     
  kMid.pctl->volumepercentage=100;    
  for (int i=0;i<16;i++)
  {
    kMid.pctl->forcepgm[i]=0;
    kMid.pctl->pgm[i]=0;
  } 

  return 0;
}

int KMidSimpleAPI::kMidLoad(const char *filename)
{
  if (kMidDevices()==0) return 0;
  return kMid.player->loadSong(filename);
}

int KMidSimpleAPI::kMidPlay(int loop)
{
  if (kMidDevices()==0) return 4;
  if (!kMid.player->isSongLoaded()) return 1; 
  if (kMid.pctl->playing==1) return 2;
  if (kMid.midi->checkInit()==-1) return 3;
  kMid.pctl->message=0;
  kMid.pctl->playing=0;
  kMid.pctl->finished=0;
  kMid.pctl->error=0;
  kMid.pctl->SPEVplayed=0;
  kMid.pctl->SPEVprocessed=0;
  kMid.pctl->millisecsPlayed=0;
  if ((kMid.pid=fork())==0)  
  {
    if (loop)
    {
      while (1)
      {
	kMid.player->play();
	if (kMid.pctl->error) return 5;
	kMid.pctl->message=0;
	kMid.pctl->playing=0;
	kMid.pctl->finished=0;
	kMid.pctl->error=0;
	kMid.pctl->SPEVplayed=0;
	kMid.pctl->SPEVprocessed=0;
	kMid.pctl->millisecsPlayed=0;
      }

    } else {
      kMid.player->play();
      if (kMid.pctl->error) return 5;
    }
    _exit(0);
  } else return 4;
  return 0;
}

int KMidSimpleAPI::kMidStop(void)
{
  if (kMidDevices()==0) return 4;
  if (kMid.pctl->playing==0) return 1;
  if (kMid.pid!=0)
  {
    kill(kMid.pid,SIGTERM);
    waitpid(kMid.pid, NULL, 0);
    kMid.pid=0;
  } else return 2;

  kMid.pctl->playing=0;   
  return 0;
}

void KMidSimpleAPI::kMidDestruct(void)
{
  delete kMid.midi;
  kMid.midi=0L;
  delete kMid.player;
  kMid.player=0L;
  delete kMid.map;
  shmdt((char *)kMid.pctl);
  shmctl(kMid.pctlsmID, IPC_RMID, 0L);  
}

int KMidSimpleAPI::kMidIsPlaying(void)
{
   return kMid.pctl->playing;
}

int KMidSimpleAPI::kMidDevices(void)
{
   return kMid.midi->midiPorts()+kMid.midi->synthDevices();
}

const char * KMidSimpleAPI::kMidName(int i)
{
   return kMid.midi->name(i);
}

const char * KMidSimpleAPI::kMidType(int i)
{
   return kMid.midi->type(i);
}

void KMidSimpleAPI::kMidSetDevice(int i)
{
   kMid.midi->setDefaultDevice(i);
}

void KMidSimpleAPI::kMidSetMidiMapper(const char *mapfilename)
{
  if (kMidDevices()==0) return;
   kMid.map=new MidiMapper(mapfilename);
   if ((kMid.map->ok() == 0L)||(!kMid.map->ok())) return;
   kMid.midi->setMidiMap(kMid.map);
}

const char *KMidSimpleAPI::kMidVersion(void)
{
   return "0.9.5";
}

const char *KMidSimpleAPI::kMidCopyright(void)
{
   return "LibKMid 0.9.5 (C)1997-2000 Antonio Larrosa Jimenez <larrosa@kde.org>.Malaga(es)";
}

/* * * * * * 
 
 Under this line (------) there's only a C wrapper for the KMidSimpleAPI class

* * * * * */


int kMidInit(void)
{
  return KMidSimpleAPI::kMidInit();
}

int kMidLoad(const char *filename)
{
  return KMidSimpleAPI::kMidLoad(filename);
}

int kMidPlay(void)
{
  return KMidSimpleAPI::kMidPlay();
}

int kMidStop(void)
{
  return KMidSimpleAPI::kMidStop();
}

void kMidDestruct(void)
{
  KMidSimpleAPI::kMidDestruct();
}

int kMidIsPlaying(void)
{
  return KMidSimpleAPI::kMidIsPlaying();
}

int kMidDevices(void)
{
  return KMidSimpleAPI::kMidDevices();
}

const char *kMidName(int i)
{
  return KMidSimpleAPI::kMidName(i);
}

const char *kMidType(int i)
{
  return KMidSimpleAPI::kMidType(i);
}

void kMidSetDevice(int i)
{
  KMidSimpleAPI::kMidSetDevice(i);
}

void kMidSetMidiMapper(const char *mapfilename)
{
  KMidSimpleAPI::kMidSetMidiMapper(mapfilename);
}

const char *kMidVersion(void)
{
  return KMidSimpleAPI::kMidVersion();
}

const char *kMidCopyright(void)
{
  return KMidSimpleAPI::kMidCopyright();
}

