/**************************************************************************

    player.cc  - class MidiPlayer. Plays a set of tracks
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Id$

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/
#include "player.h"
#include "sndcard.h"
#include "midispec.h"
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "midistat.h"
#include "mt32togm.h"

//#define PLAYERDEBUG
//#define GENERAL_DEBUG_MESSAGES

#define T2MS(ticks) (((double)ticks)*(double)60000L)/((double)tempoToMetronomeTempo(tempo)*(double)info->ticksPerCuarterNote)

#define MS2T(ms) (((ms)*(double)tempoToMetronomeTempo(tempo)*(double)info->ticksPerCuarterNote)/((double)60000L))

#define REMOVEDUPSTRINGS

MidiPlayer::MidiPlayer(DeviceManager *midi_,PlayerController *pctl)
{
  midi=midi_;
  info=NULL;
  tracks=NULL;
  songLoaded=0;
  ctl=pctl;
  spev=NULL;
  na=NULL;
  parsesong=true;
  generatebeats=false;
}

MidiPlayer::~MidiPlayer()
{
  removeSpecialEvents();
  removeSong();
}

void MidiPlayer::removeSong(void)
{
  if ((songLoaded)&&(tracks!=NULL))
  {
#ifdef PLAYERDEBUG
    printf("Removing song from memory\n");
#endif
    int i=0;
    while (i<info->ntracks)
    {
      if (tracks[i]!=NULL) delete tracks[i];
      i++;
    }
    delete tracks;
    tracks=NULL;
    if (info!=NULL)
    {
      delete info;
      info=NULL;
    }
  }
  songLoaded=0;
}

int MidiPlayer::loadSong(const char *filename)
{
  removeSong();
#ifdef PLAYERDEBUG
  printf("Loading Song : %s\n",filename);
#endif
  info=new MidiFileInfo;
  int ok;
  tracks=readMidiFile(filename,info,ok);
  if (ok<0) return ok;
  if (tracks==NULL) return -4;

  parseInfoData(info,tracks,ctl->ratioTempo);

  if (parsesong)
  {
    parseSpecialEvents();
    if (generatebeats) generateBeats();
  }

  songLoaded=1;
  return 0;
}

void MidiPlayer::insertBeat(SpecialEvent *ev,ulong ms,int num,int den)
{
  SpecialEvent *beat=new SpecialEvent;
  beat->next=ev->next;
  ev->next=beat;
  beat->id=1;
  beat->type=7;
  beat->absmilliseconds=ms;
  beat->num=num;
  beat->den=den;
}


void MidiPlayer::generateBeats(void)
{
#ifdef PLAYERDEBUG
  printf("player::Generating Beats...\n");
#endif

  if (spev==NULL) return;
  SpecialEvent *ev=spev;
  SpecialEvent *nextev=ev->next;
  ulong tempo=(ulong)(500000 * ctl->ratioTempo);
  int i=1;
  int num=4;
  int den=4;
  //    ulong beatstep=((double)tempo*4/(den*1000));
  //    ulong beatstep=T2MS(info->ticksPerCuarterNote*(4/den));
  double ticksleft=(((double)info->ticksPerCuarterNote*4)/den);

  double beatstep=T2MS(ticksleft);
  double nextbeatms=0;
  double lastbeatms=0;
  double measurems=0;

  while (nextev!=NULL)
  {
    switch (ev->type)
    {
      case (0): // End of list
	{
	};break;
      case (1): // Text
      case (2): // Lyrics
	{
	};break;
      case (3): // Change Tempo
	{
	  lastbeatms=ev->absmilliseconds;
	  ticksleft=MS2T(nextbeatms-lastbeatms);
	  tempo=ev->tempo;
	  nextbeatms=lastbeatms+T2MS(ticksleft);
	  // printf("Change at %lu to %d\n",ev->absmilliseconds,ev->tempo);
	  // beatstep=((double)tempo*4/(den*1000));
	  beatstep=T2MS(((static_cast<double>(info->ticksPerCuarterNote)*4)/den));
	};break;
      case (6): // Change number of beats per measure
	{
	  num=ev->num;
	  i=1;
	  den=ev->den;
	  // printf("Change at %lu to %d/%d\n",ev->absmilliseconds,num,den);
	  // beatstep=((double)tempo*4/(den*1000));
	  // beatstep=T2MS(info->ticksPerCuarterNote*(4/den));
	  beatstep=T2MS((((double)info->ticksPerCuarterNote*4)/den));
	  nextbeatms=ev->absmilliseconds;
	};break;
    };
    if (nextev->absmilliseconds>nextbeatms)
    {
      //printf("Adding %d,%d\n",num,tot);
      //printf("beat at %g , %d/%d\n",nextbeatms,i,num);
      //printf("  %ld %d\n",nextev->absmilliseconds,nextev->type);
      if (i == 1) {
          measurems=nextbeatms;
      }
      insertBeat(ev, static_cast<unsigned long>(nextbeatms), i++, num);
      if (i > num) {
          i=1;
      }
      lastbeatms=nextbeatms;
      nextbeatms+=beatstep;
      // nextbeatms=measurems+beatstep*i;

      ticksleft = ( (static_cast<double>(info->ticksPerCuarterNote)*4) / den);

    }

    ev=ev->next;
    nextev=ev->next;
  }

  /* ev==NULL doesn't indicate the end of the song, so continue generating beats */

  if (ev!=NULL)
  {
    if (ev->type==0)
    {
      ev=spev;
      /* Looking if ev->next is NULL is not needed because
	 we are sure that a ev->type == 0 exists, we just have
	 to assure that the first spev is not the only one */
      if (ev->next!=NULL)
	while (ev->next->type!=0) ev=ev->next;
    }
    while (nextbeatms<info->millisecsTotal)
    {
      //            printf("beat2 at %g , %d/%d\n",nextbeatms,i,num);
      if (i==1) measurems=nextbeatms;
      insertBeat(ev, static_cast<unsigned long>(nextbeatms), i++, num);
      if (i>num) i=1;
      nextbeatms+=beatstep;
      ev=ev->next;
    }
  }

  /* Regenerate IDs */

  ev=spev;
  i=1;
  while (ev!=NULL)
  {
    ev->id=i++;
    ev=ev->next;
  }


