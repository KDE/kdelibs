/**************************************************************************

    fmout.h	- class fmOut which handles the /dev/sequencer device
			for fm synths
    Copyright (C) 1998  Antonio Larrosa Jimenez

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
#ifndef _FMOUT_H
#define _FMOUT_H

#include "midiout.h"
#include "voiceman.h"

class fmOut : public midiOut
{
private:
friend class DeviceManager; 


    int patchloaded[256];
    int opl; // 3 or 4
    int nvoices;

    voiceManager *vm;

    void modifyPatch(char *buf, int key);
    void loadFMPatches  (void);

    int Patch(int p); //Returns p if the patch p has been loaded or another
			// patch (already loaded) if p hasn't been loaded 
public:
    fmOut(int d=0,int total =12);
    ~fmOut();

    virtual void openDev	(int sqfd);
    virtual void closeDev	(void);
    virtual void initDev	(void);

    virtual void noteOn		( uchar chn, uchar note, uchar vel );
    virtual void noteOff	( uchar chn, uchar note, uchar vel );
    virtual void keyPressure	( uchar chn, uchar note, uchar vel );
    virtual void chnPatchChange	( uchar chn, uchar patch );
    virtual void chnPressure	( uchar chn, uchar vel );
    virtual void chnPitchBender	( uchar chn, uchar lsb,  uchar msb );
    virtual void chnController	( uchar chn, uchar ctl , uchar v ); 

    virtual void sysex		( uchar *data,ulong size);
    virtual void setVolumePercentage    ( int i );

    
private:
static const char *FM_patches_directory;
static int delete_FM_patches_directory;
public:
static void setFMPatchesDirectory(const char *dir);

};

#endif
