/**************************************************************************

    gusout.cc  - class GUSOut which implements support for Gravis
         Ultrasound cards through a /dev/sequencer device
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
#include "gusout.h"
#include "sndcard.h"
#include "midispec.h"
#include "gusvoices.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

SEQ_USE_EXTBUF();

#ifdef HAVE_OSS_SUPPORT
struct pat_header
{
  char            magic[12];
  char            version[10];
  char            description[60];
  unsigned char   instruments;
  char            voices;
  char            channels;
  unsigned short  nr_waveforms;
  unsigned short  master_volume;
  unsigned long   data_size;
};
struct sample_header
{
  char            name[7];
  unsigned char   fractions;
  long            len;
  long            loop_start;
  long            loop_end;
  unsigned short  base_freq;
  long            low_note;
  long            high_note;
  long            base_note;
  short           detune;
  unsigned char   panning;

  unsigned char   envelope_rate[6];
  unsigned char   envelope_offset[6];

  unsigned char   tremolo_sweep;
  unsigned char   tremolo_rate;
  unsigned char   tremolo_depth;

  unsigned char   vibrato_sweep;
  unsigned char   vibrato_rate;
  unsigned char   vibrato_depth;

  char            modes;

  short           scale_frequency;
  unsigned short  scale_factor;
};

int get_dint(unsigned char *p)
{
  unsigned int v=0;

  for (int i=0;i<4;i++)
  {
    v |= (p[i] << (i*8));
  }
  return (int)v;
}

unsigned short get_word(unsigned char *p)
{
  unsigned short v=0;

  for (int i=0;i<2;i++)
    v |= (*p++ << (i*8));
  return (short)v;
}

#endif

GUSOut::GUSOut(int d,int total)
{
  seqfd = -1;
  devicetype=KMID_GUS;
  device= d;
  _ok=1;

  use8bit=0;
  nvoices=total;
  vm=new VoiceManager(nvoices);
}

GUSOut::~GUSOut()
{
  closeDev();
  
  delete vm;
  if (delete_GUS_patches_directory)
  {
    free((char *)GUS_patches_directory);
    delete_GUS_patches_directory = 0;
    GUS_patches_directory="/etc";
  }
}

void GUSOut::openDev (int sqfd)
{
  _ok=1;
  seqfd = sqfd;
  //vm->clearLists();
  if (seqfd==-1)
  {
    printfdebug("ERROR: Could not open /dev/sequencer\n");
    return;
  }

#ifdef HAVE_OSS_SUPPORT

  //seqbuf_clean();
  //ioctl(seqfd,SNDCTL_SEQ_RESET);
  //ioctl(seqfd,SNDCTL_SEQ_PANIC);

  if (ioctl(seqfd, SNDCTL_SEQ_RESETSAMPLES, &device)==-1)
  {
    printfdebug("Error reseting gus samples. Please report\n");
  };
  use8bit=0;
  totalmemory = device;
  ioctl(seqfd, SNDCTL_SYNTH_MEMAVL, &totalmemory);
  freememory = device;
  ioctl(seqfd, SNDCTL_SYNTH_MEMAVL, &freememory);

#endif


}

void GUSOut::closeDev (void)
{
  if (!ok()) return;
  vm->clearLists();
  //if (seqfd>=0)
  //    close(seqfd);
  seqfd=-1;
}

void GUSOut::initDev (void)
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
    //        chnPressure(chn,127);
    chnPitchBender(chn, 0x00, 0x40);
    chnController(chn, CTL_MAIN_VOLUME,127);
    chnController(chn, CTL_EXT_EFF_DEPTH, 0);
    chnController(chn, CTL_CHORUS_DEPTH, 0);
    chnController(chn, 0x4a, 127);
  }


  for (int i = 0; i < nvoices; i++)
  {
    SEQ_CONTROL(device, i, SEQ_VOLMODE, VOL_METHOD_LINEAR);
    SEQ_STOP_NOTE(device, i, vm->note(i), 64);
  }

#endif
}


int GUSOut::patch(int p)
{
  if (patchloaded[p]==1) return p;
  printfdebug("Not loaded %d!\n",p);
  p=0;
  while ((p<256)&&(patchloaded[p]==0)) p++;
  return p;
}

void GUSOut::noteOn  (uchar chn, uchar note, uchar vel)
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
    };
    int v=vm->allocateVoice(chn,note);
    int p;
    if (chn==PERCUSSION_CHANNEL)
      SEQ_SET_PATCH(device,v ,p=patch(note+128))
    else
      SEQ_SET_PATCH(device,v ,p=map->patch(chn,chnpatch[chn]));
    SEQ_BENDER(device, v, chnbender[chn]);

    SEQ_START_NOTE(device, v, note, vel);
    //        SEQ_CONTROL(device, v, CTL_MAIN_VOLUME, chncontroller[chn][CTL_MAIN_VOLUME]);
    SEQ_CHN_PRESSURE(device, v , chnpressure[chn]);
  }

  printfdebug("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
}

void GUSOut::noteOff (uchar chn, uchar note, uchar vel)
{
  int i;
  vm->initSearch();
  while ((i=vm->search(chn,note))!=-1)
  {
    SEQ_STOP_NOTE(device, i, note, vel);
    vm->deallocateVoice(i);
  }

#ifdef GUSOUTDEBUG
  printf("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void GUSOut::keyPressure (uchar chn, uchar note, uchar vel)
{
  int i;
  vm->initSearch();
  while ((i=vm->search(chn,note))!=-1)
    SEQ_KEY_PRESSURE(device, i, note,vel);
}

void GUSOut::chnPatchChange (uchar chn, uchar patch)
{
  if (chn==PERCUSSION_CHANNEL) return;
  int i;
  vm->initSearch();
  while ((i=vm->search(chn))!=-1)
    SEQ_SET_PATCH(device,i,map->patch(chn,patch));
  chnpatch[chn]=patch;

}

void GUSOut::chnPressure (uchar /*chn*/, uchar /*vel*/)
{
  /*    int i;
	vm->initSearch();
	while ((i=vm->search(chn))!=-1)
	SEQ_CHN_PRESSURE(device, i , vel);
	chnpressure[chn]=vel;
   */
}

void GUSOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
  chnbender[chn]=((int)msb<<7) | (lsb & 0x7F);

  int i;
  vm->initSearch();
  while ((i=vm->search(chn))!=-1)
    SEQ_BENDER(device, i, chnbender[chn]);
}

void GUSOut::chnController (uchar chn, uchar ctl, uchar v)
{
  if ((ctl==11)||(ctl==7))
  {
    v=(v*volumepercentage)/100;
    if (v>127) v=127;
  };

  int i;
  vm->initSearch();
  while ((i=vm->search(chn))!=-1)
    SEQ_CONTROL(device, i, ctl, v);

  chncontroller[chn][ctl]=v;
}

void GUSOut::sysex(uchar *, ulong )
{

}

void GUSOut::setGUSPatchesDirectory(const char *dir)
{
    if ((dir==NULL)||(dir[0]==0)) return;
    if (delete_GUS_patches_directory)
        free((char *)GUS_patches_directory);

    GUS_patches_directory = strdup(dir);
    delete_GUS_patches_directory=1;
}

char *GUSOut::patchName(int pgm)
{
  return GUS_voice_names[pgm];
}


int GUSOut::loadPatch(int pgm)
{
#ifdef HAVE_OSS_SUPPORT
  struct pat_header header;
  struct sample_header sample;
  if (patchloaded[pgm]==1)
  {
#ifdef GUSOUTDEBUG
    printf("Trying to reload a patch. This should never happen, please report.\n");
#endif
    return 0;
  }
  if ((patchName(pgm)==NULL)||((patchName(pgm))[0]==0))
  {
#ifdef GUSOUTDEBUG
    printf("Couldn't guess patch name for patch number %d\n",pgm);
#endif
    return -1;
  }
  char *s=new char[strlen(GUS_patches_directory)+strlen(patchName(pgm))+10];
  if (s==NULL) return -1;
  sprintf(s,"%s/%s.pat",GUS_patches_directory,patchName(pgm));
#ifdef GUSOUTDEBUG
  printf("Loading patch : %s\n",s);
#endif
  struct patch_info *patch=NULL;
  struct stat info;
  if (stat(s, &info)==-1)
  {
#ifdef GUSOUTDEBUG
    printf("File %s doesn't exist\n",s);
#endif
    return -1;
  }

  FILE *fh=fopen(s,"rb");
  if (fh==NULL)
  {
#ifdef GUSOUTDEBUG
    printf("Couldn't open patch %s\n",s);
#endif
    return -1;
  }

  unsigned char tmp[256];
  if (fread(tmp,1,0xef,fh)!=0xef)
  {
    fclose(fh);
#ifdef GUSOUTDEBUG
    printf("Short file ! \n");
#endif
    return -1;
  }
  memcpy ((char *) &header, tmp, sizeof (header));

  if (strncmp(header.magic,"GF1PATCH110",12)!=0)
  {
#ifdef GUSOUTDEBUG
    printf("File %s is corrupted or it isn't a patch file\n",s);
#endif
    return -1;
  }
  if (strncmp(header.version,"ID#000002",10)!=0)
  {
#ifdef GUSOUTDEBUG
    printf("File %s's version is not supported\n",s);
#endif
    return -1;
  }
  unsigned short nWaves= *(unsigned short *)&tmp[85];
#ifdef GUSOUTDEBUG
  unsigned short masterVolume= *(unsigned short *)&tmp[87];
  printf("nWaves: %d\n",nWaves);
  printf("masterVolume : %d\n",masterVolume);
#endif

  unsigned short i;
  int offset=0xef;
  for (i=0;i<nWaves;i++)
  {
    fseek(fh,offset,SEEK_SET);

    if (fread(tmp,1,sizeof(sample),fh) != sizeof(sample))
    {
      fclose(fh);
#ifdef GUSOUTDEBUG
      printf("Short file\n");
#endif
      return -1;
    }
    memcpy ((char *) &sample, tmp, sizeof (sample));
    sample.fractions = (char)tmp[7];
    sample.len = get_dint(&tmp[8]);
    sample.loop_start = get_dint(&tmp[12]);
    sample.loop_end = get_dint(&tmp[16]);
    sample.base_freq = get_word(&tmp[20]);
    sample.low_note = get_dint(&tmp[22]);
    sample.high_note = get_dint(&tmp[26]);
    sample.base_note = get_dint(&tmp[30]);
    sample.detune = (short)get_word(&tmp[34]);
    sample.panning = (unsigned char) tmp[36];

    memcpy (sample.envelope_rate, &tmp[37], 6);
    memcpy (sample.envelope_offset, &tmp[43], 6);

    sample.tremolo_sweep = (unsigned char) tmp[49];
    sample.tremolo_rate = (unsigned char) tmp[50];
    sample.tremolo_depth = (unsigned char) tmp[51];

    sample.vibrato_sweep = (unsigned char) tmp[52];
    sample.vibrato_rate = (unsigned char) tmp[53];
    sample.vibrato_depth = (unsigned char) tmp[54];
    sample.modes = (unsigned char) tmp[55];
    sample.scale_frequency = (short)get_word(&tmp[56]);
    sample.scale_factor = get_word(&tmp[58]);

    offset = offset + 96;

    patch = (struct patch_info *) malloc(sizeof (*patch) + sample.len);
    if (patch == NULL)
    {
#ifdef GUSOUTDEBUG
      printf("Not enough memory\n");
#endif
      return -1;
    }
    patch->key = GUS_PATCH;
    patch->device_no = device;
    patch->instr_no = pgm;
    patch->mode = sample.modes | WAVE_TREMOLO | WAVE_VIBRATO | WAVE_SCALE;
    patch->len = sample.len;
    patch->loop_start = sample.loop_start;
    patch->loop_end = sample.loop_end;
    patch->base_note = sample.base_note;
    patch->high_note = sample.high_note;
    patch->low_note = sample.low_note;
    patch->base_freq = sample.base_freq;
    patch->detuning = sample.detune;
    patch->panning = (sample.panning - 7) * 16;

    memcpy (patch->env_rate, sample.envelope_rate, 6);
    memcpy (patch->env_offset, sample.envelope_offset, 6);

    patch->tremolo_sweep = sample.tremolo_sweep;
    patch->tremolo_rate = sample.tremolo_rate;
    patch->tremolo_depth = sample.tremolo_depth;

    patch->vibrato_sweep = sample.vibrato_sweep;
    patch->vibrato_rate = sample.vibrato_rate;
    patch->vibrato_depth = sample.vibrato_depth;

    patch->scale_frequency = sample.scale_frequency;
    patch->scale_factor = sample.scale_factor;

    patch->volume = header.master_volume;

    if (fseek (fh, offset, 0) == -1)
    {
      fclose(fh);
      return -1;
    }

    if ((long)fread (patch->data, 1,sample.len,fh) != sample.len)
    {
#ifdef GUSOUTDEBUG
      printf ("Short file\n");
#endif
      return -1;
    }

    SEQ_WRPATCH (patch, sizeof (*patch) + sample.len);

    offset = offset + sample.len;

  }
  patchloaded[pgm]=1;

  fclose(fh);
  free(patch); // Shouldn't this 'free' be within the 'for' loop ?
  delete s;
  freememory = device;
  ioctl(seqfd, SNDCTL_SYNTH_MEMAVL, &freememory);
#endif
  return 0;
}


