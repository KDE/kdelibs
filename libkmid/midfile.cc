/**************************************************************************

    midfile.cc - function which reads a midi file,and creates the track classes
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

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/
#include "midfile.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sndcard.h"
#include "midispec.h"
#include "mt32togm.h"
#include "sys/stat.h"
#include <config.h>

#include <kprocess.h>
#include <qfile.h>

int fsearch(FILE *fh,const char *text,long *ptr);

/* This function gives the metronome tempo, from a tempo data as found in
   a midi file */
double tempoToMetronomeTempo(ulong x)
{
  return 60/((double)x/1000000);
}

double metronomeTempoToTempo(ulong x)
{
  return ((double)60*x)/1000000;
}

int uncompressFile(const char *gzname, char *tmpname)
  // Returns 0 if OK, 1 if error (tmpname not set)
{
  QString cmd("gzip -dc " + KShellProcess::quote(gzname));
  FILE *infile = popen( QFile::encodeName(cmd).data(), "r");
  if (infile==NULL) {
    fprintf(stderr,"ERROR : popen failed : %s\n",QFile::encodeName(cmd).data());
    return 1;
  }
  strcpy(tmpname, "/tmp/KMid.XXXXXXXXXX");
  int fd = mkstemp(tmpname);
  if (fd == -1)
  {
    pclose(infile);
    return 1;
  }
  FILE *outfile= fdopen(fd,"wb");
  if (outfile==NULL)
  {
    pclose(infile);
    return 1;
  }
  int n=getc(infile);
  if (n==EOF) 
  {
    pclose(infile);
    fclose(outfile);
    unlink(tmpname);
    return 1;
  }
  fputc(n,outfile);
  int buf[BUFSIZ];
  n = fread(buf, 1, BUFSIZ, infile);
  while (n>0)
  {
    fwrite(buf, 1, n, outfile);
    n = fread(buf, 1, BUFSIZ, infile);
  }

  pclose(infile);

  //if (pclose(infile) != 0) fprintf(stderr,"Error : pclose failed\n");
  // Is it right for pclose to always fail ?

  fclose(outfile);
  return 0;
}

MidiTrack **readMidiFile( const char *name, MidiFileInfo *info, int &ok)
{
  ok=1;
  MidiTrack **tracks;

  struct stat buf;
  stat(name,&buf);
  if (!S_ISREG(buf.st_mode))
  {
    fprintf(stderr,"ERROR: %s is not a regular file\n",name);
    ok=-6;
    return NULL;
  }

  FILE *fh=fopen(name,"rb");
  if (fh==NULL) 
  {
    fprintf(stderr,"ERROR: Can't open file %s\n",name);
    ok=-1;
    return NULL;
  }
  char text[4];
  fread(text,1,4,fh);
  if ((strncmp(text,"MThd",4)!=0)&&(strcmp(&name[strlen(name)-3],".gz")==0))
  {	
    fclose(fh);
    char tempname[200];
    fprintf(stderr,"Trying to open zipped midi file...\n");
    if (uncompressFile(name,tempname)!=0)
    {
      fprintf(stderr,"ERROR: %s is not a (zipped) midi file\n",name);
      ok=-2;
      return NULL;
    }
    fh=fopen(tempname,"rb");
    fread(text,1,4,fh);
    unlink(tempname);
  }

  if (strncmp(text,"MThd",4)!=0)
  {
    fseek(fh,0,SEEK_SET);
    long pos;
    if (fsearch(fh,"MThd",&pos)==0)
    {	
      fclose(fh);
      fprintf(stderr,"ERROR: %s is not a midi file.\n",name);
      ok=-2;
      return NULL;
    }
    fseek(fh,pos,SEEK_SET);
    fread(text,1,4,fh);
  }
  long header_size=readLong(fh);
  info->format=readShort(fh);
  info->ntracks=readShort(fh);
  info->ticksPerCuarterNote=readShort(fh);
  if (info->ticksPerCuarterNote<0)
  {
    fprintf(stderr,"ERROR: Ticks per cuarter note is negative !\n");
    fprintf(stderr,"Please report this error to : larrosa@kde.org\n");
    fclose(fh);
    ok=-3;
    return NULL;
  }
  if (header_size>6) fseek(fh,header_size-6,SEEK_CUR);
  tracks=new MidiTrack*[info->ntracks];
  if (tracks==NULL)
  {
    fprintf(stderr,"ERROR: Not enough memory\n");
    fclose(fh);
    ok=-4;
    return NULL;
  }
  int i=0;
  while (i<info->ntracks)
  {
    fread(text,1,4,fh);
    if (strncmp(text,"MTrk",4)!=0)
    {
      fprintf(stderr,"ERROR: Not a well built midi file\n");
      fprintf(stderr,"%s",text);
      fclose(fh);
      ok=-5;
      return NULL;
    }
    tracks[i]=new MidiTrack(fh,info->ticksPerCuarterNote,i);
    if (tracks[i]==NULL)
    {
      fprintf(stderr,"ERROR: Not enough memory");
      fclose(fh);
      ok=-4;
      return NULL;
    }
    i++;
  }

  fclose(fh);

  return tracks;

}

