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
#include "alsaout.h"

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
#endif
}

void MidiOut::closeDev (void)
{
  if (!ok()) return;
//  if (deviceType()!=KMID_ALSA) allNotesOff();
  SEQ_STOP_TIMER();
  SEQ_DUMPBUF();
  seqfd=-1;
}

void MidiOut::initDev (void)
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

void MidiOut::allNotesOff (void)
{
  for (int i=0; i<16; i++)
  {
    chnController(i, 0x78, 0);
    chnController(i, 0x79, 0);
  };
  sync(1);
}

void MidiOut::channelSilence (uchar chn)
{
  uchar i;
  for ( i=0; i<127; i++)
  {
    noteOff(chn,i,0);
  };
  sync();
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
  if (_seqbufptr && seqfd!=-1 && seqfd!=0)
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

const char *MidiOut::midiMapFilename(void)
{
  return (map!=NULL) ? map->filename() : "";
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
    case (KMID_ALSA) : return reinterpret_cast<const AlsaOut *>(this)->deviceName();
  }
  return "Unknown";
}

void MidiOut::sync(int i)
{
  if (deviceType()==KMID_ALSA) { // XXX : sync should be virtual after next bic
     reinterpret_cast<AlsaOut *>(this)->sync(i);
     return;
  }
  SEQ_DUMPBUF();
  printf("MidiOut::sync\n");
}
