/**************************************************************************

    player.cc  - class player, which plays a set of tracks
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
#include "player.h"
#include "sndcard.h"
#include "midispec.h"
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "midistat.h"
#include "mt32togm.h"

player::player(DeviceManager *midi_,PlayerController *pctl)
{
midi=midi_;
info=NULL;
tracks=NULL;
songLoaded=0;
ctl=pctl;
spev=NULL;
};

player::~player()
{
removeSpecialEvents();
removeSong();
};

void player::removeSong(void)
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
        };
    delete tracks;
    if (info!=NULL) delete info;
    };
songLoaded=0;
};

int player::loadSong(char *filename)
{
removeSong();
printf("Loading Song : %s\n",filename);
info=new midifileinfo;
int ok;
tracks=readMidiFile(filename,info,ok);
if (ok<0) return ok;
if (tracks==NULL) return -4;
parseSpecialEvents();
songLoaded=1;
return 0;
};

void player::removeSpecialEvents(void)
{
SpecialEvent * ev=spev;
while (spev!=NULL)
    {
    ev=spev->next;
    delete spev;
    spev=ev;
    };
};

void player::parseSpecialEvents(void)
{
#ifdef PLAYERDEBUG
printf("player::Parsing...\n");
#endif
removeSpecialEvents();
spev=new SpecialEvent;
SpecialEvent *pspev=spev;
pspev->type=0;
pspev->ticks=0;
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
int spev_id=1;
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
        if (ev->command==MIDI_SYSTEM_PREFIX)
                {
		if ((ev->command|ev->chn)==META_EVENT)
			{
                if ((ev->d1==1)||(ev->d1==5))
                    {
                    pspev->absmilliseconds=(ulong)minTime;
                    pspev->type=ev->d1;
		    pspev->id=spev_id++;
                    strncpy(pspev->text,(char *)ev->data,ev->length);
                    pspev->text[ev->length]=0;
                    pspev->next=new SpecialEvent;
                    pspev=pspev->next;
                    };
                if ((ev->d1==ME_SET_TEMPO)&&(tempoToMetronomeTempo(tmp=((ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2])))>=8))
                    {
                    pspev->absmilliseconds=(ulong)minTime;
                    pspev->type=3;
		    pspev->id=spev_id++;
                    tempo=tmp;
                    pspev->tempo=tempo;
                    for (j=0;j<info->ntracks;j++)
                        {
                        tracks[j]->changeTempo(tempo);
                        };
                    pspev->next=new SpecialEvent;
                    pspev=pspev->next;
                    };
		};
                };
        };   
    
delete ev;
pspev->type=0;
pspev->next=NULL;
//writeSPEV();
for (int i=0;i<info->ntracks;i++)
    {
    tracks[i]->init();
    };
};

void player::play(int calloutput,void output(void))
{		
#ifdef PLAYERDEBUG
printf("Playing...\n");
#endif
midi->openDev();
if (midi->OK()==0) {printf("Player :: Couldn't play !\n");ctl->error=1;return;};
midi->initDev();
parsePatchesUsed(tracks,info,ctl->gm);
midi->setPatchesToUse(info->patchesUsed);

int trk;
int minTrk;
double minTime=0;
double maxTime;
for (int i=0;i<info->ntracks;i++)
    {
    tracks[i]->init();
    }; 
midi->tmrStart();
ulong tempo=1000000;
ulong tmp;
Midi_event *ev=new Midi_event;
ctl->ev=ev;
ctl->ticksTotal=info->ticksTotal;
ctl->ticksPlayed=0;
ctl->millisecsPlayed=0;
ulong ticksplayed=0;
double absTimeAtChangeTempo=0;
double absTime=0;
double diffTime=0;
midiStat *midistat;
//ulong mspass;
double prevms=0;
int j;
int halt=0;
ctl->tempo=tempo;
int playing;
ctl->paused=0;
if ((ctl->message!=0)&&(ctl->message & PLAYER_SETPOS))
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
	midistat->sendData(midi,ctl->gm);
	delete midistat;
	ctl->moving=0;
	};
timeval begintv;
gettimeofday(&begintv, NULL);
ctl->beginmillisec=begintv.tv_sec*1000+begintv.tv_usec/1000;
ctl->OK=1;
ctl->playing=playing=1;
while (playing)
    {
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
/*	if (ctl->message & PLAYER_SETPOS)
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
*/	};
    prevms=minTime;
    ctl->millisecsPlayed=minTime;
    trk=0;
    minTrk=0;
    maxTime=minTime + 120000L /* milliseconds */;
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
    if ((minTime==maxTime)/* || (minTicks> 60000L)*/)
	{
	playing=0;
#ifdef PLAYERDEBUG
	printf("END of playing\n");
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
	midi->wait(minTime-diffTime);
	};
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
                            };
			if ((ev->d1==ME_SET_TEMPO)&&(tempoToMetronomeTempo(tmp=((ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2])))>=8))
			    {
			    absTimeAtChangeTempo=absTime;
			    ticksplayed=0;
			    ctl->SPEVplayed++;
			    tempo=tmp;
			    midi->tmrSetTempo((int)tempoToMetronomeTempo(tempo));
			    ctl->tempo=tempo;
			    for (j=0;j<info->ntracks;j++)
			       	{
			       	tracks[j]->changeTempo(tempo);
			       	}; 
			    };
			};
		break;
	};

    if (calloutput)
	{
	sync();
	output();
	};
    
    };
