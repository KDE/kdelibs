/**************************************************************************

    midimapper.cc  - The midi mapper object  
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
#include "midimapper.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

MidiMapper::MidiMapper(const char *name)
{
  _ok=1;
  keymaps=NULL;
  _filename=NULL;
  mapPitchBender=0;
  mapExpressionToVolumeEvents=0;
  if ((name==NULL)||(name[0]==0))
  {
    deallocateMaps();    
    int i;
    for (i=0;i<16;i++) 
    {
      channelmap[i]=i;
      channelPatchForced[i]=-1;
    }
    for (i=0;i<128;i++) patchmap[i]=i;
  }
  else
    loadFile(name);
}

MidiMapper::~MidiMapper()
{
  delete _filename;
  deallocateMaps();
}

void MidiMapper::deallocateMaps(void)
{
  int i;
  for (i=0;i<16;i++) channelKeymap[i]=NULL;
  for (i=0;i<128;i++) patchKeymap[i]=NULL;
  Keymap *km;
  while (keymaps!=NULL)
  {
    km=keymaps->next;
    delete keymaps;
    keymaps=km;
  }
}

void MidiMapper::getValue(char *s,char *v)
{
  char *c=s;
  while ((*c!=0)&&(*c!='=')) c++;
  if (*c==0) v[0]=0;
  else
  {
    c++;
    while (*c!=0)
    {
      *v=*c;
      c++;v++;
    }
    *v=0;
  }
}

void MidiMapper::removeSpaces(char *s)
{
  char *a=s;
  while ((*a!=0)&&(*a==' ')) a++;
  if (*a==0) {*s=0;return;};
  while (*a!=0)
  {
    while ((*a!=0)&&(*a!=' ')&&(*a!=10)&&(*a!=13))    
    {
      *s=*a;
      s++;
      a++;
    }
    while ((*a!=0)&&((*a==' ')||(*a==10)||(*a==13))) a++;
    *s=' ';s++;
    if (*a==0) {*s=0;return;};
  }
  *s=0;

}

int MidiMapper::countWords(char *s)
{
  int c=0;
  while (*s!=0)
  {
    if (*s==' ') c++;
    s++;
  }
  return c;
}

void MidiMapper::getWord(char *t,char *s,int w)
{
  int i=0;
  *t=0;
  while ((*s!=0)&&(i<w))
  {
    if (*s==' ') i++;
    s++;
  }
  while ((*s!=0)&&(*s!=' ')&&(*s!=10)&&(*s!=13))
  {
    *t=*s;
    t++;s++;
  }
  *t=0;
}


void MidiMapper::loadFile(const char *name)
{
  _ok=1;
  FILE *fh = fopen(name,"rt");
  if ( fh == NULL ) { _ok = -1; return; };
  char s[101];
  s[0] = 0;
  if ( _filename != NULL ) delete _filename;
  _filename = new char[ strlen(name)+1 ];
  strcpy(_filename,name);
#ifdef MIDIMAPPERDEBUG
  printf("Loading mapper ...\n");
#endif
  while (!feof(fh))
  {
    s[0]=0;
    while ((!feof(fh))&&((s[0]==0)||(s[0]=='#'))) fgets(s,100,fh);
    if (strncmp(s,"DEFINE",6)==0)
    {
      if (strncmp(&s[7],"PATCHMAP",8)==0) readPatchmap(fh);
      else
	if (strncmp(&s[7],"KEYMAP",6)==0) readKeymap(fh,s);
	else
	  if (strncmp(&s[7],"CHANNELMAP",10)==0) readChannelmap(fh);
	  else
	  {
	    printf("ERROR: Unknown DEFINE line in map file\n");
	    _ok=0;
	  }
      if (_ok==0)
      {
	printf("The midi map file will be ignored\n");
	fclose(fh);
	return;
      }
    }
    else if (strncmp(s,"OPTIONS",7)==0) readOptions(fh);	
  }
  fclose(fh);
}

MidiMapper::Keymap *MidiMapper::createKeymap(char *name,uchar use_same_note,uchar note)
{
  Keymap *km=new Keymap;
  strcpy(km->name,name);
  int i;
  if (use_same_note==1)
  {
    for (i=0;i<128;i++)
      km->key[i]=note;
  }
  else
  {
    for (i=0;i<128;i++)
      km->key[i]=i;
  }
  addKeymap(km);
  return km;
}

void MidiMapper::addKeymap(Keymap *newkm)
{
  Keymap *km=keymaps;
  if (keymaps==NULL)
  {
    keymaps=newkm;
    newkm->next=NULL;
    return;
  }
  while (km->next!=NULL) km=km->next;
  km->next=newkm;
  newkm->next=NULL;
  return;
}

MidiMapper::Keymap *MidiMapper::keymap(char *n)
{
  Keymap *km=keymaps;
  while ((km!=NULL)&&(strcmp(km->name,n)!=0)) km=km->next;
  return km;
}

void MidiMapper::readOptions(FILE *fh)
{
#ifdef MIDIMAPPERDEBUG
  printf("Loading Options ... \n");
#endif
  char s[101];
  char v[101];
  char t[101];
  int fin=0;
  mapPitchBender=0;
  while (!fin)
  {
    s[0]=0;
    while ((s[0]==0)||(s[0]=='#')) fgets(s,100,fh);
    if (strncmp(s,"PitchBenderRatio",16)==0) 
    {
      getValue(s,v);
      removeSpaces(v);
      getWord(t,v,0);
      mapPitchBender=1;
      pitchBenderRatio=atoi(t);
    }
    else if (strncmp(s,"MapExpressionToVolumeEvents",27)==0) mapExpressionToVolumeEvents=1;
    else if (strncmp(s,"END",3)==0) 
    {
      fin=1;
    }
    else 
    {
      printf("ERROR: Invalid option in OPTIONS section of map file : (%s)\n",s);
      _ok=0;
      return;
    }
  }
}

void MidiMapper::readPatchmap(FILE *fh)
{
  char s[101];
  char v[101];
  char t[101];
  char name[101];
  int i=0;
  int j,w;
#ifdef MIDIMAPPERDEBUG
  printf("Loading Patch map ... \n");
#endif
  while (i<128)
  {
    s[0]=0;
    while ((s[0]==0)||(s[0]=='#')) fgets(s,100,fh);
    getValue(s,v);
    removeSpaces(v);
    w=countWords(v);
    j=0;
    patchKeymap[i]=NULL;
    patchmap[i]=i;
    while (j<w)
    {
      getWord(t,v,j);
      if (strcmp(t,"AllKeysTo")==0)
      {
	j++;
	if (j>=w) 
	{
	  printf("ERROR: Invalid option in PATCHMAP section of map file\n");
	  _ok=0;
	  return;
	}
	getWord(t,v,j);
	sprintf(name,"AllKeysTo%s",t);
	patchKeymap[i]=createKeymap(name,1,atoi(t));
      }
      else
      {
	patchmap[i]=atoi(t);
      }
      j++;
    }
    i++;
  }
  s[0]=0;
  while ((s[0]==0)||(s[0]=='#')||(s[0]==10)||(s[0]==13)) fgets(s,100,fh);
  if (strncmp(s,"END",3)!=0)
  {
    printf("ERROR: End of section not found in map file\n");
    _ok=0;
    return;
  }
}

void MidiMapper::readKeymap(FILE *fh,char *first_line)
{
  char s[101];
  char v[101];
#ifdef MIDIMAPPERDEBUG
  printf("Loading Key map ... %s",first_line);
#endif
  removeSpaces(first_line);
  getWord(v,first_line,2);
  Keymap *km=new Keymap;
  strcpy(km->name,v);
  int i=0;
  while (i<128)
  {
    s[0]=0;
    while ((s[0]==0)||(s[0]=='#')) fgets(s,100,fh);
    getValue(s,v);
    removeSpaces(v);
    km->key[i]=atoi(v);
    i++;
  }
  s[0]=0;
  while ((s[0]==0)||(s[0]=='#')||(s[0]==10)||(s[0]==13)) fgets(s,100,fh);
  if (strncmp(s,"END",3)!=0)
  {
    printf("ERROR: End of section not found in map file\n");
    _ok=0;
    return;
  }
  addKeymap(km);
}

void MidiMapper::readChannelmap(FILE *fh)
{
  char s[101];
  char v[101];
  char t[101];
  int i=0;
  int w,j;
#ifdef MIDIMAPPERDEBUG
  printf("Loading Channel map ... \n");
#endif
  while (i<16)
  {
    s[0]=0;
    while ((s[0]==0)||(s[0]=='#')) fgets(s,100,fh);
    getValue(s,v);
    removeSpaces(v);
    w=countWords(v);
    j=0;
    channelKeymap[i]=NULL;
    channelPatchForced[i]=-1;
    channelmap[i]=i;
    while (j<w)
    {
      getWord(t,v,j);
      if (strcmp(t,"Keymap")==0)
      {
	j++;
	if (j>=w) 
	{
	  printf("ERROR: Invalid option in CHANNELMAP section of map file\n");
	  _ok=0;
	  return;
	}
	getWord(t,v,j);
	channelKeymap[i]=keymap(t); 
      }
      else if (strcmp(t,"ForcePatch")==0)
      {
	j++;
	if (j>=w) 
	{
	  printf("ERROR: Invalid option in CHANNELMAP section of map file\n");
	  _ok=0;
	  return;
	}
	getWord(t,v,j);
	channelPatchForced[i]=atoi(t); 
      }
      else
      {
	channelmap[i]=atoi(t); 
      }
      j++;
    }
    i++;
  }
  s[0]=0;
  while ((s[0]==0)||(s[0]=='#')||(s[0]==10)||(s[0]==13)) fgets(s,100,fh);
  if (strncmp(s,"END",3)!=0)
  {
    printf("END of section not found in map file\n");
    _ok=0;
    return;
  }

}

const char *MidiMapper::filename(void)
{
  return (_filename)? _filename : "";
}

uchar MidiMapper::key(uchar chn,uchar pgm, uchar note)
{
  uchar notemapped=note;
  if (patchKeymap[pgm]!=NULL) notemapped=patchKeymap[pgm]->key[note];
  if (channelKeymap[chn]!=NULL) notemapped=channelKeymap[chn]->key[note];
  return notemapped;
}

uchar MidiMapper::patch(uchar chn,uchar pgm)
{
  return (channelPatchForced[chn] == -1) ? 
    patchmap[pgm] : (uchar)channelPatchForced[chn] ;
}

void MidiMapper::pitchBender(uchar ,uchar &lsb,uchar &msb)
{
  if (mapPitchBender)
  {
    short pbs=((short)msb<<7) | (lsb & 0x7F);
    pbs=pbs-0x2000;
    short pbs2=(((long)pbs*pitchBenderRatio)/4096);
#ifdef MIDIMAPPERDEBUG
    printf("Pitch Bender (%d): %d -> %d \n",chn,pbs,pbs2);
#endif
    pbs2=pbs2+0x2000;
    lsb=pbs2 & 0x7F;
    msb=(pbs2 >> 7)&0x7F;  
  }
}

void MidiMapper::controller(uchar ,uchar &ctl, uchar &)
{
  if ((mapExpressionToVolumeEvents)&&(ctl==11)) ctl=7;
}