#ifdef PLAYERDEBUG
  printf("player::Beats Generated\n");
#endif

}

void MidiPlayer::removeSpecialEvents(void)
{
  SpecialEvent * ev=spev;
  while (spev!=NULL)
  {
    ev=spev->next;
    delete spev;
    spev=ev;
  }
  delete na;
  na=0;
}

void MidiPlayer::parseSpecialEvents(void)
{
#ifdef PLAYERDEBUG
  printf("player::Parsing...\n");
#endif
  removeSpecialEvents();
  spev=new SpecialEvent;
  if (spev==NULL) return;
  SpecialEvent *pspev=spev;
  pspev->type=0;
  pspev->ticks=0;
  if (na) delete na;
  na=new NoteArray();
  if (!na) { delete spev; spev=0L; return; };
  int trk;
  int minTrk;
  double minTime=0;
  double maxTime;
  ulong tempo=(ulong)(500000 * (ctl->ratioTempo));
  ulong firsttempo=0;
  for (int i=0;i<info->ntracks;i++)
  {
    tracks[i]->init();
    tracks[i]->changeTempo(tempo);
  }
  MidiEvent *ev=new MidiEvent;
  //ulong mspass;
  double prevms=0;
  int spev_id=1;
  int j;
  int parsing=1;
#ifdef REMOVEDUPSTRINGS
  char lasttext[1024];
  ulong lasttexttime=0;
  lasttext[0]=0;
  int lasttexttype=0;
#endif
  while (parsing)
  {
    prevms=minTime;
    trk=0;
    minTrk=0;
    maxTime=minTime + 2 * 60000L;
    minTime=maxTime;
    parsing=0;
    while (trk<info->ntracks)
    {
      if (tracks[trk]->absMsOfNextEvent()<minTime)
      {
        minTrk=trk;
        minTime=tracks[minTrk]->absMsOfNextEvent();
        parsing=1;
      }
      trk++;
    }
    //  if ((minTime==maxTime))
    if (parsing==0)
    {
      // parsing=0;
#ifdef PLAYERDEBUG
      printf("END of parsing\n");
#endif
    }
    else
    {
      // mspass=(ulong)(minTime-prevms);
      trk=0;
      while (trk<info->ntracks)
      {
        tracks[trk]->currentMs(minTime);
        trk++;
      }     
    }
    trk=minTrk;
    tracks[trk]->readEvent(ev);
    switch (ev->command)
    {
      case (MIDI_NOTEON) :
        if (ev->vel==0) na->add((ulong)minTime,ev->chn,0, ev->note);
        else na->add((ulong)minTime,ev->chn,1,ev->note);
        break;
      case (MIDI_NOTEOFF) :
        na->add((ulong)minTime,ev->chn,0, ev->note);
        break;
      case (MIDI_PGM_CHANGE) :
        na->add((ulong)minTime,ev->chn, 2,ev->patch);
        break;
      case (MIDI_SYSTEM_PREFIX) :
        {
          if ((ev->command|ev->chn)==META_EVENT)
          {
            switch (ev->d1)
            {
              case (1) :
              case (5) :
                {
                  if (pspev!=NULL)
                  {
                    pspev->absmilliseconds=(ulong)minTime;
                    pspev->type=ev->d1;
                    pspev->id=spev_id++;
#ifdef PLAYERDEBUG
                    printf("ev->length %ld\n",ev->length);
 
#endif
                    strncpy(pspev->text,(char *)ev->data,
                        (ev->length>1024)? (1023) : (ev->length) );
                    pspev->text[(ev->length>1024)? (1023):(ev->length)]=0;
#ifdef PLAYERDEBUG
                    printf("(%s)(%s)\n",pspev->text,lasttext);
#endif
#ifdef REMOVEDUPSTRINGS
                    if ((strcmp(pspev->text,lasttext)!=0)||(pspev->absmilliseconds!=lasttexttime)||(pspev->type!=lasttexttype))
                    {
                      lasttexttime=pspev->absmilliseconds;
                      lasttexttype=pspev->type;
                      strcpy(lasttext,pspev->text);
#endif
                      pspev->next=new SpecialEvent;
#ifdef PLAYERDEBUG
                      if (pspev->next==NULL) printf("pspev->next=NULL\n");
#endif
                      pspev=pspev->next;
#ifdef REMOVEDUPSTRINGS
                    }
#endif
                  }
                }
                break;
              case (ME_SET_TEMPO) :
                {
                  if (pspev!=NULL)
                  {
                    pspev->absmilliseconds=(ulong)minTime;
                    pspev->type=3;
                    pspev->id=spev_id++;
                    tempo=(ulong)(((ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2])) * ctl->ratioTempo);
                    pspev->tempo=tempo;
                    if (firsttempo==0) firsttempo=tempo;
                    for (j=0;j<info->ntracks;j++)
                    {
                      tracks[j]->changeTempo(tempo);
                    }
                    pspev->next=new SpecialEvent;
                    pspev=pspev->next;
                  }
                }
                break;
              case (ME_TIME_SIGNATURE) :
                {
                  if (pspev!=NULL)
                  {
                    pspev->absmilliseconds=(ulong)minTime;
                    pspev->type=6;
                    pspev->id=spev_id++;
                    pspev->num=ev->d2;
                    pspev->den=ev->d3;
                    pspev->next=new SpecialEvent;
                    pspev=pspev->next;
                  }
                }
                break;
            }
          }
        }
        break;
    }
  }
 
  delete ev;
  pspev->type=0;
  pspev->absmilliseconds=(ulong)prevms;
  pspev->next=NULL;
  if (firsttempo==0) firsttempo=tempo;
  ctl->tempo=firsttempo;
 
  //writeSPEV();
  for (int i=0;i<info->ntracks;i++)
  {
    tracks[i]->init();
  }
}

