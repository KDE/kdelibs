/**************************************************************************

    synthout.cc   - class synthOut which handles the /dev/sequencer device
			for synths (as AWE32)
    This file is part of LibKMid 0.9.5
    Copyright (C) 1997,98  Antonio Larrosa Jimenez and P.J.Leonard
                  1999,2000 Antonio Larrosa Jimenez
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/
#include "synthout.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "sndcard.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include "awe_sup.h"
#include "midispec.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

SEQ_USE_EXTBUF();

SynthOut::SynthOut(int d)
{
  seqfd = -1;
  devicetype=KMID_SYNTH;
  device= d;
  _ok=1;
}

SynthOut::~SynthOut()
{
  closeDev();
}

void SynthOut::openDev (int sqfd)
{
  _ok=1;
  seqfd = sqfd;
  if (seqfd==-1)
  {
    printfdebug("ERROR: Could not open /dev/sequencer\n");
    return;
  }
#ifdef HAVE_OSS_SUPPORT
  /*
     int i=1;
     ioctl(seqfd,SNDCTL_SEQ_THRESHOLD,i);
     printfdebug("Threshold : %d\n",i);
   */
#ifdef SYNTHOUTDEBUG
  printfdebug("Number of synth devices : %d\n",ndevs);
  printfdebug("Number of midi ports : %d\n",nmidiports);
  printfdebug("Rate : %d\n",m_rate);
#endif

#ifdef HAVE_AWE32

  struct synth_info info;

  // Should really collect the possible devices and let the user choose ?

  info.device = device;

  if (ioctl (seqfd, SNDCTL_SYNTH_INFO, &info) == -1)
    printfdebug(" ioctl  SNDCTL_SYNTH_INFO FAILED \n");

  if (info.synth_type == SYNTH_TYPE_SAMPLE
      && info.synth_subtype == SAMPLE_TYPE_AWE32)
  {

    // Enable layered patches ....
    AWE_SET_CHANNEL_MODE(device,1);
#ifdef SYNTHOUTDEBUG
    printfdebug(" Found AWE32 dev=%d \n",device);
#endif
  }
#endif // HAVE_AWE32
#endif // HAVE_OSS_SUPPORT

}

void SynthOut::closeDev (void)
{
  if (!ok()) return;
  //if (seqfd>=0) close(seqfd);
  seqfd=-1;
}

void SynthOut::initDev (void)
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
#endif
}

void SynthOut::noteOn  (uchar chn, uchar note, uchar vel)
{
  if (vel==0)
  {
    noteOff(chn,note,vel);
  }
  else
  {
    SEQ_START_NOTE(device, map->channel(chn),
	map->key(chn,chnpatch[chn],note),
	vel);
  }
#ifdef SYNTHOUTDEBUG
  printfdebug("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void SynthOut::noteOff (uchar chn, uchar note, uchar)
{
  SEQ_STOP_NOTE(device, map->channel(chn),
      map->key(chn,chnpatch[chn],note), 0);
#ifdef SYNTHOUTDEBUG
  printfdebug("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void SynthOut::keyPressure (uchar chn, uchar note, uchar vel)
{
  SEQ_KEY_PRESSURE(device, map->channel(chn), map->key(chn,chnpatch[chn],note),vel);
}

void SynthOut::chnPatchChange (uchar chn, uchar patch)
{
  SEQ_SET_PATCH(device,map->channel(chn),map->patch(chn,patch));
  chnpatch[chn]=patch;
}

void SynthOut::chnPressure (uchar chn, uchar vel)
{
  SEQ_CHN_PRESSURE(device, map->channel(chn) , vel);
  chnpressure[chn]=vel;
}

void SynthOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
  chnbender[chn]=((int)msb<<7) | (lsb & 0x7F);
  SEQ_BENDER(device, map->channel(chn), chnbender[chn]);
}

void SynthOut::chnController (uchar chn, uchar ctl, uchar v)
{
  if ((ctl==11)||(ctl==7))
  {
    v=(v*volumepercentage)/100;
    if (v>127) v=127;
  }

  SEQ_CONTROL(device, map->channel(chn), ctl, v);
  chncontroller[chn][ctl]=v;
}

void SynthOut::sysex(uchar *, ulong )
{
  // AWE32 doesn't respond to sysex (AFAIK)
/*
#ifndef HAVE_AWE32
  ulong i=0;
  SEQ_MIDIOUT(device, MIDI_SYSTEM_PREFIX);
  while (i<size)
  {
    SEQ_MIDIOUT(device, *data);
    data++;
    i++;
  };
  printfdebug("sysex\n");
#endif
*/
}
