/**************************************************************************

    synthout.cc   - class synthOut which handles the /dev/sequencer device
			for synths (as AWE32)
    Copyright (C) 1997,98  Antonio Larrosa Jimenez
			P.J.Leonard (P.J.Leonard@bath.ac.uk)

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
#include "../version.h"

SEQ_USE_EXTBUF();

synthOut::synthOut(int d)
{
    seqfd = -1;
    devicetype=KMID_SYNTH;
    device= d;
#ifdef HANDLETIMEINDEVICES
    count=0.0;
    lastcount=0.0;
    rate=100;
#endif
    ok=1;
}

synthOut::~synthOut()
{
    delete Map;
    closeDev();
}

void synthOut::openDev (int sqfd)
{
    ok=1;
    seqfd = sqfd;
    if (seqfd==-1)
    {
        printf("ERROR: Could not open /dev/sequencer\n");
        return;
    }
#ifdef HANDLETIMEINDEVICES
    ioctl(seqfd,SNDCTL_SEQ_NRSYNTHS,&ndevs);
    ioctl(seqfd,SNDCTL_SEQ_NRMIDIS,&nmidiports);
    rate=0;
    int r=ioctl(seqfd,SNDCTL_SEQ_CTRLRATE,&rate);
    if ((r==-1)||(rate<=0)) rate=HZ;
    convertrate=1000/rate;
    /*int i=1;
     ioctl(seqfd,SNDCTL_SEQ_THRESHOLD,i);
     printf("Threshold : %d\n",i);
     */
#ifdef SYNTHOUTDEBUG
    printf("Number of synth devices : %d\n",ndevs);
    printf("Number of midi ports : %d\n",nmidiports);
    printf("Rate : %d\n",rate);
#endif
    
    count=0.0;
    lastcount=0.0;
#endif
    
#ifdef HAVE_AWE32  
    
    struct synth_info info;
    
    // Should really collect the possible devices and let the user choose ?
    
    info.device = device;
    
    if (ioctl (seqfd, SNDCTL_SYNTH_INFO, &info) == -1) {
        printf(" ioctl  SNDCTL_SYNTH_INFO FAILED \n");
    }
    if (info.synth_type == SYNTH_TYPE_SAMPLE
        && info.synth_subtype == SAMPLE_TYPE_AWE32) {
        
        // Enable layered patches ....
        AWE_SET_CHANNEL_MODE(device,1);
#ifdef SYNTHOUTDEBUG
        printf(" Found AWE32 dev=%d \n",device);
#endif
    }
#endif
    
}

void synthOut::closeDev (void)
{
    if (!OK()) return;
#ifdef HANDLETIMEINDEVICES
    SEQ_STOP_TIMER();
    SEQ_DUMPBUF();
#endif
    //if (seqfd>=0)
    //    close(seqfd);
    seqfd=-1;
}

void synthOut::initDev (void)
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
        
    }
}

void synthOut::noteOn  (uchar chn, uchar note, uchar vel)
{
    if (vel==0)
    {
        noteOff(chn,note,vel);
    }
    else
    {
        SEQ_START_NOTE(device, Map->Channel(chn),
                       Map->Key(chn,chn_patch[chn],note),
                       vel);
    }
#ifdef SYNTHOUTDEBUG
    printf("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void synthOut::noteOff (uchar chn, uchar note, uchar vel)
{
    SEQ_STOP_NOTE(device, Map->Channel(chn),
                  Map->Key(chn,chn_patch[chn],note), 0);
#ifdef SYNTHOUTDEBUG
    printf("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void synthOut::keyPressure (uchar chn, uchar note, uchar vel)
{
    // Hmmm is this implemented in /dev/sequencer ?
    // Yes, it is.
    SEQ_KEY_PRESSURE(device, Map->Channel(chn), Map->Key(chn,chn_patch[chn],note),vel);
}

void synthOut::chnPatchChange (uchar chn, uchar patch)
{
    
    SEQ_SET_PATCH(device,Map->Channel(chn),Map->Patch(chn,patch)); 
    chn_patch[chn]=patch;
}

void synthOut::chnPressure (uchar chn, uchar vel)
{
    
    SEQ_CHN_PRESSURE(device, Map->Channel(chn) , vel);
    chn_pressure[chn]=vel;
}

void synthOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
    chn_bender[chn]=((int)msb<<7) | (lsb & 0x7F);
    SEQ_BENDER(device, Map->Channel(chn), chn_bender[chn]);
}

void synthOut::chnController (uchar chn, uchar ctl, uchar v) 
{
    if ((ctl==11)||(ctl==7))
    {
        v=(v*volumepercentage)/100;
        if (v>127) v=127;
    }
    
    SEQ_CONTROL(device, Map->Channel(chn), ctl, v);
    chn_controller[chn][ctl]=v;
}

void synthOut::sysex(uchar *, ulong )
{
    // AWE32 doesn't respond to sysex (IFAIK)
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
    printf("sysex\n");
#endif
    */
}
