/**************************************************************************

    midistat.cc	- class midiStat, change it internally and then send it. 
    Copyright (C) 1997  Antonio Larrosa Jimenez

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
#include "midistat.h"
#include "midiout.h"
#include <sys/soundcard.h>

midiStat::midiStat()
{
int i;
tempo=1000000;
for (int chn=0;chn<N_CHANNELS;chn++)
	{
	chn_patch[chn]=3;
	chn_bender[chn]=0x4000;
	chn_pressure[chn]=127;
	for (i=0;i<N_CTL;i++)
		chn_controller[chn][i]=0;
	chn_controller[chn][CTL_MAIN_VOLUME]=127;
	chn_controller[chn][0x4a]=127;
	};
};

midiStat::~midiStat()
{
};

//    void noteOn		( uchar chn, uchar note, uchar vel );
//    void noteOff	( uchar chn, uchar note, uchar vel );

void midiStat::chnPatchChange	( uchar chn, uchar patch )
{
chn_patch[chn]=patch;
//printf("Patch : %d %d\n",chn,patch);
};

void midiStat::chnPressure	( uchar chn, uchar vel )
{
chn_pressure[chn]=vel;
};

void midiStat::chnPitchBender	( uchar chn, uchar lsb,  uchar msb )
{
chn_bender[chn]=((int)msb<<8|lsb);
};

void midiStat::chnController	( uchar chn, uchar ctl , uchar v )
{
chn_controller[chn][ctl]=v;
};

void midiStat::tmrSetTempo(int v)
{
tempo=v;
};

void midiStat::sendData(midiOut *midi)
{
for (int chn=0;chn<N_CHANNELS;chn++)
	{
//	printf("Restoring channel %d\n",chn);
	midi->chnPatchChange(chn,chn_patch[chn]);
//        printf("Patch : %d %d\n",chn,chn_patch[chn]);
//	midi->chnPitchBender(chn,chn_bender[chn]&0xFF,chn_bender[chn]>>8);
//	midi->chnPressure(chn,chn_pressure[chn]);
/*	for (int i=0;i<N_CTL;i++)
		midi->chnController(chn,i,chn_controller[chn][i]);
*/	};
midi->tmrSetTempo(tempo);
midi->sync();
};
