/**************************************************************************

    midiout.cc   - class midiOut which handles external midi devices
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
#include "midiout.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "sndcard.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include "midispec.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/ioctl.h>

SEQ_USE_EXTBUF();

MidiOut::MidiOut(int d)
{
  seqfd = -1;
  devicetype=KMID_EXTERNAL_MIDI;
  device= d;
  count=0.0;
  lastcount=0.0;
  m_rate=100;
  convertrate=10;
  volumepercentage=100;
  map=new MidiMapper(NULL);
  if (map==NULL) { printfdebug("ERROR : midiOut : Map is NULL\n"); return; };
  _ok=1;
};

MidiOut::~MidiOut()
{
  delete map;
  closeDev();
}

void MidiOut::openDev (int sqfd)
{
#ifdef HAVE_OSS_SUPPORT
  _ok=1;
  seqfd=sqfd;
  if (seqfd==-1)
  {
    printfdebug("ERROR: Could not open /dev/sequencer\n");
    _ok=0;
    return;
  }

  ioctl(seqfd,SNDCTL_SEQ_NRSYNTHS,&ndevs);
  ioctl(seqfd,SNDCTL_SEQ_NRMIDIS,&nmidiports);
  m_rate=0;
  int r=ioctl(seqfd,SNDCTL_SEQ_CTRLRATE,&m_rate);
  if ((r==-1)||(m_rate<=0)) m_rate=HZ;

  midi_info midiinfo;
  midiinfo.device=device;
  convertrate=1000/m_rate;

#ifdef MIDIOUTDEBUG
  printfdebug("Number of synth devices : %d\n",ndevs);
  printfdebug("Number of midi ports : %d\n",nmidiports);
  printfdebug("Rate : %d\n",m_rate);

  int i;
  synth_info synthinfo;
  for (i=0;i<ndevs;i++)
  {
    synthinfo.device=i;
    if (ioctl(seqfd,SNDCTL_SYNTH_INFO,&synthinfo)!=-1)
    {
      printfdebug("----");
      printfdebug("Device : %d\n",i);
      printfdebug("Name : %s\n",synthinfo.name);
      switch (synthinfo.synth_type)
      {
	case (SYNTH_TYPE_FM) : printfdebug("FM\n");break;
	case (SYNTH_TYPE_SAMPLE) : printfdebug("Sample\n");break;
	case (SYNTH_TYPE_MIDI) : printfdebug("Midi\n");break;
	default : printfdebug("default type\n");break;
      }
      switch (synthinfo.synth_subtype)
      {
	case (FM_TYPE_ADLIB) : printfdebug("Adlib\n");break;
	case (FM_TYPE_OPL3) : printfdebug("Opl3\n");break;
	case (MIDI_TYPE_MPU401) : printfdebug("Mpu-401\n");break;
	case (SAMPLE_TYPE_GUS) : printfdebug("Gus\n");break;
	default : printfdebug("default subtype\n");break;
      }
    }
  }

  for (i=0;i<nmidiports;i++)
  {
    midiinfo.device=i;
    if (ioctl(seqfd,SNDCTL_MIDI_INFO,&midiinfo)!=-1)
    {
      printfdebug("----");
      printfdebug("Device : %d\n",i);
      printfdebug("Name : %s\n",midiinfo.name);
      printfdebug("Device type : %d\n",midiinfo.dev_type);
    }
  }

#endif

  count=0.0;
  lastcount=0.0;
  if (nmidiports<=0)
  {
    printfdebug("ERROR: There is no midi port !!\n");
    _ok=0;
    return;
  }
#endif

}

void MidiOut::closeDev (void)
{
  if (!ok()) return;
  SEQ_STOP_TIMER();
  SEQ_DUMPBUF();
//if (seqfd>=0) close(seqfd);
  seqfd=-1;
}

void MidiOut::initDev (void)
{
#ifdef HAVE_OSS_SUPPORT
  int chn;
  if (!ok()) return;
  count=0.0;
  lastcount=0.0;
  uchar gm_reset[5]={0x7e, 0x7f, 0x09, 0x01, 0xf7};
  sysex(gm_reset, sizeof(gm_reset));
  for (chn=0;chn<16;chn++)
  {
    chnmute[chn]=0;
    chnPatchChange(chn,0);
    chnPressure(chn,127);
    chnPitchBender(chn, 0x00, 0x40);
    chnController(chn, CTL_MAIN_VOLUME,110*volumepercentage);
    chnController(chn, CTL_EXT_EFF_DEPTH, 0);
    chnController(chn, CTL_CHORUS_DEPTH, 0);
    chnController(chn, 0x4a, 127);
  }
#endif
}

void MidiOut::setMidiMapper(MidiMapper *_map)
{
  delete map;
  map=_map;
}

void MidiOut::noteOn  (uchar chn, uchar note, uchar vel)
{
  if (vel==0)
  {
    noteOff(chn,note,vel);
  }
  else
  {
    SEQ_MIDIOUT(device, MIDI_NOTEON + map->channel(chn));
    SEQ_MIDIOUT(device, map->key(chn,chnpatch[chn],note));
    SEQ_MIDIOUT(device, vel);
  }
#ifdef MIDIOUTDEBUG
  printfdebug("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void MidiOut::noteOff (uchar chn, uchar note, uchar vel)
{
  SEQ_MIDIOUT(device, MIDI_NOTEOFF + map->channel(chn));
  SEQ_MIDIOUT(device, map->key(chn,chnpatch[chn],note));
  SEQ_MIDIOUT(device, vel);
#ifdef MIDIOUTDEBUG
  printfdebug("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void MidiOut::keyPressure (uchar chn, uchar note, uchar vel)
{
  SEQ_MIDIOUT(device, MIDI_KEY_PRESSURE + map->channel(chn));
  SEQ_MIDIOUT(device, map->key(chn,chnpatch[chn],note));
  SEQ_MIDIOUT(device, vel);
}

void MidiOut::chnPatchChange (uchar chn, uchar patch)
{
#ifdef MIDIOUTDEBUG
  printfdebug("PATCHCHANGE [%d->%d] %d -> %d\n",
      chn,map->channel(chn),patch,map->patch(chn,patch));
#endif
  SEQ_MIDIOUT(device, MIDI_PGM_CHANGE + map->channel(chn));
  SEQ_MIDIOUT(device, map->patch(chn,patch));
  chnpatch[chn]=patch;
}

void MidiOut::chnPressure (uchar chn, uchar vel)
{
  SEQ_MIDIOUT(device, MIDI_CHN_PRESSURE + map->channel(chn));
  SEQ_MIDIOUT(device, vel);

  chnpressure[chn]=vel;
}

void MidiOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
  SEQ_MIDIOUT(device, MIDI_PITCH_BEND + map->channel(chn));
  /*
#ifdef AT_HOME
  short pbs=((short)msb<<7) | (lsb & 0x7F);
  pbs=pbs-0x2000;
  short pbs2=(((long)pbs*672)/4096);
  printfdebug("Pitch Bender (%d): %d -> %d \n",chn,pbs,pbs2);
  pbs2=pbs2+0x2000;
  lsb=pbs2 & 0x7F;
  msb=(pbs2 >> 7)&0x7F;
#endif
   */
  map->pitchBender(chn,lsb,msb);
  SEQ_MIDIOUT(device, lsb);
  SEQ_MIDIOUT(device, msb);
  chnbender[chn]=(msb << 8) | (lsb & 0xFF);
}

