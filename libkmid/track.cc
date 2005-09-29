/**************************************************************************

    track.cc  - class track, which has a midi file track and its events
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

#include "track.h"
#include <stdlib.h>
#include "sndcard.h"
#include "midispec.h"
#include "midfile.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define T2MS(ticks) (((double)ticks)*(double)60000L)/((double)tempoToMetronomeTempo(tempo)*(double)tPCN)

#define MS2T(ms) (((ms)*(double)tempoToMetronomeTempo(tempo)*(double)tPCN)/((double)60000L))

#define PEDANTIC_TRACK
#define CHANGETEMPO_ONLY_IN_TRACK0
//#define TRACKDEBUG
//#define TRACKDEBUG2

MidiTrack::MidiTrack(FILE *file,int tpcn,int Id)
{
  id=Id;
  tPCN=tpcn;
  currentpos=0;
  size=0;
  data=0L;
  tempo=1000000;
  if (feof(file)) 
  {
    clear();
    return; 
  };
  size=readLong(file);
#ifdef TRACKDEBUG
  printf("Track %d : Size %ld\n",id,size);
#endif
  data=new uchar[size];
  if (data==NULL)
  {
    perror("track: Not enough memory ?");
    exit(-1);
  }
  ulong rsize=0;
  if ((rsize=fread(data,1,size,file))!=size)
  {
    fprintf(stderr,"track (%d): File is corrupt : Couldn't load track (%ld!=%ld) !!\n", id, rsize, size);
    size=rsize;
  };
  /*
     ptrdata=data;
     current_ticks=0;
     delta_ticks=readVariableLengthValue();
     wait_ticks=delta_ticks;
     endoftrack=0;
   */
  init();
}

MidiTrack::~MidiTrack()
{
  delete data;
  endoftrack=1;
  currentpos=0;
  size=0;
}

int MidiTrack::power2to(int i)
{
  return 1<<i;
}

ulong MidiTrack::readVariableLengthValue(void)
{
  ulong dticks=0;

  while ((*ptrdata) & 0x80)
  {
#ifdef PEDANTIC_TRACK
    if (currentpos>=size)
    {
      endoftrack=1;
      fprintf(stderr, "track (%d) : EndofTrack found by accident !\n",id);
      delta_ticks = wait_ticks = ~0;	
      time_at_next_event=10000 * 60000L;
      return 0;
    }
    else
#endif
    {
      dticks=(dticks << 7) | (*ptrdata) & 0x7F;
      ptrdata++;currentpos++;
    }

  }
  dticks=((dticks << 7) | (*ptrdata) & 0x7F);
  ptrdata++;currentpos++;

#ifdef PEDANTIC_TRACK

  if (currentpos>=size)
  {
    endoftrack=1;
    fprintf(stderr,"track (%d): EndofTrack found by accident 2 !\n",id);
    dticks=0;
    delta_ticks = wait_ticks = ~0;	
    time_at_next_event=10000 * 60000L;
    return 0;
  }
#endif
#ifdef TRACKDEBUG
  printfdebug("track(%d): DTICKS : %ld\n",id,dticks);
  usleep(10);
#endif
  return dticks;
}

int MidiTrack::ticksPassed (ulong ticks)
{
  if (endoftrack==1) return 0;
  if (ticks>wait_ticks) 
  {
    printfdebug("track (%d): ERROR : TICKS PASSED > WAIT TICKS\n", id);
    return 1;
  }
  wait_ticks-=ticks;
  return 0;
}

int MidiTrack::msPassed (ulong ms)
{
  if (endoftrack==1) return 0;
  current_time+=ms;
  //fprintf(stderr, "old + %ld = CURR %g  ", ms,current_time);
  if ( current_time>time_at_next_event )
  {
    fprintf(stderr, "track (%d): ERROR : MS PASSED > WAIT MS\n", id);
    return 1;
  }
#ifdef TRACKDEBUG
  if (current_time==time_at_next_event) printfdebug("track(%d): _OK_",id);
#endif
  return 0;
}

int MidiTrack::currentMs(double ms)
{
  if (endoftrack==1) return 0;
  current_time=ms;
  //printfdebug("CURR %g",current_time);
#ifdef PEDANTIC_TRACK
  if (current_time>time_at_next_event) 
  {
    fprintf(stderr,"track(%d): ERROR : MS PASSED > WAIT MS\n", id);
    exit(-1);
    return 1;
  }
#endif
  return 0;
}

