/*  midiout.h	- class midiOut which handles the /dev/sequencer device
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

/**
 * External MIDI port output class . This class is used to send midi
 * events to external midi devices. 
 *
 * MidiOut is inherited by other MIDI devices classes
 * (like @ref SynthOut or @ref FMOut) to support a common API.
 *
 * In general, you don't want to use MidiOut directly, but within a 
 * @ref DeviceManager object, which is the preferred way to generate music.
 *
 * If you want to add support for other devices (I don't think
 * there are any) you just have to create a class that inherits from MidiOut
 * and create one object of your new class in @ref DeviceManager::initManager.
 *
 * @short Sends MIDI events to external MIDI devices 
 * @author Antonio Larrosa <larrosa@kde.org>
 */
class MidiOut
{
//friend class DeviceManager; 
protected:
// This is the /dev/sequencer file handler. 
// Remember NOT TO CLOSE it on MidiOut, but just on DeviceManager
    int                 seqfd; 

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
  int	devicetype; 

  int 		volumepercentage;

  MidiMapper	*map;

  uchar	chnpatch [N_CHANNELS];
  int  	chnbender [N_CHANNELS];
  uchar	chnpressure [N_CHANNELS];
  uchar	chncontroller [N_CHANNELS][N_CTL];
  int	chnmute [N_CHANNELS];

  int	_ok;

  void seqbuf_dump (void);
  void seqbuf_clean(void);

public:

/**
 * Constructor. After constructing a MidiOut device, you must open it
 * (using @ref openDev). Additionally you may want to initialize it
 * (with @ref initDev),
 */
  	MidiOut(int d=0);

/**
 * Destructor. It doesn't matter if you close the device (@ref closeDev) before
 * you destruct the object because in other case, it will be closed here.
 */
virtual ~MidiOut();

/**
 * Opens the device. This is generally called from @ref DeviceManager , so you
 * shouldn't call this yourself (except if you created the MidiOut object
 * yourself.
 * @param sqfd a file descriptor of /dev/sequencer
 * @see #closeDev
 * @see #initDev
 */
  virtual void openDev	(int sqfd);

/**
 * Closes the device. It basically tells the device (the file descriptor)
 * is going to be closed.
 * @see openDev
 */
  virtual void closeDev	();

/**
 * Initializes the device sending generic standard midi events and controllers,
 * such as changing the patches of each channel to an Acoustic Piano (000),
 * setting the volume to a normal value, etc.
 */
  virtual void initDev	();

/**
 * @return the device type of the object. This is to identify the 
 * inherited class that a given object is polymorphed to.
 * The returned value is one of these :
 *
 * @li KMID_EXTERNAL_MIDI if it's a @ref MidiOut object
 * @li KMID_SYNTH if it's a @ref SynthOut object (as an AWE device)
 * @li KMID_FM if it's a @ref FMOut object
 * @li KMID_GUS if it's a @ref GUSOut object
 *
 * which are defined in midispec.h
 * 
 */
  int          deviceType () const { return devicetype; };

/**
 * @return the name and type of this MIDI device. 
 */
  const char * deviceName (void) const;

#ifdef HANDLETIMEINDEVICES
  int  rate	(void) { return rate; };
#endif

/**
 * Sets a @ref MidiMapper object to be used to modify the midi events before
 * sending them.
 *
 * @param map the MidiMapper to use.
 *
 * @see MidiMapper
 * @see #midiMapFilename
 */
  void setMidiMapper	( MidiMapper *map );

/**
 * See @ref DeviceManager::noteOn
 */
  virtual void noteOn	( uchar chn, uchar note, uchar vel );

/**
 * See @ref DeviceManager::noteOff
 */
  virtual void noteOff	( uchar chn, uchar note, uchar vel );

/**
 * See @ref DeviceManager::keyPressure
 */
  virtual void keyPressure	( uchar chn, uchar note, uchar vel );

/**
 * See @ref DeviceManager::chnPatchChange
 */
  virtual void chnPatchChange	( uchar chn, uchar patch );

/**
 * See @ref DeviceManager::chnPressure
 */
  virtual void chnPressure	( uchar chn, uchar vel );

/**
 * See @ref DeviceManager::chnPitchBender
 */
  virtual void chnPitchBender	( uchar chn, uchar lsb,  uchar msb );

/**
 * See @ref DeviceManager::chnController
 */
  virtual void chnController	( uchar chn, uchar ctl , uchar v ); 

/**
 * See @ref DeviceManager::sysex
 */
  virtual void sysex		( uchar *data,ulong size);

/**
 * Mutes all notes being played on a given channel.
 */
  virtual void channelSilence	( uchar chn );

/**
 * Mute or "unmute" a given channel .
 * @param chn channel to work on
 * @param b if true, the device will ignore subsequent notes played on the chn
 * channel, and mute all notes being played on it. If b is false, the channel
 * is back to work.
 */
  virtual void channelMute	( uchar chn, int a );

/**
 * Change all channel volume events multiplying it by this percentage correction
 * Instead of forcing a channel to a fixed volume, this method allows to 
 * music to fade out even when it was being played softly.
 * @param volper is an integer value, where 0 is quiet, 100 is used to send
 * an unmodified value, 200 play music twice louder than it should, etc.
 */ 
  virtual void setVolumePercentage ( int volper ) 
  			{ volumepercentage = volper; };

/**
 * @return true if everything's ok and false if there has been any problem
 */
  int ok (void) 
    { if (seqfd<0) return 0;
        return (_ok>0);
    };

#ifdef HANDLETIMEINDEVICES
  void wait        (double ticks);
  void tmrSetTempo (int v);
  void tmrStart    ();
  void tmrStop     ();
  void tmrContinue ();
  void sync        (int i=0);  // if i==1 syncronizes by cleaning the buffer
			// instead of sending it (in fact, this is what
			// syncronizing really means :-)
#endif

/**
 * @return the path to the file where the current used @ref MidiMapper object
 * reads the configuration from, or an empty string if there's no MidiMapper.
 */
  char *midiMapFilename ();

};

#endif