/*
NoteArray *MidiPlayer::parseNotes(void)
{
#ifdef PLAYERDEBUG
  printf("player::Parsing Notes...\n");
#endif
  NoteArray *na=new NoteArray();
  int trk;
  int minTrk;
  double minTime=0;
  double maxTime;
  for (int i=0;i<info->ntracks;i++)
  {
    tracks[i]->init();
  }; 
  ulong tempo=1000000;
  ulong tmp;
  Midi_event *ev=new Midi_event;
  //ulong mspass;
  double prevms=0;
  int j;
  int parsing=1;
  while (parsing)
  {
    prevms=minTime;
    trk=0;
    minTrk=0;
    maxTime=minTime + 2 * 60000L;
    minTime=maxTime;
    while (trk<info->ntracks)
    {
      if (tracks[trk]->absMsOfNextEvent()<minTime)
      {
	minTrk=trk;
	minTime=tracks[minTrk]->absMsOfNextEvent();
      };
      trk++;
    };
    if ((minTime==maxTime))
    {
      parsing=0;
#ifdef PLAYERDEBUG
      printf("END of parsing\n");
#endif
    }
    else
    {	
      //	mspass=(ulong)(minTime-prevms);
      trk=0;
      while (trk<info->ntracks)
      {
	tracks[trk]->currentMs(minTime);
	trk++;
      };
    };
    trk=minTrk;
    tracks[trk]->readEvent(ev);
    if (ev->command==MIDI_NOTEON)
    {
      if (ev->vel==0) {printf("note off at %g\n",minTime);na->add((ulong)minTime,ev->chn,0, ev->note);}
      else {printf("note on at %g\n",minTime);na->add((ulong)minTime,ev->chn,1,ev->note);}
    }
    else
      if (ev->command==MIDI_NOTEOFF) na->add((ulong)minTime,ev->chn,0, ev->note);
    if (ev->command==MIDI_PGM_CHANGE) na->add((ulong)minTime,ev->chn, 2,ev->patch);
    if (ev->command==MIDI_SYSTEM_PREFIX)
    {
      if (((ev->command|ev->chn)==META_EVENT)&&(ev->d1==ME_SET_TEMPO))
      {
	tempo=(ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2]);
	for (j=0;j<info->ntracks;j++)
	{
	  tracks[j]->changeTempo(tempo);
	};
      };
    };

  };

  delete ev;
  for (int i=0;i<info->ntracks;i++)
  {
    tracks[i]->init();
  };
  return na;
};
*/