void GUSOut::setPatchesToUse(int *patchesused)
{
#ifdef HAVE_OSS_SUPPORT
  int k;
  for (k=0;k<256;k++) patchloaded[k]=0;

  int patchesordered[256]; //This holds the pgm used ordered by a method which
  // put first the patches more oftenly used, and then the least
  // In example, if a song only uses a piano and a splash cymbal,
  // This is set to : 0,188,-1,-1,-1,-1 ...
  patchesLoadingOrder(patchesused,patchesordered);

  // If above line doesn't work, perhaps you could try this ? :
  // for (int j=0;j<256;j++) patchesordered[j]=patchesused[j];
#ifdef GUSOUTDEBUG
  printf("Patches used : \n");
  for (k=0;k<256;k++)
  {
    if (patchesused[k]!=-1) printf("%d,",patchesused[k]);
  }
  printf("\n Patches used, sorted :\n");
  for (k=0;k<256;k++)
  {
    if (patchesordered[k]!=-1) printf("%d,",patchesordered[k]);
  }
#endif

  int i=0;
  while (patchesordered[i]!=-1)
  {
#ifdef GUSOUTDEBUG
    printf("Load Patch : %d\n",patchesordered[i]);
#endif
    loadPatch(patchesordered[i]);
    i++;
  }
#endif
}

