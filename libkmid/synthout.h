/*  synthout.h	- class synthOut which handles the /dev/sequencer device
			for synths (as AWE32)
    Copyright (C) 1997,98  Antonio Larrosa Jimenez and P.J.Leonard
		  1999,2000 Antonio Larrosa Jimenez

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
#ifndef _SYNTHOUT_H
#define _SYNTHOUT_H

#include "midiout.h"

class SynthOut : public MidiOut
{
private:
   friend class DeviceManager; 

public:
   SynthOut(int d=0);
   ~SynthOut();

   void openDev	(int sqfd);
   void closeDev(void);
   void initDev	(void);

   void noteOn		( uchar chn, uchar note, uchar vel );
   void noteOff	( uchar chn, uchar note, uchar vel );
   void keyPressure	( uchar chn, uchar note, uchar vel );
   void chnPatchChange	( uchar chn, uchar patch );
   void chnPressure	( uchar chn, uchar vel );
   void chnPitchBender	( uchar chn, uchar lsb,  uchar msb );
   void chnController	( uchar chn, uchar ctl , uchar v ); 

   void sysex		( uchar *data,ulong size);
};

#endif
