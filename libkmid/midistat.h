/**************************************************************************

    midistat.h	- class midiStat, change it internally and then send it. 
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
#ifndef _MIDISTAT_H
#define _MIDISTAT_H

#include "dattypes.h"

#ifndef N_CHANNELS
#define N_CHANNELS 16
#endif

#ifndef N_CTL
#define N_CTL    256
#endif


class DeviceManager;
class midiStat
{
private:
    ulong		tempo;

    unsigned char	chn_patch	[N_CHANNELS];
    int          	chn_bender	[N_CHANNELS];
    unsigned char	chn_pressure	[N_CHANNELS];
    unsigned char	chn_controller	[N_CHANNELS][N_CTL];

    int                 chn_lastisvolumeev [N_CHANNELS];
    
public:
    midiStat();
    ~midiStat();

//    void noteOn		( uchar chn, uchar note, uchar vel );
//    void noteOff	( uchar chn, uchar note, uchar vel );
    void keyPressure	( uchar chn, uchar note, uchar vel );
    void chnPatchChange	( uchar chn, uchar patch );
    uchar chnPatch	( uchar chn ) { return chn_patch[chn]; };

    void chnPressure	( uchar chn, uchar vel );
    uchar chnPressure	( uchar chn ) {return chn_pressure[chn];};
    void chnPitchBender	( uchar chn, uchar lsb,  uchar msb );
    int chnPitchBender	( uchar chn) {return chn_bender[chn];};
    void chnController	( uchar chn, uchar ctl , uchar v ); 
    uchar chnController	( uchar chn, uchar ctl ) {return chn_controller[chn][ctl];}; 

    void sysex		( uchar *data,ulong size);

    void tmrSetTempo(int v);

    void sendData(DeviceManager *midi,int gm=1);
};

#endif
