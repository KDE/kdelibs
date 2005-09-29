/**************************************************************************

    midistat.cc	- class MidiStatus, change it internally and then send it. 
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/
#include "midistat.h"
#include "deviceman.h"
#include "sndcard.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

extern int MT32toGM[128];

MidiStatus::MidiStatus()
{
  int i;
  tempo=1000000;
  for (int chn=0;chn<16;chn++)
  {
    chn_patch[chn]=0;
    chn_bender[chn]=0x4000;
    chn_pressure[chn]=127;
    for (i=0;i<256;i++)
      chn_controller[chn][i]=0;
    chn_controller[chn][CTL_MAIN_VOLUME]=127;
    chn_controller[chn][11]=127;
    chn_controller[chn][0x4a]=127;
    chn_lastisvolumeev[chn]=1;
  }
}

MidiStatus::~MidiStatus()
{
}

//    void noteOn	( uchar chn, uchar note, uchar vel );
//    void noteOff	( uchar chn, uchar note, uchar vel );

void MidiStatus::chnPatchChange	( uchar chn, uchar patch )
{
  chn_patch[chn]=patch;
}

void MidiStatus::chnPressure	( uchar chn, uchar vel )
{
  chn_pressure[chn]=vel;
}

void MidiStatus::chnPitchBender	( uchar chn, uchar lsb,  uchar msb )
{
  chn_bender[chn]=((int)msb<<8|lsb);
}

void MidiStatus::chnController	( uchar chn, uchar ctl , uchar v )
{
  if (ctl==7) chn_lastisvolumeev[chn]=1;
  else if (ctl==11) chn_lastisvolumeev[chn]=0;

  chn_controller[chn][ctl]=v;
}

void MidiStatus::tmrSetTempo(int v)
{
  tempo=v;
}

void MidiStatus::sendData(DeviceManager *midi,int gm)
{
  for (int chn=0;chn<16;chn++)
  {
#ifdef MIDISTATDEBUG
    printf("Restoring channel %d\n",chn);
#endif
    midi->chnPatchChange(chn,
	(gm==1)?(chn_patch[chn]):(MT32toGM[chn_patch[chn]]));
    midi->chnPitchBender(chn,chn_bender[chn]&0xFF,chn_bender[chn]>>8);
    midi->chnPressure(chn,chn_pressure[chn]);
    if (chn_lastisvolumeev[chn])
    {
      midi->chnController(chn,11,chn_controller[chn][11]);
      midi->chnController(chn,CTL_MAIN_VOLUME,chn_controller[chn][CTL_MAIN_VOLUME]);
    } else {
      midi->chnController(chn,CTL_MAIN_VOLUME,chn_controller[chn][CTL_MAIN_VOLUME]);
      midi->chnController(chn,11,chn_controller[chn][11]);
    }
    /*
       for (int i=0;i<256;i++)
       midi->chnController(chn,i,chn_controller[chn][i]);
     */
  }
  midi->tmrSetTempo(tempo);
  midi->sync();
}
