/**************************************************************************

    midimapper.cc  - The midi mapper object  
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
#include "midimapper.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../version.h"

MidiMapper::MidiMapper(const char *name)
{
ok=1;
keymaps=NULL;
filename=NULL;
mapPitchBender=0;
mapExpressionToVolumeEvents=0;
if ((name==NULL)||(name[0]==0))
    {
    DeallocateMaps();    
    int i;
    for (i=0;i<16;i++) 
	{
	channel[i]=i;
	channelPatchForced[i]=-1;
	};
    for (i=0;i<128;i++) patchmap[i]=i;
    }
   else
    LoadFile(name);
};

MidiMapper::~MidiMapper()
{
if (filename!=NULL) delete filename;
DeallocateMaps();
};

void MidiMapper::DeallocateMaps(void)
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
    };
}

void MidiMapper::getValue(char *s,char *v)
{
char *c=s;
while ((*c!=0)&&(*c!='=')) c++;
if (c==0) v[0]=0;
   else
    {
    c++;
    while (*c!=0)
        {
        *v=*c;
        c++;v++;
        };
    *v=0;
    };
};

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
        };
    while ((*a!=0)&&((*a==' ')||(*a==10)||(*a==13))) a++;
    *s=' ';s++;
    if (*a==0) {*s=0;return;};
    };
*s=0;

};

int MidiMapper::countWords(char *s)
{
int c=0;
while (*s!=0)
    {
    if (*s==' ') c++;
    s++;
    };
return c;
};

void MidiMapper::getWord(char *t,char *s,int w)
{
int i=0;
*t=0;
while ((*s!=0)&&(i<w))
    {
    if (*s==' ') i++;
    s++;
    };
while ((*s!=0)&&(*s!=' ')&&(*s!=10)&&(*s!=13))
    {
    *t=*s;
    t++;s++;
    };
*t=0;
};


void MidiMapper::LoadFile(const char *name)
{
ok=1;
FILE *fh=fopen(name,"rt");
if (fh==NULL) {ok=-1;return;};
char s[101];
s[0]=0;
if (filename!=NULL) delete filename;
filename=new char[strlen(name)+1];
strcpy(filename,name);
printf("Loading mapper ...\n");
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
		ok=0;
                };
	if (ok==0)
	    {
	    printf("The midi map file will be ignored\n");
	    fclose(fh);
	    return;
	    };
        }
       else if (strncmp(s,"OPTIONS",7)==0) readOptions(fh);	
    };
fclose(fh);
};

Keymap *MidiMapper::createKeymap(char *name,uchar use_same_note,uchar note)
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
    };
AddKeymap(km);
return km;
};

void MidiMapper::AddKeymap(Keymap *newkm)
{
Keymap *km=keymaps;
if (keymaps==NULL)
    {
    keymaps=newkm;
    newkm->next=NULL;
    return;
    };
while (km->next!=NULL) km=km->next;
km->next=newkm;
newkm->next=NULL;
return;
};

Keymap *MidiMapper::GiveMeKeymap(char *n)
{
Keymap *km=keymaps;
while ((km!=NULL)&&(strcmp(km->name,n)!=0)) km=km->next;
return km;
};

void MidiMapper::readOptions(FILE *fh)
{
printf("Loading Options ... \n");
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
        PitchBenderRatio=atoi(t);
	}
	else if (strncmp(s,"MapExpressionToVolumeEvents",27)==0) mapExpressionToVolumeEvents=1;
	else if (strncmp(s,"END",3)==0) 
	  {
	fin=1;
	}
        else 
	  {
          printf("ERROR: Invalid option in OPTIONS section of map file : (%s)\n",s);
	  ok=0;
	  return;
          };
   };
};

void MidiMapper::readPatchmap(FILE *fh)
{
char s[101];
char v[101];
char t[101];
char name[101];
int i=0;
int j,w;
printf("Loading Patch map ... \n");
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
		ok=0;
		return;
                };
            getWord(t,v,j);
	    sprintf(name,"AllKeysTo%s",t);
            patchKeymap[i]=createKeymap(name,1,atoi(t));
            }
           else
            {
            patchmap[i]=atoi(t);
            };
        j++;
        };
    i++;
    };
s[0]=0;
while ((s[0]==0)||(s[0]=='#')||(s[0]==10)||(s[0]==13)) fgets(s,100,fh);
if (strncmp(s,"END",3)!=0)
    {
    printf("ERROR: End of section not found in map file\n");
    ok=0;
    return;
    };
};

void MidiMapper::readKeymap(FILE *fh,char *first_line)
{
char s[101];
char v[101];
printf("Loading Key map ... %s",first_line);
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
    };
s[0]=0;
while ((s[0]==0)||(s[0]=='#')||(s[0]==10)||(s[0]==13)) fgets(s,100,fh);
if (strncmp(s,"END",3)!=0)
    {
    printf("ERROR: End of section not found in map file\n");
    ok=0;
    return;
    };
AddKeymap(km);
};

void MidiMapper::readChannelmap(FILE *fh)
{
char s[101];
char v[101];
char t[101];
int i=0;
int w,j;
printf("Loading Channel map ... \n");
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
    channel[i]=i;
    while (j<w)
        {
        getWord(t,v,j);
        if (strcmp(t,"Keymap")==0)
            {
            j++;
            if (j>=w) 
                {
                printf("ERROR: Invalid option in CHANNELMAP section of map file\n");
		ok=0;
		return;
                };
            getWord(t,v,j);
            channelKeymap[i]=GiveMeKeymap(t); 
            }
        else if (strcmp(t,"ForcePatch")==0)
	    {
            j++;
            if (j>=w) 
                {
                printf("ERROR: Invalid option in CHANNELMAP section of map file\n");
		ok=0;
		return;
                };
            getWord(t,v,j);
	    channelPatchForced[i]=atoi(t); 
	    }
           else
            {
            channel[i]=atoi(t); 
            };
        j++;
        };
    i++;
    };
s[0]=0;
while ((s[0]==0)||(s[0]=='#')||(s[0]==10)||(s[0]==13)) fgets(s,100,fh);
if (strncmp(s,"END",3)!=0)
    {
    printf("END of section not found in map file\n");
    ok=0;
    return;
    };

};

uchar MidiMapper::Key(uchar chn,uchar pgm, uchar note)
{
uchar notemapped=note;
if (patchKeymap[pgm]!=NULL) notemapped=patchKeymap[pgm]->key[note];
if (channelKeymap[chn]!=NULL) notemapped=channelKeymap[chn]->key[note];
return notemapped;
};


char *MidiMapper::getFilename(void)
{
return (filename!=NULL)? filename : (char *)"";
};

uchar MidiMapper::Patch(uchar chn,uchar pgm)
{
return (channelPatchForced[chn] == -1) ? 
		patchmap[pgm] : (uchar)channelPatchForced[chn] ;
};

void MidiMapper::PitchBender(uchar chn,uchar &lsb,uchar &msb)
{
if (mapPitchBender)
   {
   short pbs=((short)msb<<7) | (lsb & 0x7F);
   pbs=pbs-0x2000;
   short pbs2=(((long)pbs*PitchBenderRatio)/4096);
#ifdef MIDIMAPPERDEBUG
   printf("Pitch Bender (%d): %d -> %d \n",chn,pbs,pbs2);
#endif
   pbs2=pbs2+0x2000;
   lsb=pbs2 & 0x7F;
   msb=(pbs2 >> 7)&0x7F;  
   };
};

void MidiMapper::Controller(uchar chn,uchar &ctl, uchar &v)
{
if ((mapExpressionToVolumeEvents)&&(ctl==11)) ctl=7;
};