void MidiTrack::readEvent(MidiEvent *ev)
{
  int i,j;
  if (endoftrack==1) 
  {
    ev->command=0;
    return;
  }
  /*
     printfdebug("...... %d\n",id);
     printfdebug("current : %g , tane : %g\n",current_time,time_at_next_event);
     printfdebug("......\n");
   */
  int skip_event=0;
  current_time=time_at_next_event;
  if (((*ptrdata)&0x80)!=0)
  {
    ev->command=(*ptrdata);
    ptrdata++;currentpos++;
    lastcommand=ev->command;
  }
  else
  {
    ev->command=lastcommand;
  }

#ifdef PEDANTIC_TRACK
  if (currentpos>=size)
  {
    endoftrack=1;
    delta_ticks = wait_ticks = ~0;	
    time_at_next_event=10000 * 60000L;
    ev->command=MIDI_SYSTEM_PREFIX;
    ev->chn=0xF;
    ev->d1=ME_END_OF_TRACK;
    fprintf(stderr, "track (%d): EndofTrack found by accident 3\n",id);
    return;
  }
#endif

  ev->chn=ev->command & 0xF;
  ev->command=ev->command & 0xF0;
  switch (ev->command)
  {
    case (MIDI_NOTEON) :
      ev->note = *ptrdata;ptrdata++;currentpos++;
      ev->vel  = *ptrdata;ptrdata++;currentpos++;
      if (ev->vel==0)
	note[ev->chn][ev->note]=FALSE;
      else
	note[ev->chn][ev->note]=TRUE;

#ifdef TRACKDEBUG2
      if (ev->chn==6) {
	if (ev->vel==0) printfdebug("Note Onf\n");
	else printfdebug("Note On\n");
      };
#endif
      break;
    case (MIDI_NOTEOFF) :
#ifdef TRACKDEBUG2
      if (ev->chn==6) printfdebug("Note Off\n");
#endif
      ev->note = *ptrdata;ptrdata++;currentpos++; 
      ev->vel  = *ptrdata;ptrdata++;currentpos++;
      note[ev->chn][ev->note]=FALSE;

      break;
    case (MIDI_KEY_PRESSURE) :
#ifdef TRACKDEBUG2
      if (ev->chn==6) printfdebug ("Key press\n");
#endif
      ev->note = *ptrdata;ptrdata++;currentpos++; 
      ev->vel  = *ptrdata;ptrdata++;currentpos++;
      break;
    case (MIDI_PGM_CHANGE) :
#ifdef TRACKDEBUG2
      if (ev->chn==6) printfdebug ("Pgm\n");
#endif
      ev->patch = *ptrdata;ptrdata++;currentpos++; 
      break;
    case (MIDI_CHN_PRESSURE) :
#ifdef TRACKDEBUG2
      if (ev->chn==6) printfdebug ("Chn press\n");
#endif
      ev->vel  = *ptrdata;ptrdata++;currentpos++;
      break;
    case (MIDI_PITCH_BEND) :
#ifdef TRACKDEBUG2
      if (ev->chn==6) printfdebug ("Pitch\n");
#endif
      ev->d1 = *ptrdata;ptrdata++;currentpos++; 
      ev->d2 = *ptrdata;ptrdata++;currentpos++;
      break;
    case (MIDI_CTL_CHANGE) :
#ifdef TRACKDEBUG2
      if (ev->chn==6) printfdebug (stderr, "Ctl\n");
#endif
      ev->ctl = *ptrdata;ptrdata++; currentpos++;
      ev->d1  = *ptrdata;ptrdata++;currentpos++;
      /*
	 switch (ev->ctl)
	 {
	 case (96) : printfdebug("RPN Increment\n");break;
	 case (97) : printfdebug("RPN Decrement\n");break;
	 case (98) : printfdebug("nRPN 98 %d\n",ev->d1);break;
	 case (99) : printfdebug("nRPN 99 %d\n",ev->d1);break;
	 case (100) : printfdebug("RPN 100 %d\n",ev->d1);break;
	 case (101) : printfdebug("RPN 101 %d\n",ev->d1);break;
	 };
       */
      break;

    case (MIDI_SYSTEM_PREFIX) :
#ifdef TRACKDEBUG2
      if (ev->chn==6) printfdebug ("Sys Prefix\n");
#endif
      switch ((ev->command|ev->chn))
      {
	case (0xF0) : 
	case (0xF7) :
	  ev->length=readVariableLengthValue();
#ifdef PEDANTIC_TRACK
	  if (endoftrack) 
	  {
	    ev->command=MIDI_SYSTEM_PREFIX;
	    ev->chn=0xF;
	    ev->d1=ME_END_OF_TRACK;
	  }
	  else
#endif
	  {
	    ev->data=ptrdata;
	    ptrdata+=ev->length;currentpos+=ev->length;
	  }
	  break;
	case (0xFE):
	case (0xF8):
	  //		printfdebug("Active sensing\n");
	  break;
	case (META_EVENT) : 
	  ev->d1=*ptrdata;ptrdata++;currentpos++;
	  switch (ev->d1)
	  {
	    case (ME_END_OF_TRACK) :
	      i=0;
	      j=0;
	      while ((note[i][j]==FALSE)&&(i<16))
	      {
		j++;
		if (j==128) { j=0; i++; };
	      }
	      if (i<16) // that is, if there is any key still pressed
	      {
		ptrdata--;currentpos--;
		ev->chn=i;
		ev->command=MIDI_NOTEOFF;
		ev->note = j;
		ev->vel  = 0;
		note[ev->chn][ev->note]=FALSE;
		fprintf(stderr,"Note Off(simulated)\n");
		return;
	      }
	      else
	      {
		endoftrack=1;
		delta_ticks = wait_ticks = ~0;
		time_at_next_event=10000 * 60000L;
#ifdef TRACKDEBUG
		printfdebug("EndofTrack %d event\n",id);
#endif
	      }
	      break;
	    case (ME_SET_TEMPO):
	      ev->length=readVariableLengthValue();
#ifdef PEDANTIC_TRACK
	      if (endoftrack) 
	      {
		ev->command=MIDI_SYSTEM_PREFIX;
		ev->chn=0xF;
		ev->d1=ME_END_OF_TRACK;
	      }
	      else
#endif
	      {
		ev->data=ptrdata;
		ptrdata+=ev->length;currentpos+=ev->length;
		//  tempo=((ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2]));
		//  ticks_from_previous_tempochange=0;
		// 	time_at_previous_tempochange=current_time;
#ifdef TRACKDEBUG
		printfdebug("Track %d : Set Tempo : %ld\n",id,tempo);
#endif
#ifdef CHANGETEMPO_ONLY_IN_TRACK0
		if (id!=0) skip_event=1;
#endif
	      }
	      break;
	    case (ME_TIME_SIGNATURE) :
	      ev->length=*ptrdata;ptrdata++;currentpos++;
	      ev->d2=*ptrdata;ptrdata++;currentpos++;
	      ev->d3=power2to(*ptrdata);ptrdata++;currentpos++;
	      ev->d4=*ptrdata;ptrdata++;currentpos++;
	      ev->d5=*ptrdata;ptrdata++;currentpos++;
#ifdef TRACKDEBUG
	      printfdebug("TIME SIGNATURE :\n");	
	      printfdebug("%d\n",ev->d2);
	      printfdebug("----  %d metronome , %d number of 32nd notes per quarter note\n",ev->d4,ev->d5);
	      printfdebug("%d\n",ev->d3);
#endif
	      break;
	    case (ME_TRACK_SEQ_NUMBER) :
	    case (ME_TEXT) :
	    case (ME_COPYRIGHT) :
	    case (ME_SEQ_OR_TRACK_NAME) :
	    case (ME_TRACK_INSTR_NAME) :
	    case (ME_LYRIC) :
	    case (ME_MARKER) :
	    case (ME_CUE_POINT) :
	    case (ME_CHANNEL_PREFIX) :
	    case (ME_MIDI_PORT) :
	    case (ME_SMPTE_OFFSET) :
	    case (ME_KEY_SIGNATURE) :
	      ev->length=readVariableLengthValue();
#ifdef PEDANTIC_TRACK
	      if (endoftrack) 
	      {
		ev->command=MIDI_SYSTEM_PREFIX;
		ev->chn=0xF;
		ev->d1=ME_END_OF_TRACK;
	      }
	      else
#endif
	      {
		ev->data=ptrdata;
		ptrdata+=ev->length;currentpos+=ev->length;
	      }
	      break;
	    default: 
#ifdef GENERAL_DEBUG_MESSAGES
	      fprintf(stderr,"track (%d) : Default handler for meta event " \
		  "0x%x\n", id, ev->d1);
#endif
	      ev->length=readVariableLengthValue();
#ifdef PEDANTIC_TRACK
	      if (endoftrack) 
	      {
		ev->command=MIDI_SYSTEM_PREFIX;
		ev->chn=0xF;
		ev->d1=ME_END_OF_TRACK;
	      }
	      else
#endif
	      {
		ev->data=ptrdata;
		ptrdata+=ev->length;currentpos+=ev->length;
	      }
	      break;
	  }
	  break;
	default : 
	  fprintf(stderr,"track (%d): Default handler for system event 0x%x\n",
	      id, (ev->command|ev->chn));
	  break;
      }
      break;
    default : 
      fprintf(stderr,"track (%d): Default handler for event 0x%x\n",
	  id, (ev->command|ev->chn));
      break;
  }
#ifdef PEDANTIC_TRACK
  if (currentpos>=size)
  {
    endoftrack=1;
    delta_ticks = wait_ticks = ~0;	
    time_at_next_event=10000 * 60000L;
    printfdebug("track (%d): EndofTrack reached\n",id);
  }
#endif
  if (endoftrack==0)
  {
    current_ticks+=delta_ticks;
    delta_ticks=readVariableLengthValue();
#ifdef PEDANTIC_TRACK
    if (endoftrack) 
    {
      ev->command=MIDI_SYSTEM_PREFIX;
      ev->chn=0xF;
      ev->d1=ME_END_OF_TRACK;
      return;
    }
#endif
    ticks_from_previous_tempochange+=delta_ticks;

    time_at_next_event=T2MS(ticks_from_previous_tempochange)+time_at_previous_tempochange;
    /*
       printf("tane2 : %g, ticks : %g, delta_ticks %ld, tempo : %ld\n",
       time_at_next_event,ticks_from_previous_tempochange,delta_ticks,tempo);
       printf("timeatprevtc %g , curr %g\n",time_at_previous_tempochange,current_time);
     */
    wait_ticks=delta_ticks;

  }
  if (skip_event) readEvent(ev);
}


