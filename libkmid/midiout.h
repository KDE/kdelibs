/**************************************************************************

    midiout.h	- class midiOut which handles the /dev/sequencer device
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
#ifndef _MIDIOUT_H
#define _MIDIOUT_H

#include <stdio.h>
#include "dattypes.h"
#include "midimapper.h"
#include "../version.h"

#define N_CHANNELS 16
#define N_CTL    256

class midiOut
{
protected:
friend class DeviceManager; 
    int                 seqfd; // Is the file handler, but MUST NOT be closed
    int			device;
#ifdef HANDLETIMEINDEVICES
    int			ndevs; // Total number of devices
    int			nmidiports; // Total number of midi ports

    double		count;
    double		lastcount;
    double		lasttime;
    double		begintime;
    
    int			rate;
    double		convertrate; // A "constant" used to convert from
				// milliseconds to the computer rate
#endif
    int        devicetype; //As this class is inherited by many other
                  // classes, to support other cards, this varialbe
                  // holds the type of card, so that polymorphism is
                  // better used.
                  // The values it can get are defined as KMID_... in midispec.h

    MidiMapper          *Map;

    unsigned char	chn_patch	[N_CHANNELS];
    int          	chn_bender	[N_CHANNELS];
    unsigned char	chn_pressure	[N_CHANNELS];
    unsigned char	chn_controller	[N_CHANNELS][N_CTL];

    int			chn_mute        [N_CHANNELS];

    int			ok;

    void seqbuf_dump (void);
    void seqbuf_clean(void);
public:
    midiOut(int d=0);
virtual   ~midiOut();


virtual    void openDev		(int sqfd);
virtual    void closeDev	(void);
virtual    void initDev		(void);
	int 	devType		(void) {return devicetype;};
	char *	devName		(void);
#ifdef HANDLETIMEINDEVICES
    int Rate		(void) { return rate; };
#endif

    void useMapper		( MidiMapper *map);

virtual    void noteOn		( uchar chn, uchar note, uchar vel );
virtual    void noteOff		( uchar chn, uchar note, uchar vel );
virtual    void keyPressure	( uchar chn, uchar note, uchar vel );
virtual    void chnPatchChange	( uchar chn, uchar patch );
virtual    void chnPressure	( uchar chn, uchar vel );
virtual    void chnPitchBender	( uchar chn, uchar lsb,  uchar msb );
virtual    void chnController	( uchar chn, uchar ctl , uchar v ); 

virtual    void sysex		( uchar *data,ulong size);
    void channelSilence		( uchar chn );
    void channelMute    	( uchar chn, int a );

    int OK (void) 
    {
        if (seqfd<0) return 0;
        return (ok>0);
    };

#ifdef HANDLETIMEINDEVICES
    void wait (double ticks);
    void tmrSetTempo(int v);
    void tmrStart(void);
    void tmrStop(void);
    void tmrContinue(void);
    void sync(int i=0);  // if i==1 syncronizes by cleaning the buffer
			// instead of sending it (in fact, this is what
			// you syncronizing really means :-)
#endif

    char *getMidiMapFilename(void);

};

#endif
