/**************************************************************************

    midfile.cc - function which reads a midi file,and creates the track classes
    Copyright (C) 1997  Antonio Larrosa Jimenez

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
#include "midfile.h"
#include <string.h>
#include <sys/soundcard.h>
#include "midispec.h"

//#define MIDFILE_DEBUG

/* This function gives the metronome tempo, from a tempo data as found in
 a midi file */
double tempoToMetronomeTempo(ulong x)
{
return 60/((double)x/1000000);
};


track **readMidiFile(char *name,midifileinfo *info)
{
track **Tracks;
FILE *fh=fopen(name,"rb");
if (fh==NULL) 
	{
	perror("Can't open file");
	exit(-1);
	};
char text[4];
fread(text,1,4,fh);
if (strncmp(text,"MThd",4)!=0)
	{
	perror("Not a midi file");
	exit(-1);
	};
long header_size=readLong(fh);
info->format=readShort(fh);
info->ntracks=readShort(fh);
info->ticksPerCuarterNote=readShort(fh);
if (info->ticksPerCuarterNote<0)
	{
	perror("Ticks per cuarter note is negative !");
	exit(-1);
	};
if (header_size>6) fseek(fh,header_size-6,SEEK_CUR);
Tracks=new track*[info->ntracks];
if (Tracks==NULL)
	{
	perror("Not enough memory");
	exit(-1);
	};
int i=0;
while (i<info->ntracks)
	{
	fread(text,1,4,fh);
	if (strncmp(text,"MTrk",4)!=0)
		{
		perror("Not a well built midi file");
		printf("%s",text);
		exit(-1);
		};
	Tracks[i]=new track(fh,info->ticksPerCuarterNote,i);
	if (Tracks[i]==NULL)
		{
		perror("Not enough memory");
		exit(-1);
		};
	i++;
	};

info->ticksTotal=0;
info->millisecsTotal=0.0;
info->ticksPlayed=0;
//ulong tempticks=0;
int parsing=1;
int trk,minTrk;
//ulong minTicks;
ulong tempo=1000000;
#ifdef MIDFILE_DEBUG
printf("Parsing 1 ...\n");
#endif
/*while (parsing)
    {
    trk=0;
    minTrk=0;
    minTicks=~0;
    while (trk<info->ntracks)
        {
        if (Tracks[trk]->waitTicks()<minTicks)
                {
                minTrk=trk;
                minTicks=Tracks[minTrk]->waitTicks();
                };
        trk++;
        };
    if ((minTicks==~(ulong)0) ||
(((((double)minTicks)*60000L)/(tempoToMetronomeTempo(tempo)*(double)info->ticksPerCuarterNote))> 60000L))
        {
        parsing=0;
        printf("END of parsing\n");
        }
	else
	{
	info->ticksTotal+=minTicks;
	tempticks+=minTicks;
	info->ticksPlayed+=(ulong)((double)(minTicks*(tempo/10000))/(double)info->ticksPerCuarterNote);
        trk=0;
        while (trk<info->ntracks)
            {
            Tracks[trk]->ticksPassed(minTicks);
            trk++;
            };
        trk=minTrk;
        Tracks[trk]->readEvent(&ev);
	if ((ev.command==MIDI_SYSTEM_PREFIX)&&(ev.d1==ME_SET_TEMPO))
		{
		info->millisecsTotal+=(tempticks*60L)/(ulong)(tempoToMetronomeTempo(tempo)*(double)info->ticksPerCuarterNote);
		tempticks=0;
		tempo=(ev.data[0]<<16)|(ev.data[1]<<8)|(ev.data[2]);
		};
	};
    
    };
info->millisecsTotal+=((double)(tempticks*60L)/(tempoToMetronomeTempo(tempo)*(double)info->ticksPerCuarterNote));*/
int j;
for (i=0;i<info->ntracks;i++)
    {
    Tracks[i]->init();
    };
double prevms=0;
double minTime=0;
double maxTime;
Midi_event *ev=new Midi_event;
while (parsing)
    {
    prevms=minTime;
    trk=0;
    minTrk=0;
    maxTime=minTime + 2 * 60000L;
    minTime=maxTime;
    while (trk<info->ntracks)
        {
        if (Tracks[trk]->absMsOfNextEvent()<minTime)
                {
                minTrk=trk;
                minTime=Tracks[minTrk]->absMsOfNextEvent();
                };
        trk++;
        };
    if ((minTime==maxTime))
        {
        parsing=0;
#ifdef MIDFILE_DEBUG
        printf("END of parsing\n");
#endif
        }
        else
        {
        trk=0;
        while (trk<info->ntracks)
            {
            Tracks[trk]->currentMs(minTime);
            trk++;
            };
        };
    trk=minTrk;
    Tracks[trk]->readEvent(ev);
        if (ev->command==MIDI_SYSTEM_PREFIX)
                {
                if (((ev->command|ev->chn)==META_EVENT)&&(ev->d1==ME_SET_TEMPO))
                    {
                    tempo=(ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2]);
//		    printf("setTempo %ld\n",tempo);
                    for (j=0;j<info->ntracks;j++)
                        {
                        Tracks[j]->changeTempo(tempo);
                        };
                    };
                };
    };

delete ev;
info->millisecsTotal=prevms;

for (i=0;i<info->ntracks;i++)
    {
    Tracks[i]->init();
    };
#ifdef MIDFILE_DEBUG
printf("info.ticksTotal = %ld \n",info->ticksTotal);
printf("info.ticksPlayed= %ld \n",info->ticksPlayed);
printf("info.millisecsTotal  = %g \n",info->millisecsTotal);
printf("info.TicksPerCN = %d \n",info->ticksPerCuarterNote);
#endif
return Tracks;
};
