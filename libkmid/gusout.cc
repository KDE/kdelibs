/**************************************************************************

    gusout.cc  - class gusOut which implements support for Gravis
         Ultrasound cards through a /dev/sequencer device
    Copyright (C) 1998  Antonio Larrosa Jimenez

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
#include "gusout.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "sndcard.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include "../version.h"
#include "midispec.h"

SEQ_USE_EXTBUF();

gusOut::gusOut(int d,int total)
{
seqfd = -1;
devicetype=KMID_GUS;
device= d;
#ifdef HANDLETIMEINDEVICES
count=0.0;
lastcount=0.0;
rate=100;
#endif
ok=1;

use8bit=0;
nvoices=total;
vm=new voiceManager(nvoices);
};

gusOut::~gusOut()
{
delete Map;
closeDev();
if (delete_GUS_patches_directory) 
    {
    delete GUS_patches_directory;
    delete_GUS_patches_directory = 0;
    GUS_patches_directory="/etc";
    };
};

void gusOut::openDev (int sqfd)
{
ok=1;
seqfd = sqfd;
//vm->cleanLists();
if (seqfd==-1)
    {
    printf("ERROR: Could not open /dev/sequencer\n");
    return;
    };
#ifdef HANDLETIMEINDEVICES
ioctl(seqfd,SNDCTL_SEQ_NRSYNTHS,&ndevs);
ioctl(seqfd,SNDCTL_SEQ_NRMIDIS,&nmidiports);

rate=0;
int r=ioctl(seqfd,SNDCTL_SEQ_CTRLRATE,&rate);
if ((r==-1)||(rate<=0)) rate=HZ;
convertrate=1000/rate;

count=0.0;
lastcount=0.0;

#endif
//seqbuf_clean();
//ioctl(seqfd,SNDCTL_SEQ_RESET);
//ioctl(seqfd,SNDCTL_SEQ_PANIC);

if (ioctl(seqfd, SNDCTL_SEQ_RESETSAMPLES, &device)==-1) 
   {
   printf("Error reseting gus samples. Please report\n");
   };
use8bit=0;
totalmemory = device;
ioctl(seqfd, SNDCTL_SYNTH_MEMAVL, &totalmemory);
freememory = device;
ioctl(seqfd, SNDCTL_SYNTH_MEMAVL, &freememory);

#ifdef GUSOUTDEBUG
printf("GUS Device %d opened (%d voices)\n",device,nvoices);
printf("Number of synth devices : %d\n",ndevs);
printf("Number of midi ports : %d\n",nmidiports);
printf("Rate : %d\n",rate);
#endif


};

void gusOut::closeDev (void)
{
if (!OK()) return;
#ifdef HANDLETIMEINDEVICES
SEQ_STOP_TIMER();
SEQ_DUMPBUF();
#endif
vm->cleanLists();
//if (seqfd>=0)
//    close(seqfd);
seqfd=-1;
};

void gusOut::initDev (void)
{
int chn;
if (!OK()) return;
#ifdef HANDLETIMEINDEVICES
count=0.0;
lastcount=0.0;
#endif
uchar gm_reset[5]={0x7e, 0x7f, 0x09, 0x01, 0xf7};
sysex(gm_reset, sizeof(gm_reset));
for (chn=0;chn<16;chn++)
    {
    chn_mute[chn]=0;
    chnPatchChange(chn,0);
    chnPressure(chn,127);
    chnPitchBender(chn, 0x00, 0x40);
    chnController(chn, CTL_MAIN_VOLUME,127);
    chnController(chn, CTL_EXT_EFF_DEPTH, 0);
    chnController(chn, CTL_CHORUS_DEPTH, 0);
    chnController(chn, 0x4a, 127);
    };


for (int i = 0; i < nvoices; i++)
     {
     SEQ_CONTROL(device, i, SEQ_VOLMODE, VOL_METHOD_LINEAR);
     SEQ_STOP_NOTE(device, i, vm->Note(i), 64);
     };

};


int gusOut::Patch(int p)
{
if (patchloaded[p]==1) return p;
printf("Not loaded %d!\n",p);
p=0;
while ((p<256)&&(patchloaded[p]==0)) p++;
return p;
};

void gusOut::noteOn  (uchar chn, uchar note, uchar vel)
{
if (vel==0)
    {
    noteOff(chn,note,vel);
    }
   else
    {
    if (chn==PERCUSSION_CHANNEL)
        {
	if (patchloaded[note+128]==0) return;
       else
	if (patchloaded[chn_patch[chn]]==0) return;
        };
    int v=vm->allocateVoice(chn,note);
    int p;
    if (chn==PERCUSSION_CHANNEL)
        SEQ_SET_PATCH(device,v ,p=Patch(note+128))
       else
        SEQ_SET_PATCH(device,v ,p=Map->Patch(chn,chn_patch[chn])); 
    SEQ_BENDER(device, v, chn_bender[chn]);

    SEQ_START_NOTE(device, v, note, vel);
    SEQ_CONTROL(device, v, CTL_MAIN_VOLUME, chn_controller[chn][CTL_MAIN_VOLUME]);
    SEQ_CHN_PRESSURE(device, v , chn_pressure[chn]);
    };

#ifdef GUSOUTDEBUG
printf("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
};

void gusOut::noteOff (uchar chn, uchar note, uchar vel)
{
int i;
vm->initSearch();
while ((i=vm->Search(chn,note))!=-1)
   {
   SEQ_STOP_NOTE(device, i, note, vel);
   vm->deallocateVoice(i);
   };

#ifdef GUSOUTDEBUG
printf("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
};

void gusOut::keyPressure (uchar chn, uchar note, uchar vel)
{
int i;
vm->initSearch();
while ((i=vm->Search(chn,note))!=-1)
   SEQ_KEY_PRESSURE(device, i, note,vel);
};

void gusOut::chnPatchChange (uchar chn, uchar patch)
{
if (chn==PERCUSSION_CHANNEL) return;
int i;
vm->initSearch();
while ((i=vm->Search(chn))!=-1)
   SEQ_SET_PATCH(device,i,Map->Patch(chn,patch)); 
chn_patch[chn]=patch;
   
};

void gusOut::chnPressure (uchar chn, uchar vel)
{
int i;
vm->initSearch();
while ((i=vm->Search(chn))!=-1)
   SEQ_CHN_PRESSURE(device, i , vel);
chn_pressure[chn]=vel;
};

void gusOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
//chn_bender[chn]=(msb << 8) | (lsb & 0xFF);
//chn_bender[chn]=(msb << 7)+ (lsb);
chn_bender[chn]=((int)msb<<7) | (lsb & 0x7F);

int i;
vm->initSearch();
while ((i=vm->Search(chn))!=-1)
//   SEQ_PITCHBEND(device, i, chn_bender[chn]);
   SEQ_BENDER(device, i, chn_bender[chn]);
/*
int i=0;
while (i<nvoices)
    {
    if ((vm->Used(i)==1)&&(vm->Channel(i)==chn))
	SEQ_BENDER(device, i, chn_bender[chn]);
    i++;
    };
*/
};