void MidiPlayer::play(bool calloutput,void output(void))
{		
#ifdef PLAYERDEBUG
  printf("Playing...\n");
#endif
  
  if (midi->midiPorts()+midi->synthDevices()==0) 
  {
    fprintf(stderr,"Player :: There are no midi ports !\n");
    ctl->error=1;
    return;
  }

  midi->openDev();
  if (midi->ok()==0) 
  {
    fprintf(stderr,"Player :: Couldn't play !\n");
    ctl->error=1;
    return;
  }
  midi->setVolumePercentage(ctl->volumepercentage);
  midi->initDev();
  //    parsePatchesUsed(tracks,info,ctl->gm);
  midi->setPatchesToUse(info->patchesUsed);

  int trk;
  int minTrk;
  double minTime=0;
  double maxTime;
  int i;
  ulong tempo=(ulong)(500000 * ctl->ratioTempo);
  for (i=0;i<info->ntracks;i++)
  {
    tracks[i]->init();
    tracks[i]->changeTempo(tempo);
  }

  midi->tmrStart(info->ticksPerCuarterNote);
  MidiEvent *ev=new MidiEvent;
  ctl->ev=ev;
  ctl->ticksTotal=info->ticksTotal;
  ctl->ticksPlayed=0;
  //ctl->millisecsPlayed=0;
  ulong ticksplayed=0;
  double absTimeAtChangeTempo=0;
  double absTime=0;
  double diffTime=0;
  MidiStatus *midistat;
  //ulong mspass;
  double prevms=0;
  int j;
  int halt=0;
  ctl->tempo=tempo;
  ctl->num=4;
  ctl->den=4;
  int playing;
  ctl->paused=0;
  if ((ctl->message!=0)&&(ctl->message & PLAYER_SETPOS))
  {
    ctl->moving=1;
    ctl->message&=~PLAYER_SETPOS;
    midi->sync(1);
    midi->tmrStop();
    midi->closeDev();
    midistat = new MidiStatus();
    setPos(ctl->gotomsec,midistat);
    minTime=ctl->gotomsec;
    prevms=(ulong)minTime;
    midi->openDev();
    midi->tmrStart(info->ticksPerCuarterNote);
    diffTime=ctl->gotomsec;
    midistat->sendData(midi,ctl->gm);
    delete midistat;
    midi->setPatchesToUse(info->patchesUsed);
    ctl->moving=0;
  } else
    for (i=0;i<16;i++)
    {
      if (ctl->forcepgm[i])
      {
	midi->chnPatchChange(i, ctl->pgm[i]);
      }
    }

  timeval begintv;
  gettimeofday(&begintv, NULL);
  ctl->beginmillisec=begintv.tv_sec*1000+begintv.tv_usec/1000;
  ctl->OK=1;
  ctl->playing=playing=1;

  while (playing)
    {
      /*
      if (ctl->message!=0)
      {
	if (ctl->message & PLAYER_DOPAUSE)
	{
	  diffTime=minTime;
	  ctl->message&=~PLAYER_DOPAUSE;
	  midi->sync(1);
	  midi->tmrStop();
	  ctl->paused=1; 
	  midi->closeDev();
	  while ((ctl->paused)&&(!(ctl->message&PLAYER_DOSTOP))
	      &&(!(ctl->message&PLAYER_HALT))) sleep(1);
	  midi->openDev();
	  midi->tmrStart();
	  ctl->OK=1;
	  printf("Continue playing ... \n");
	};
	if (ctl->message & PLAYER_DOSTOP)
	{
	  ctl->message&=~PLAYER_DOSTOP;
	  playing=0;
	};
	if (ctl->message & PLAYER_HALT)
	{
	  ctl->message&=~PLAYER_HALT;
	  playing=0;
	  halt=1;
	};
	if (ctl->message & PLAYER_SETPOS)
	{
	  ctl->moving=1;
	  ctl->message&=~PLAYER_SETPOS;
	  midi->sync(1);
	  midi->tmrStop();
	  midi->closeDev();
	  midistat = new midiStat();
	  SetPos(ctl->gotomsec,midistat);
	  minTime=ctl->gotomsec;
	  prevms=(ulong)minTime;
	  midi->openDev();
	  midi->tmrStart();
	  diffTime=ctl->gotomsec;
	  ctl->moving=0;
	  midistat->sendData(midi,ctl->gm);
	  delete midistat;
	  ctl->OK=1;
	  while (ctl->OK==1) ;
	  ctl->moving=0;
	};
      };
      */
	prevms=minTime;
      //    ctl->millisecsPlayed=minTime;
      trk=0;
      minTrk=0;
      maxTime=minTime + 120000L /* milliseconds */;
	minTime=maxTime;
      playing=0;
      while (trk<info->ntracks)
      {
	if (tracks[trk]->absMsOfNextEvent()<minTime)
	{
	  minTrk=trk;
	  minTime=tracks[minTrk]->absMsOfNextEvent();
	  playing=1;
	}
	trk++;
      }
#ifdef PLAYERDEBUG
      printf("minTime %g\n",minTime);
#endif
      // if ((minTime==maxTime)/* || (minTicks> 60000L)*/)
      if (playing==0)
      {
	// playing=0;
#ifdef PLAYERDEBUG
	printf("END of playing\n");
#endif
      }
      else
      {	
	// mspass=(ulong)(minTime-prevms);
	trk=0;
	while (trk<info->ntracks)
	{
	  tracks[trk]->currentMs(minTime);
	  trk++;
	}
	midi->wait(minTime-diffTime);
      }
      trk=minTrk;
      tracks[trk]->readEvent(ev);
      switch (ev->command)
      {
	case (MIDI_NOTEON) : 
	  midi->noteOn(ev->chn, ev->note, ev->vel);break;
	case (MIDI_NOTEOFF): 
	  midi->noteOff(ev->chn, ev->note, ev->vel);break;
	case (MIDI_KEY_PRESSURE) :
	  midi->keyPressure(ev->chn, ev->note,ev->vel);break;
	case (MIDI_PGM_CHANGE) :
	  if (!ctl->forcepgm[ev->chn])
	    midi->chnPatchChange(ev->chn, (ctl->gm==1)?(ev->patch):(MT32toGM[ev->patch]));break;
	case (MIDI_CHN_PRESSURE) :
	  midi->chnPressure(ev->chn, ev->vel);break;
	case (MIDI_PITCH_BEND) :
	  midi->chnPitchBender(ev->chn, ev->d1,ev->d2);break;
	case (MIDI_CTL_CHANGE) :
	  midi->chnController(ev->chn, ev->ctl,ev->d1);break;
	case (MIDI_SYSTEM_PREFIX) :
	  if ((ev->command|ev->chn)==META_EVENT)
	  {
	    if ((ev->d1==5)||(ev->d1==1))
	    {
	      ctl->SPEVplayed++;
	    }
	    if (ev->d1==ME_SET_TEMPO)
	    {
	      absTimeAtChangeTempo=absTime;
	      ticksplayed=0;
	      ctl->SPEVplayed++;
	      tempo=(ulong)(((ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2]))*ctl->ratioTempo);
#ifdef PLAYERDEBUG
              printf("Tempo : %ld %g (ratio : %g)\n",tempo,tempoToMetronomeTempo(tempo),ctl->ratioTempo);
#endif
              midi->tmrSetTempo((int)tempoToMetronomeTempo(tempo));
              ctl->tempo=tempo;       
	      for (j=0;j<info->ntracks;j++)
	      {
		tracks[j]->changeTempo(tempo);
	      }
	    }
	    if (ev->d1==ME_TIME_SIGNATURE)
	    {
	      ctl->num=ev->d2;
	      ctl->den=ev->d3;
	      ctl->SPEVplayed++;
	    }
	  }
	  break;
      }
      if (calloutput)
      {
	midi->sync();
	output();
      }

    }
  ctl->ev=NULL;
  delete ev;