void parseInfoData(MidiFileInfo *info,MidiTrack **tracks,float ratioTempo)
{

  info->ticksTotal=0;
  info->millisecsTotal=0.0;
  info->ticksPlayed=0;
  int i;
  for (i=0;i<256;i++)
  {
    info->patchesUsed[i]=0;
  }

  int parsing=1;
  int trk,minTrk;
  ulong tempo=(ulong)(500000 * ratioTempo);

#ifdef MIDFILEDEBUG
  printf("Parsing 1 ...\n");
#endif

  int pgminchannel[16];
  for (i=0;i<16;i++) 
  {
    pgminchannel[i]=0;
  }

  int j;
  for (i=0;i<info->ntracks;i++)
  {
    tracks[i]->init();
    tracks[i]->changeTempo(tempo);
  }
  double prevms=0;
  double minTime=0;
  double maxTime;
  MidiEvent *ev=new MidiEvent;
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
      }
      trk++;
    }
    if ((minTime==maxTime))
    {
      parsing=0;
#ifdef MIDFILEDEBUG
      printf("END of parsing\n");
#endif
    }
    else
    {
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
	if (ev->chn!=PERCUSSION_CHANNEL)
	  info->patchesUsed[pgminchannel[ev->chn]]++;
	else
	  info->patchesUsed[ev->note+128]++;
	break;
      case (MIDI_PGM_CHANGE) :
	pgminchannel[ev->chn]=(ev->patch);
	break;
      case (MIDI_SYSTEM_PREFIX) :
	if (((ev->command|ev->chn)==META_EVENT)&&(ev->d1==ME_SET_TEMPO))
	{
	  tempo=(ulong)(((ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2])) * ratioTempo);
	  for (j=0;j<info->ntracks;j++)
	  {
	    tracks[j]->changeTempo(tempo);
	  }
	}
	break;
    }
  }

  delete ev;
  info->millisecsTotal=prevms;

  for (i=0;i<info->ntracks;i++)
  {
    tracks[i]->init();
  }

#ifdef MIDFILEDEBUG
  printf("info.ticksTotal = %ld \n",info->ticksTotal);
  printf("info.ticksPlayed= %ld \n",info->ticksPlayed);
  printf("info.millisecsTotal  = %g \n",info->millisecsTotal);
  printf("info.TicksPerCN = %d \n",info->ticksPerCuarterNote);
#endif

}


void parsePatchesUsed(MidiTrack **tracks,MidiFileInfo *info,int gm)
{
  int i;
  for (i=0;i<256;i++)
  {
    info->patchesUsed[i]=0;
  }
  int parsing=1;
  int trk,minTrk;
  ulong tempo=500000;

#ifdef MIDFILEDEBUG
  printf("Parsing for patches ...\n");
#endif

  int j;
  for (i=0;i<info->ntracks;i++)
  {
    tracks[i]->init();
  }
  double prevms=0;
  double minTime=0;
  double maxTime;
  ulong tmp;
  MidiEvent *ev=new MidiEvent;
  int pgminchannel[16];
  for (i=0;i<16;i++)
  {
    pgminchannel[i]=0;
  }

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
      }
      trk++;
    }
    if ((minTime==maxTime))
    {
      parsing=0;
#ifdef MIDFILEDEBUG
      printf("END of parsing for patches\n");
#endif
    }
    else
    {
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
	if (ev->chn!=PERCUSSION_CHANNEL)
	  info->patchesUsed[pgminchannel[ev->chn]]++;
	else
	  info->patchesUsed[ev->note+128]++;
	break;
      case (MIDI_PGM_CHANGE) :
	pgminchannel[ev->chn]=(gm==1)?(ev->patch):(MT32toGM[ev->patch]);
	break;
      case (MIDI_SYSTEM_PREFIX) :
	if (((ev->command|ev->chn)==META_EVENT)&&(ev->d1==ME_SET_TEMPO))
	{
	  if (tempoToMetronomeTempo(tmp=((ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2])))>=8)
	  {
	    tempo=tmp;
	    //                   printf("setTempo %ld\n",tempo);
	    for (j=0;j<info->ntracks;j++)
	    {
	      tracks[j]->changeTempo(tempo);
	    }
	  }
	}
	break;   
    }
  }

  delete ev;

  for (i=0;i<info->ntracks;i++)
  {
    tracks[i]->init();
  }

}

int fsearch(FILE *fh,const char *text,long *ptr)
  // Search for "text" throught the fh file and then returns :
  // text MUST BE smaller than 256 characters
  //   0 if not was found 
  //   1 if it was found and in ptr (if !=NULL) the position where text begins.
{
  if ((text==NULL)||(text[0]==0)) return 0;
  char buf[1024];
  char tmp[256];
  long pos;
  int l=strlen(text);
  int i,k,r;
  while (!feof(fh))
  {
    pos=ftell(fh);
    k=fread(buf,1,1024,fh);
    i=0;
    while (i<k)
    {
      if (buf[i]==text[0])
      {
	if (k-i>=l)
	  r=strncmp(text,&buf[i],l);
	else
	{
	  fseek(fh,pos+i,SEEK_SET);
	  if (fread(tmp,1,l,fh)<(uint)l) return 0;
	  fseek(fh,pos+k,SEEK_SET);
	  r=strncmp(text,tmp,l);
	}
	if (r==0)
	{
	  if (ptr!=NULL) *ptr=pos+i;
	  return 1;
	}
      }
      i++;
    }
  }
  return 0;
}