void gusOut::chnController (uchar chn, uchar ctl, uchar v) 
{
int i;
vm->initSearch();
while ((i=vm->Search(chn))!=-1)
   SEQ_CONTROL(device, i, ctl, v);

/*
int i=0;
while (i<nvoices)
    {
    if ((vm->Used(i)==1)&&(vm->Channel(i)==chn))
	SEQ_CONTROL(device, i, ctl, v);
    i++;
    };
*/
chn_controller[chn][ctl]=v;
};

void gusOut::sysex(uchar *, ulong )
{

};

void gusOut::setGUSPatchesDirectory(const char *dir)
{
if ((dir==NULL)||(dir[0]==0)) return;
if (delete_GUS_patches_directory) delete GUS_patches_directory;
GUS_patches_directory=new char[strlen(dir)+1];
strcpy(GUS_patches_directory,dir);
delete_GUS_patches_directory=1;
};

char *gusOut::patchName(int pgm)
{
return "acpiano";
};


int gusOut::loadPatch(int pgm)
{
if (patchloaded[pgm]==1)
    {
    printf("Trying to reload a patch. This should never happen, please report.\n");
    return 0;
    };
if (patchName(pgm)==NULL)
    {
    printf("Couldn't guess patch name for patch number %d\n",pgm);
    return -1;
    };
char *s=new char[strlen(GUS_patches_directory)+strlen(patchName(pgm))+10];
if (s==NULL) return -1;
sprintf(s,"%s/%s.pat",GUS_patches_directory,patchName(pgm));
patch_info *patch;
struct stat info;
if (stat(s, &info)==-1)
   {
   printf("File %s doesn't exist\n",s);
   return -1;
   };

FILE *fh=fopen(s,"rb");
if (fh==NULL)
   {
   printf("Couldn't open patch %s\n",s);
   return -1;
   };

char tmp[256];
fread(tmp,0xef,1,fh);
if (strncmp(&tmp[0],"GF1PATCH110",12)!=0)
    {
    printf("File %s is corrupted or it isn't a patch file\n",s);
    return -1;
    };
if (strncmp(&tmp[12],"ID#000002",10)!=0)
    {
    printf("File %s's version is not supported\n",s);
    return -1;
    };
unsigned short nWaves= *(unsigned short *)&tmp[85];
unsigned short mainVolume= *(unsigned short *)&tmp[87];
unsigned short i;
int wavedataposition=0xef;
for (i=0;i<nWaves;i++)
    {
    };


delete s;
freememory = device;
ioctl(seqfd, SNDCTL_SYNTH_MEMAVL, &freememory);
return 0;
};