void MidiTrack::clear(void)
{
  endoftrack=1;
  ptrdata=data;
  current_ticks=0;
  currentpos=0;

  for (int i=0;i<16;i++)
    for (int j=0;j<128;j++)
      note[i][j]=FALSE;

  delta_ticks = wait_ticks = ~0;	
  time_at_previous_tempochange=0;
  current_time=0;
  ticks_from_previous_tempochange=0;
  tempo=1000000;
  time_at_next_event=10000 * 60000L;

}


void MidiTrack::init(void)
{
  if (data==0L) { clear(); return; };
  endoftrack=0;
  ptrdata=data;
  current_ticks=0;
  currentpos=0;

  for (int i=0;i<16;i++)
    for (int j=0;j<128;j++)
      note[i][j]=FALSE;

  delta_ticks=readVariableLengthValue();
  if (endoftrack) return;
  wait_ticks=delta_ticks;


  time_at_previous_tempochange=0;
  current_time=0;
  ticks_from_previous_tempochange=wait_ticks;
  tempo=1000000;
  time_at_next_event=T2MS(delta_ticks);
  //printf("tane1 : %g\n",time_at_next_event);
}

void MidiTrack::changeTempo(ulong t)
{
  if (endoftrack==1) return;
  if (tempo==t) return;
  double ticks;
  time_at_previous_tempochange=current_time;
  ticks=MS2T(time_at_next_event-current_time);
  tempo=t;
  time_at_next_event=T2MS(ticks)+current_time;
  ticks_from_previous_tempochange=ticks;

}

/*
double MidiTrack::absMsOfNextEvent (void) 
{
  //printf("%d : %g\n",id,time_at_next_event);
  return time_at_next_event;
}
*/

#undef T2MS
#undef MS2T