void MidiOut::chnController (uchar chn, uchar ctl, uchar v)
{
  SEQ_MIDIOUT(device, MIDI_CTL_CHANGE + map->channel(chn));
#ifdef AT_HOME
  if (ctl==11) ctl=7;
#endif
  map->controller(chn,ctl,v);
  if ((ctl==11)||(ctl==7))
  {
    v=(v*volumepercentage)/100;
    if (v>127) v=127;
  }

  SEQ_MIDIOUT(device, ctl);
  SEQ_MIDIOUT(device, v);

  chncontroller[chn][ctl]=v;
}

void MidiOut::sysex(uchar *data, ulong size)
{
  ulong i=0;
  SEQ_MIDIOUT(device, MIDI_SYSTEM_PREFIX);
  while (i<size)
  {
    SEQ_MIDIOUT(device, *data);
    data++;
    i++;
  }
#ifdef MIDIOUTDEBUG
  printfdebug("sysex\n");
#endif
}

void MidiOut::channelSilence (uchar chn)
{
  uchar i;
  for ( i=0; i<127; i++)
  {
    noteOff(chn,i,0);
  };
  SEQ_DUMPBUF();
}

void MidiOut::channelMute(uchar chn, int a)
{
  if (a==1)
  {
    chnmute[chn]=a;
    channelSilence(chn);
  }
  else if (a==0)
  {
    chnmute[chn]=a;
  }
  /*  else ignore the call to this function */
}