#ifdef PLAYERDEBUG
  printf("Syncronizing ...\n");
#endif
  if (halt) 
    midi->sync(1);
  else 
    midi->sync();
#ifdef PLAYERDEBUG
  printf("Closing device ...\n");
#endif
  midi->allNotesOff();
  midi->closeDev();
  ctl->playing=0;
#ifdef PLAYERDEBUG
  printf("Bye...\n");
#endif
  ctl->OK=1;
  ctl->finished=1;
}


void MidiPlayer::setPos(ulong gotomsec,MidiStatus *midistat)
{
  int trk,minTrk;
  ulong tempo=(ulong)(500000 * ctl->ratioTempo);
  double minTime=0,maxTime,prevms=0;
  int i,j,likeplaying=1;

  MidiEvent *ev=new MidiEvent;
  ctl->SPEVplayed=0;
  for (i=0;i<info->ntracks;i++)
  {
    tracks[i]->init();
    tracks[i]->changeTempo(tempo);
  }

  for (i=0;i<16;i++)
  {
    if (ctl->forcepgm[i]) midistat->chnPatchChange(i, ctl->pgm[i]);
  }

  while (likeplaying)
  {
    trk=0;
    minTrk=0;
    maxTime=minTime + 120000L; /*milliseconds (2 minutes)*/
    minTime=maxTime;
    while (trk<info->ntracks)
    {
      if (tracks[trk]->absMsOfNextEvent()<minTime)
      {
	minTrk=trk;
	minTime=tracks[minTrk]->absMsOfNextEvent();
      }
      trk++;
    }
    if (minTime==maxTime) 
    {
      likeplaying=0;
#ifdef GENERAL_DEBUG_MESSAGES
      printf("END of likeplaying\n");
#endif
    }
    else
    {	
      if (minTime>=gotomsec)
      {
	prevms=gotomsec;
	likeplaying=0;
#ifdef GENERAL_DEBUG_MESSAGES
	printf("Position reached !! \n");
#endif
	minTime=gotomsec;
      }
      else
      {
	prevms=minTime;
      }
      trk=0;
      while (trk<info->ntracks)
      {
	tracks[trk]->currentMs(minTime);
	trk++;
      }
    }

    if (likeplaying)
    {
      trk=minTrk;
      tracks[trk]->readEvent(ev);
      switch (ev->command)
      {
	/*  case (MIDI_NOTEON) : 
	    midistat->noteOn(ev->chn, ev->note, ev->vel);break;
	    case (MIDI_NOTEOFF): 
	    midistat->noteOff(ev->chn, ev->note, ev->vel);break;
	    case (MIDI_KEY_PRESSURE) :
	    midistat->keyPressure(ev->chn, ev->note,ev->vel);break;
	 */
	case (MIDI_PGM_CHANGE) :
	  if (!ctl->forcepgm[ev->chn]) midistat->chnPatchChange(ev->chn, ev->patch);break;
	case (MIDI_CHN_PRESSURE) :
	  midistat->chnPressure(ev->chn, ev->vel);break;
	case (MIDI_PITCH_BEND) :
	  midistat->chnPitchBender(ev->chn, ev->d1,ev->d2);break;
	case (MIDI_CTL_CHANGE) :
	  midistat->chnController(ev->chn, ev->ctl,ev->d1);break;
	case (MIDI_SYSTEM_PREFIX) :
	  if ((ev->command|ev->chn)==META_EVENT)
	  {
	    if ((ev->d1==5)||(ev->d1==1))
	    {
	      ctl->SPEVplayed++;
	    }
	    if (ev->d1==ME_SET_TEMPO)
	    {
	      ctl->SPEVplayed++;
	      tempo=(ulong)(((ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2]))*ctl->ratioTempo);

	      midistat->tmrSetTempo((int)tempoToMetronomeTempo(tempo));		
	      for (j=0;j<info->ntracks;j++)
	      {
		tracks[j]->changeTempo(tempo);
	      }
	    }
	    if (ev->d1==ME_TIME_SIGNATURE)
	    {
	      ctl->num=ev->d2;
	      ctl->den=ev->d3;
	      ctl->SPEVplayed++;
	    }
	  }
	  break;
      }
    }
  }
  delete ev;
  ctl->tempo=tempo;
}


void MidiPlayer::debugSpecialEvents(void)
{
  SpecialEvent *pspev=spev;
  printf("**************************************\n");
  while ((pspev!=NULL)&&(pspev->type!=0))
  {
    printf("t:%d ticks:%d diff:%ld abs:%ld s:%s tempo:%ld\n",pspev->type,pspev->ticks,pspev->diffmilliseconds,pspev->absmilliseconds,pspev->text,pspev->tempo);
    pspev=pspev->next;
  }

}

void MidiPlayer::setParseSong(bool b)
{
  parsesong=b;
}

void MidiPlayer::setGenerateBeats(bool b)
{
  generatebeats=b;
}

void MidiPlayer::setTempoRatio(double ratio)
{
  if (songLoaded)
  {
    ctl->ratioTempo=ratio;
    parseInfoData(info,tracks,ctl->ratioTempo);
    if (parsesong)
    {
      parseSpecialEvents();
      if (generatebeats) generateBeats();

    }
  }
  else
  {
    ctl->tempo=(ulong)((ctl->tempo*ctl->ratioTempo)/ratio);
    ctl->ratioTempo=ratio;
  }

}

#undef T2MS
#undef MS2T