ctl->ev=NULL;
delete ev;
#ifdef PLAYERDEBUG
printf("Syncronizing ...\n");
#endif
if (halt) 
    midi->sync(1);
   else 
    midi->sync();
midi->closeDev();
ctl->playing=0;
#ifdef PLAYERDEBUG
printf("Bye...\n");
#endif
ctl->OK=1;
ctl->finished=1;
};


void player::SetPos(ulong gotomsec,midiStat *midistat)
{
int trk;
int minTrk;
ulong tempo=1000000;
ulong tmp;
double minTime,maxTime;
double prevms=0;
minTime=0;
int likeplaying=1;
Midi_event *ev=new Midi_event;
ctl->SPEVplayed=0;
int i,j;
for (i=0;i<info->ntracks;i++)
    {
    tracks[i]->init();
    }; 

while (likeplaying)
    {
    trk=0;
    minTrk=0;
    maxTime=minTime+ 2 /*minutes*/ * 60000L;
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
		}
		else
		{
		prevms=minTime;
		};
        trk=0;
        while (trk<info->ntracks)
            {
	    tracks[trk]->currentMs(minTime);
            trk++;
            };
	};
    if (likeplaying) 
	{
	trk=minTrk;
	tracks[trk]->readEvent(ev);
	switch (ev->command)
	    {
/*	    case (MIDI_NOTEON) : 
		midistat->noteOn(ev->chn, ev->note, ev->vel);break;
	    case (MIDI_NOTEOFF): 
		midistat->noteOff(ev->chn, ev->note, ev->vel);break;
	    case (MIDI_KEY_PRESSURE) :
		midistat->keyPressure(ev->chn, ev->note,ev->vel);break;
*/	    case (MIDI_PGM_CHANGE) :
		midistat->chnPatchChange(ev->chn, ev->patch);break;
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
                            };
			if ((ev->d1==ME_SET_TEMPO)&&(tempoToMetronomeTempo(tmp=((ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2])))>=8))
			    {
			    ctl->SPEVplayed++;
			    tempo=tmp;
	
			    midistat->tmrSetTempo((int)tempoToMetronomeTempo(tempo));		
                            for (j=0;j<info->ntracks;j++)
                                {
                                tracks[j]->changeTempo(tempo);
                                };	
			    };
			};
		break;
	    };
	};
    };
delete ev;
ctl->tempo=tempo;
};


void player::writeSPEV(void)
{
SpecialEvent *pspev=spev;
printf("**************************************\n");
while ((pspev!=NULL)&&(pspev->type!=0))
    {
    printf("t:%d ticks:%d diff:%ld abs:%ld s:%s tempo:%d\n",pspev->type,pspev->ticks,pspev->diffmilliseconds,pspev->absmilliseconds,pspev->text,pspev->tempo);
    pspev=pspev->next;
    };

};