void MidiOut::seqbuf_dump (void)
{
#ifdef HAVE_OSS_SUPPORT
  if (_seqbufptr)
    if (write (seqfd, _seqbuf, _seqbufptr) == -1)
    {
      printfdebug("Error writing to /dev/sequencer in MidiOut::seq_buf_dump\n");
      perror ("write /dev/sequencer in seqBufDump\n");
      exit (-1);
    }
  _seqbufptr = 0;
#endif
}

void MidiOut::seqbuf_clean(void)
{
#ifdef HAVE_OSS_SUPPORT
  _seqbufptr=0;
#endif
}

void MidiOut::wait(double ticks)
{
  SEQ_WAIT_TIME(((int)(ticks/convertrate)));
#ifdef MIDIOUTDEBUG
  printfdebug("Wait  >\t ticks: %g\n",ticks);
#endif
}

#ifdef MIDIOUTDEBUG
void MidiOut::tmrSetTempo(int v)
#else
void MidiOut::tmrSetTempo(int)
#endif
{
#ifdef MIDIOUTDEBUG
  printfdebug("SETTEMPO  >\t tempo: %d\n",v);
#endif

  //SEQ_SET_TEMPO(v);
  //SEQ_DUMPBUF();
}

void MidiOut::sync(int i)
{
#ifdef MIDIOUTDEBUG
  printfdebug("Sync %d\n",i);
#endif
#ifdef HAVE_OSS_SUPPORT
  if (i==1)
  {
    seqbuf_clean();
    /* If you have any problem, try removing the next 2 lines,
       I though they would be useful here, but I don't know
       what they exactly do :-) */
    ioctl(seqfd,SNDCTL_SEQ_RESET);
    ioctl(seqfd,SNDCTL_SEQ_PANIC);
  }
  ioctl(seqfd, SNDCTL_SEQ_SYNC);
#endif
}

void MidiOut::tmrStart(void)
{
  SEQ_START_TIMER();
  SEQ_DUMPBUF();
}

void MidiOut::tmrStop(void)
{
  SEQ_STOP_TIMER();
  SEQ_DUMPBUF();
}

void MidiOut::tmrContinue(void)
{
  SEQ_CONTINUE_TIMER();
  SEQ_DUMPBUF();
}

char *MidiOut::midiMapFilename(void)
{
  return (map!=NULL) ? map->filename() : (char *)"";
}

const char * MidiOut::deviceName(void) const
{
  switch (deviceType())
  {
    case (KMID_EXTERNAL_MIDI) : return "External Midi";
    case (KMID_SYNTH) : return "Synth";
    case (KMID_FM) : return "FM";
    case (KMID_GUS) : return "GUS";
    case (KMID_AWE) : return "AWE";
  }
  return "Unknown";
}

