/*  fmout.h	- class fmOut which handles the /dev/sequencer device
			for FM synths
    Copyright (C) 1998,99  Antonio Larrosa Jimenez

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

/**
 * FM device output class . FMOut is used to send MIDI events to 
 * FM devices, such as AdLib cards, or OPL3 synthesizers.
 *
 * FMOut inherits @ref MidiOut and supports the same simple API.
 * 
 * The preferred way to use this class is by selecting a FM device
 * on the @ref MidiManager and using a @ref MidiManager object directly
 *
 * @short Sends MIDI events to FM devices
 * @author Antonio Larrosa <larrosa@kde.org>
 */
class FMOut : public MidiOut
{
private:
friend class DeviceManager; 


  int patchloaded[256];
  int opl; // 3 or 4
  int nvoices;

  voiceManager *vm;

  void modifyPatch(char *buf, int key);
  void loadFMPatches  (void);

/**
 * Internal.
 * @return p if the patch p has been loaded, or another patch (already loaded)
 * if p hasn't been loaded. 
 */
  int patch(int p);

public:
/**
 * Constructor. See @ref MidiOut::MidiOut for more information.
 */
  FMOut  ( int d=0, int total =12 );

/**
 * Destructor. 
 */
  ~FMOut ();

/**
 * See @ref MidiOut::openDev
 */
  virtual void openDev	( int sqfd );

/**
 * See @ref MidiOut::closeDev
 */
  virtual void closeDev	( void );

/**
 * See @ref MidiOut::initDev
 */
  virtual void initDev	( void );

/**
 * See @ref MidiOut::noteOn
 */
  virtual void noteOn		( uchar chn, uchar note, uchar vel );

/**
 * See @ref MidiOut::noteOff
 */
  virtual void noteOff		( uchar chn, uchar note, uchar vel );
/**
 * See @ref MidiOut::keyPressure
 */
  virtual void keyPressure	( uchar chn, uchar note, uchar vel );
/**
 * See @ref MidiOut::chnPatchChange
 */
  virtual void chnPatchChange	( uchar chn, uchar patch );
/**
 * See @ref MidiOut::chnPressure
 */
  virtual void chnPressure	( uchar chn, uchar vel );
/**
 * See @ref MidiOut::chnPitchBender
 */
  virtual void chnPitchBender	( uchar chn, uchar lsb,  uchar msb );
/**
 * See @ref MidiOut::chnController
 */
  virtual void chnController	( uchar chn, uchar ctl , uchar v ); 

/**
 * It's an empty function, as FM devices don't support System Exclusive
 * messages
 */
  virtual void sysex		( uchar *data,ulong size);
/**
 * See @ref MidiOut::setVolumePercentage
 */
  virtual void setVolumePercentage    ( int i );

private:
  static const char *FMPatchesDirectory;
  static int deleteFMPatchesDirectory;

public:
/**
 * Sets the directory where the FM patches are stored, that is, where the
 * std.o3, std.sb, drums.o3 and drums.sb files can be found.
 *
 * It will store a copy of the parameter, so you should delete the memory
 * used by the parameter you passed.
 */
  static void setFMPatchesDirectory(const char *dir);

};

#endif
