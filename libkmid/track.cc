/**************************************************************************

    track.cc  - class track, which has a midi file track and its events
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
#include "track.h"
#include <stdlib.h>
#include "sndcard.h"
#include "midispec.h"
#include "midfile.h"

#define T2MS(ticks) (((double)ticks)*(double)60000L)/((double)tempoToMetronomeTempo(tempo)*(double)tPCN)

#define MS2T(ms) (((ms)*(double)tempoToMetronomeTempo(tempo)*(double)tPCN)/((double)60000L))


#define PEDANTIC_TRACK

track::track(FILE *file,int tpcn,int Id)
{
id=Id;
tPCN=tpcn;
currentpos=0;
size=readLong(file);
data=(uchar *)malloc(size);
if (data==NULL)
	{
	perror("Not enough memory ?");
	exit(-1);
	};
fread(data,1,size,file);
/*ptrdata=data;
current_ticks=0;
delta_ticks=readVariableLengthValue();
wait_ticks=delta_ticks;
endoftrack=0;*/
init();
};

track::~track()
{
free(data);
endoftrack=1;
currentpos=0;
size=0;
};

int track::power2to(int i)
{
return 1<<i;
/*int d=1;
while (i>0)
    {
    d*=2;
    i--;
    };
return d;
*/
};

ulong track::readVariableLengthValue(void)
{
ulong dticks=0;

while ((*ptrdata) & 0x80)
	{
#ifdef PEDANTIC_TRACK
	if (currentpos>=size)
	    {
	    endoftrack=1;
	    printf("EndofTrack %d encountered !\n",id);
	    delta_ticks = wait_ticks = ~0;	
    	    time_at_next_event=10000 * 60000L;
	    return 0;
	    }
	   else
#endif
	    {
	    dticks=(dticks << 7) | (*ptrdata) & 0x7F;
	    ptrdata++;currentpos++;
	    };

	};
dticks=((dticks << 7) | (*ptrdata) & 0x7F);
ptrdata++;currentpos++;

#ifdef PEDANTIC_TRACK

if (currentpos>=size)
    {
    endoftrack=1;
    printf("EndofTrack %d encountered 2 !\n",id);
    dticks=0;
    delta_ticks = wait_ticks = ~0;	
    time_at_next_event=10000 * 60000L;
    return 0;
    }
#endif
#ifdef TRACKDEBUG
printf("DTICKS : %ld\n",dticks);
#endif
return dticks;
};

int track::ticksPassed (ulong ticks)
{
if (endoftrack==1) return 0;
if (ticks>wait_ticks) {printf("ERROR : TICKS PASSED > WAIT TICKS\n");return 1;};
wait_ticks-=ticks;
return 0;
};

int track::msPassed (ulong ms)
{
if (endoftrack==1) return 0;
current_time+=ms;
//printf("old + %ld = CURR %g  ",ms,current_time);
if (current_time>time_at_next_event) {printf("ERROR : MS PASSED > WAIT MS\n");return 1;};
#ifdef TRACKDEBUG
if (current_time==time_at_next_event) printf("_OK_");
#endif
return 0;
};

int track::currentMs(double ms)
{
if (endoftrack==1) return 0;
current_time=ms;
//printf("CURR %g",current_time);
#ifdef PEDANTIC_TRACK
if (current_time>time_at_next_event) 
	{
	printf("ERROR : MS PASSED > WAIT MS\n");
	exit(-1);
	return 1;
	};
#endif
return 0;
};



void track::readEvent(Midi_event *ev)
{
if (endoftrack==1) 
	{
	ev->command=0;
	return;
	};
/*
printf("...... %d\n",id);
printf("current : %g , tane : %g\n",current_time,time_at_next_event);
printf("......\n");
*/
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
	};

ev->chn=ev->command & 0xF;
ev->command=ev->command & 0xF0;
switch (ev->command)
    {
    case (MIDI_NOTEON) :
	ev->note = *ptrdata;ptrdata++;currentpos++;
	ev->vel  = *ptrdata;ptrdata++;currentpos++;
	break;
    case (MIDI_NOTEOFF) :
	ev->note = *ptrdata;ptrdata++;currentpos++; 
	ev->vel  = *ptrdata;ptrdata++;currentpos++;
	break;
    case (MIDI_KEY_PRESSURE) :
	ev->note = *ptrdata;ptrdata++;currentpos++; 
	ev->vel  = *ptrdata;ptrdata++;currentpos++;
	break;
    case (MIDI_PGM_CHANGE) :
	ev->patch = *ptrdata;ptrdata++;currentpos++; 
	break;
    case (MIDI_CHN_PRESSURE) :
	ev->vel  = *ptrdata;ptrdata++;currentpos++;
	break;
    case (MIDI_PITCH_BEND) :
	ev->d1 = *ptrdata;ptrdata++;currentpos++; 
	ev->d2 = *ptrdata;ptrdata++;currentpos++;
	break;
    case (MIDI_CTL_CHANGE) :
	ev->ctl = *ptrdata;ptrdata++; currentpos++;
	ev->d1  = *ptrdata;ptrdata++;currentpos++;
/*	switch (ev->ctl)
		{
		case (96) : printf("RPN Increment\n");break;
		case (97) : printf("RPN Decrement\n");break;
		case (98) : printf("nRPN 98 %d\n",ev->d1);break;
		case (99) : printf("nRPN 99 %d\n",ev->d1);break;
		case (100) : printf("RPN 100 %d\n",ev->d1);break;
		case (101) : printf("RPN 101 %d\n",ev->d1);break;
		};
*/	break;
    
    case (MIDI_SYSTEM_PREFIX) :
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
			};
		break;
	    case (0xFE):
	    case (0xF8):