void gusOut::setPatchesToUse(int *patchesused)
{
int patchesordered[256]; //This holds the pgm used ordered by a method which
               // put first the patches more oftenly used, and then the least
               // In example, if a song only uses a piano and a splash cymbal,
               // This is set to : 0,188,-1,-1,-1,-1 ...
getPatchesLoadingOrder(patchesused,patchesordered);

int i=0;
while (patchesordered[i]!=-1)
    {
    loadPatch(patchesordered[i]);
    i++;
    };
};

void gusOut::getPatchesLoadingOrder(int *patchesused,int *patchesordered)
{
int tempmelody[128];
int tempdrums[128];
int i,j;
for (i=0,j=128;i<128;i++,j++) 
   {
   tempmelody[i]=patchesused[i];
   tempdrums[i]=patchesused[j];
   };
/* SORT */ // Decreasing order (first most used patch, then less used patch)
/* Once they are sorted, the result is put on patchesordered in the following
  way : If tempmelody is : M0 M1 M2 M3 ... M127 and tempdrums is :
  D0 D1 D2 D3 ... D127, the result is :
     M0 D0 M1 M2 D1 M3 M4 D2 M5 M6 D3  ...
     P0 P1 P2 P3 P4 P5 P6 P7 P8 P9 P10 ...
*/

i=0;
int totalmelody=0;
while ((i<128)&&(tempmelody[i]!=0))
   {
   totalmelody++;
   i++;
   };
i=0;
int totaldrums=0;
while ((i<128)&&(tempdrums[i]!=0))
   {
   totaldrums++;
   i++;
   };
int c=0;
int tgt=0;
while (c<totalmelody)
    {
    patchesordered[tgt]=tempmelody[c];
    if (c%2==0) tgt++;
    tgt++;
    c++;
    };
c=0;
tgt=1;
while (c<totaldrums)
    {
    patchesordered[tgt]=tempdrums[c];
    tgt+=3;
    c++;
    };
// Now we put as not used (-1) the rest of the array
i=totalmelody+totaldrums;
while (i<256)
    {
    patchesordered[i]=-1;
    i++;
    };
};

char *gusOut::GUS_patches_directory="/etc";
int gusOut::delete_GUS_patches_directory = 0;