int compare_decreasing(const void *a,const void *b)
{
  struct instr_gm
  {
    int used;
    int pgm;
  };
  instr_gm *ai=(instr_gm *)a;
  instr_gm *bi=(instr_gm *)b;
  return ai->used<bi->used;
}


void GUSOut::patchesLoadingOrder(int *patchesused,int *patchesordered)
{
  struct instr_gm
  {
    int used;
    int pgm;
  };

  instr_gm tempmelody[128];
  instr_gm tempdrums[128];
  int i,j;
  for (i=0,j=128;i<128;i++,j++)
  {
    tempmelody[i].used=patchesused[i];
    tempmelody[i].pgm=i;
    tempdrums[i].used=patchesused[j];
    tempdrums[i].pgm=j;
  }
  /* SORT */ // Decreasing order (first most used patch, then less used patch)
  qsort(&tempmelody[0],128,sizeof(instr_gm),compare_decreasing);
  qsort(&tempdrums[0],128,sizeof(instr_gm),compare_decreasing);

  /* Once they are sorted, the result is put on patchesordered in the following
   * way : If tempmelody is : M0 M1 M2 M3 ... M127 and tempdrums is :
   * D0 D1 D2 D3 ... D127, the result is :
   * M0 D0 M1 M2 D1 M3 M4 D2 M5 M6 D3  ...
   * P0 P1 P2 P3 P4 P5 P6 P7 P8 P9 P10 ...
   */

#ifdef GUSOUTDEBUG
  for (int k=0;k<128;k++)
  {
    printf("%d - %d\n",tempmelody[k].used,tempmelody[k].pgm);
  }
  for (int k=0;k<128;k++)
  {
    printf("%d : %d\n",tempdrums[k].used,tempdrums[k].pgm);
  }
#endif

  i=0;
  int totalmelody=0;
  while ((i<128)&&(tempmelody[i].used!=0))
  {
    totalmelody++;
    i++;
  }
  i=0;
  int totaldrums=0;
  while ((i<128)&&(tempdrums[i].used!=0))
  {
    totaldrums++;
    i++;
  }
#ifdef GUSOUTDEBUG
  printf("Totalmelody : %d,totaldrums : %d\n",totalmelody,totaldrums);
#endif
  int tgt=0;

  int tm=totalmelody;
  int td=totaldrums;
  int cm,cd;
  cm=cd=0;
  if ((tm!=0)&&(td!=0))
  {
    patchesordered[0]=tempmelody[0].pgm;
    patchesordered[1]=tempdrums[0].pgm;
    tm--;td--;
    cm++;cd++;
    tgt+=2;
    while ((tm>0)&&(td>0))
    {
      if (((tgt-1)%3)==0)
      {
	patchesordered[tgt]=tempdrums[cd].pgm;
	cd++;
	td--;
      }
      else
      {
	patchesordered[tgt]=tempmelody[cm].pgm;
	cm++;
	tm--;
      }
      tgt++;	
    }
  }
  while (tm>0)
  {
    patchesordered[tgt]=tempmelody[cm].pgm;
    tgt++;
    cm++;
    tm--;
  }
  while (td>0)
  {
    patchesordered[tgt]=tempdrums[cd].pgm;
    tgt++;
    cd++;
    td--;
  }

  // Now we put as not used (-1) the rest of the array
  while (tgt<256)
  {
    patchesordered[tgt]=-1;
    tgt++;
  }
}

//char *GUSOut::GUS_patches_directory="/mnt/dosc/gravis/patches";
const char *GUSOut::GUS_patches_directory="/usr/share/ultrasnd";

int GUSOut::delete_GUS_patches_directory = 0;
/* No, this doesn't delete any file :-) it's just for internal use */