//		printf("Active sensing\n");
		break;
	    case (META_EVENT) : 
		ev->d1=*ptrdata;ptrdata++;currentpos++;
		switch (ev->d1)
		    {
		    case (ME_END_OF_TRACK) :
			endoftrack=1;
			delta_ticks = wait_ticks = ~0;	
    			time_at_next_event=10000 * 60000L;
			
//			printf("EndofTrack %d event\n",id);
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
			tempo=((ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2]));
			ticks_from_previous_tempochange=0;
			time_at_previous_tempochange=current_time;
			};
			break;
		    case (ME_TIME_SIGNATURE) :
			ev->length=*ptrdata;ptrdata++;currentpos++;
			ev->d2=*ptrdata;ptrdata++;currentpos++;
			ev->d3=*ptrdata;ptrdata++;currentpos++;
			ev->d4=*ptrdata;ptrdata++;currentpos++;
			ev->d5=*ptrdata;ptrdata++;currentpos++;
/*
			printf("TIME SIGNATURE :\n");	
			printf("%d\n",ev->d2);
			printf("----  %d metronome , %d number of 32nd notes per quarter note\n",ev->d4,ev->d5);
			printf("%d\n",power2to(ev->d3));
*/
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
			};
			break;
		    default: 
#ifdef GENERAL_DEBUG_MESSAGES
			printf("Warning : Default handler for meta event " \
				"0x%x\n", ev->d1);
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
			};
			break;
		    };
		break;
	    default : 
		printf("Warning: Default handler for system event 0x%x\n",
							(ev->command|ev->chn));
		break;
	    };
	break;
	default : 
	    printf("Warning: Default handler for event 0x%x\n",
							(ev->command|ev->chn));
	    break;
     };
#ifdef PEDANTIC_TRACK
if (currentpos>=size)
    {
    endoftrack=1;
    delta_ticks = wait_ticks = ~0;	
    time_at_next_event=10000 * 60000L;
    printf("EndofTrack %d reached\n",id);
    };
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
			};
#endif
    ticks_from_previous_tempochange+=delta_ticks;

    time_at_next_event=T2MS(ticks_from_previous_tempochange)+time_at_previous_tempochange;
/*
printf("tane2 : %g, ticks : %g, delta_ticks %ld, tempo : %ld\n",
	time_at_next_event,ticks_from_previous_tempochange,delta_ticks,tempo);
printf("timeatprevtc %g , curr %g\n",time_at_previous_tempochange,current_time);
*/
    wait_ticks=delta_ticks;

    };
};

void track::init(void)
{
endoftrack=0;
ptrdata=data;
current_ticks=0;
currentpos=0;
delta_ticks=readVariableLengthValue();
		if (endoftrack) return;
wait_ticks=delta_ticks;


time_at_previous_tempochange=0;
current_time=0;
ticks_from_previous_tempochange=wait_ticks;
tempo=1000000;
time_at_next_event=T2MS(delta_ticks);
//printf("tane1 : %g\n",time_at_next_event);
};

void track::changeTempo(ulong t)
{
if (endoftrack==1) return;
if (tempo==t) return;
double ticks;
//	printf("********-----%d, %d\n",id,tPCN);
time_at_previous_tempochange=current_time;
//	printf("tane : %g , current : %g\n",time_at_next_event,current_time);
ticks=MS2T(time_at_next_event-current_time);
//	printf("ticks : %g , oldtime : %g, oldtempo : %ld\n"
//				,ticks,time_at_next_event,tempo);
//	printf("ticks from previous timechange : %g\n",
//				ticks_from_previous_tempochange);
tempo=t;
//	printf("deltaticks : %ld\n",delta_ticks);
//	printf("The Tempo has changed in track %d, now it is set to %ld\n",id,tempo);
time_at_next_event=T2MS(ticks)+current_time;
//	printf("time : %g\n",time_at_next_event);
ticks_from_previous_tempochange=ticks;

};

double track::absMsOfNextEvent (void) 
{
//printf("%d : %g\n",id,time_at_next_event);
return time_at_next_event;
}; 
