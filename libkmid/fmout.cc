/**************************************************************************

    fmout.cc   - class fmOut which handles the /dev/sequencer device
			for fm synths
    This file is part of LibKMid 0.9.5
    Copyright (C) 1998,99,2000  Antonio Larrosa Jimenez
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
#include "fmout.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "sndcard.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <limits.h>
#include "midispec.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

SEQ_USE_EXTBUF();

FMOut::FMOut( int d, int total )
{
  seqfd = -1;
  devicetype = KMID_FM;
  device = d;
  _ok = 1;
  // Put opl=3 for opl/3 (better quality/ 6 voices)
  //  or opl=2 for fm output (less quality/ 18 voices, which is better imho) :
  opl = 2;
  // But be aware that opl=3 is not intended to be fully supported by now

  nvoices = total;
  vm = new VoiceManager (nvoices);
}

FMOut::~FMOut()
{
  closeDev();
  delete vm;
  if (deleteFMPatchesDirectory)
  {
    delete FMPatchesDirectory;
    deleteFMPatchesDirectory = 0;
    FMPatchesDirectory="/etc";
  }
}

void FMOut::openDev (int sqfd)
{
#ifdef HAVE_OSS_SUPPORT
  _ok=1;
  seqfd = sqfd;
  //vm->clearLists();
  if ( seqfd == -1 )
  {
    printfdebug("ERROR: Could not open /dev/sequencer\n");
    return;
  }

  loadFMPatches();
#endif

}

void FMOut::closeDev (void)
{
  if (!ok()) return;
  vm->clearLists();
  //if (seqfd>=0) close(seqfd);
  seqfd = -1;
}

void FMOut::initDev (void)
{
#ifdef HAVE_OSS_SUPPORT
  int chn;
  if (!ok()) return;
  uchar gm_reset[5]={0x7e, 0x7f, 0x09, 0x01, 0xf7};
  sysex(gm_reset, sizeof(gm_reset));
  for (chn=0;chn<16;chn++)
  {
    chnmute[chn]=0;
    chnPatchChange(chn,0);
    chnPressure(chn,127);
    chnPitchBender(chn, 0x00, 0x40);
    chnController(chn, CTL_MAIN_VOLUME,127);
    chnController(chn, CTL_EXT_EFF_DEPTH, 0);
    chnController(chn, CTL_CHORUS_DEPTH, 0);
    chnController(chn, 0x4a, 127);
  }

  if (opl==3) ioctl(seqfd, SNDCTL_FM_4OP_ENABLE, &device);
  SEQ_VOLUME_MODE(device,VOL_METHOD_LINEAR);

  for (int i = 0; i < nvoices; i++)
  {
    SEQ_CONTROL(device, i, SEQ_VOLMODE, VOL_METHOD_LINEAR);
    SEQ_STOP_NOTE(device, i, vm->note(i), 64);
  }
#endif
}

void FMOut::loadFMPatches(void)
{
#ifdef HAVE_OSS_SUPPORT
  char patchesfile[PATH_MAX];
  char drumsfile[PATH_MAX];
  int size;
  struct sbi_instrument instr;
  char tmp[60];
  int i,j;
  for ( i=0; i<256; i++ )
    patchloaded[i] = 0;
  int stereoeffect=rand()%3;
  FILE *fh;
  int datasize;

  if (opl==3)
  {
    snprintf(patchesfile, PATH_MAX, "%s/std.o3",FMPatchesDirectory);
    size=60;
  }
  else
  {
    snprintf(patchesfile, PATH_MAX, "%s/std.sb",FMPatchesDirectory);
    size=52;
  }
  fh=fopen(patchesfile,"rb");
  if (fh==NULL) return;

  for (i=0;i<128;i++)
  {
    fread(tmp,size,1,fh);
    patchloaded[i]=1;
    instr.key = ((strncmp(tmp, "4OP", 3) == 0))? OPL3_PATCH : FM_PATCH;
    datasize = (strncmp(tmp, "4OP", 3) == 0)? 22 : 11;
    instr.device=device;
    instr.channel = i;
    // Let's get some stereo effect ...
    tmp[46] = (tmp[46] & 0xcf) | ((++stereoeffect)<<4);
    stereoeffect=stereoeffect%3;
    for (j=0; j<22; j++)
      instr.operators[j] = tmp[j+36];
    SEQ_WRPATCH(&instr,sizeof(instr));
  }
  fclose(fh);

  if (opl==3)
  {
    snprintf(drumsfile, PATH_MAX, "%s/drums.o3",FMPatchesDirectory);
  }
  else
  {
    snprintf(drumsfile, PATH_MAX, "%s/drums.sb",FMPatchesDirectory);
  }

  fh=fopen(drumsfile,"rb");
  if (fh==NULL) return;

  for (i=128;i<175;i++)
  {
    fread(tmp,size,1,fh);
    patchloaded[i]=1;
    instr.key = (strncmp(tmp, "4OP", 3) == 0)? OPL3_PATCH : FM_PATCH;
    datasize = (strncmp(tmp, "4OP", 3) == 0)? 22 : 11;
    instr.device=device;
    instr.channel = i;
    // Let's get some stereo effect ...
    tmp[46] = (tmp[46] & 0xcf) | ((++stereoeffect)<<4);
    stereoeffect=stereoeffect%3;
    for (j=0; j<22; j++)
      instr.operators[j] = tmp[j+36];
    SEQ_WRPATCH(&instr,sizeof(instr));
  }
  fclose(fh);

#ifdef FMOUTDEBUG
  printfdebug("Patches loaded\n");
#endif
#endif
}

int FMOut::patch(int p)
{
  if (patchloaded[p]==1) return p;
#ifdef FMOUTDEBUG
  printfdebug("Not loaded %d!\n",p);
#endif
  p=0;
  while ((p<256)&&(patchloaded[p]==0)) p++;
  return p;
}

void FMOut::noteOn  (uchar chn, uchar note, uchar vel)
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
	if (patchloaded[chnpatch[chn]]==0) return;
    }
    int v=vm->allocateVoice(chn,note);
    int p;
    if (chn==PERCUSSION_CHANNEL)
      SEQ_SET_PATCH(device,v ,p=patch(note+128))
    else
      SEQ_SET_PATCH(device,v ,p=map->patch(chn,chnpatch[chn]));
    SEQ_BENDER(device, v, chnbender[chn]);

    SEQ_START_NOTE(device, v, note, vel);
    //    SEQ_CONTROL(device, v, CTL_MAIN_VOLUME, chncontroller[chn][CTL_MAIN_VOLUME]);

    SEQ_CHN_PRESSURE(device, v , chnpressure[chn]);
  }

#ifdef FMOUTDEBUG
  printfdebug("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void FMOut::noteOff (uchar chn, uchar note, uchar vel)
{
  int i;
  vm->initSearch();
  while ((i=vm->search(chn,note))!=-1)
  {
    SEQ_STOP_NOTE(device, i, note, vel);
    vm->deallocateVoice(i);
  }

#ifdef FMOUTDEBUG
  printfdebug("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void FMOut::keyPressure (uchar chn, uchar note, uchar vel)
{
  int i;
  vm->initSearch();
  while ((i=vm->search(chn,note))!=-1)
    SEQ_KEY_PRESSURE(device, i, note,vel);
}

void FMOut::chnPatchChange (uchar chn, uchar patch)
{
  if (chn==PERCUSSION_CHANNEL) return;
  int i;
  vm->initSearch();
  while ((i=vm->search(chn))!=-1)
    SEQ_SET_PATCH(device,i,map->patch(chn,patch));

  chnpatch[chn]=patch;
}

void FMOut::chnPressure (uchar chn, uchar vel)
{
  int i;
  vm->initSearch();
  while ((i=vm->search(chn))!=-1)
    SEQ_CHN_PRESSURE(device, i , vel);

  chnpressure[chn]=vel;
}

void FMOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
  chnbender[chn]=((int)msb<<7) | (lsb & 0x7F);

  int i;
  vm->initSearch();
  while ((i=vm->search(chn))!=-1)
    SEQ_BENDER(device, i, chnbender[chn]);

}

void FMOut::chnController (uchar chn, uchar ctl, uchar v)
{
  if ((ctl==11)||(ctl==7))
  {
    v=(v*volumepercentage)/100;
    if (v>127) v=127;
  }
  int i;
  vm->initSearch();
  while ((i=vm->search(chn))!=-1)
    SEQ_CONTROL(device, i, ctl, v);

  chncontroller[chn][ctl]=v;
}

void FMOut::sysex(uchar *, ulong )
{

}

void FMOut::setFMPatchesDirectory(const char *dir)
{
  if ((dir==NULL)||(dir[0]==0)) return;
  if (deleteFMPatchesDirectory) delete FMPatchesDirectory;
  char *FMPatchesDirectory2=new char[strlen(dir)+1];
  strcpy(FMPatchesDirectory2,dir);
  FMPatchesDirectory = FMPatchesDirectory2;
  deleteFMPatchesDirectory=1;
}

void FMOut::setVolumePercentage    ( int i )
{
#ifdef HAVE_OSS_SUPPORT
  int fd=open("/dev/mixer0",O_RDWR,0);
  if (fd==-1) return;
  int a=i*255/100;
  if (a>255) a=255;
  a=(a<<8) | a;
  if (ioctl(fd,MIXER_WRITE(SOUND_MIXER_SYNTH),&a) == -1)
    printfdebug("ERROR writing to mixer\n");
  close(fd);
#endif
  volumepercentage=i;
}


const char *FMOut::FMPatchesDirectory = "/etc";
int FMOut::deleteFMPatchesDirectory = 0;
