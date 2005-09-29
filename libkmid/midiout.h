/*  midiout.h	- class midiOut which handles the /dev/sequencer device
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
#ifndef _MIDIOUT_H
#define _MIDIOUT_H

#include <libkmid/dattypes.h>
#include <libkmid/deviceman.h>
#include <libkmid/midimapper.h>
#include <stdio.h>

/**
 * External MIDI port output class . This class is used to send midi
 * events to external midi devices.
 *
 * MidiOut is inherited by other MIDI devices classes
 * (like SynthOut or FMOut) to support a common API.
 *
 * In general, you don't want to use MidiOut directly, but within a
 * DeviceManager object, which is the preferred way to generate music.
 *
 * If you want to add support for other devices (I don't think
 * there are any) you just have to create a class that inherits from MidiOut
 * and create one object of your new class in
 * DeviceManager::initManager().
 *
 * @short Sends MIDI events to external MIDI devices
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */
class MidiOut
{
  private:
    class MidiOutPrivate;
    MidiOutPrivate *d;

  protected:

  /**
   * @internal
   * This is the /dev/sequencer file handler.
   * Remember _not_to_close_ it on MidiOut, but just on DeviceManager
   */
  int seqfd;

  int device;

  int devicetype;

  int volumepercentage;

  MidiMapper *map;

  uchar	chnpatch [16];
  int  	chnbender [16];
  uchar	chnpressure [16];
  uchar	chncontroller [16][256];
  int	chnmute [16];

  int	_ok;

  void seqbuf_dump (void);
  void seqbuf_clean(void);

  public:

  /**
   * Constructor. After constructing a MidiOut device, you must open it
   * (using openDev() ). Additionally you may want to initialize it
   * (with initDev() ),
   */
  MidiOut(int d=0);

  /**
   * Destructor. It doesn't matter if you close the device ( closeDev() )
   * before you destruct the object because in other case, it will be closed
   * here.
   */
  virtual ~MidiOut();

  /**
   * Opens the device. This is generally called from DeviceManager , so you
   * shouldn't call this yourself (except if you created the MidiOut object
   * yourself.
   * @param sqfd a file descriptor of /dev/sequencer
   * @see closeDev
   * @see initDev
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
   * @li KMID_EXTERNAL_MIDI if it's a MidiOut object
   * @li KMID_SYNTH if it's a SynthOut object (as an AWE device)
   * @li KMID_FM if it's a FMOut object
   * @li KMID_GUS if it's a GUSOut object
   *
   * which are defined in midispec.h
   *
   * @see deviceName
   */
  int          deviceType () const { return devicetype; };

  /**
   * Returns the name and type of this MIDI device.
   * @see deviceType
   */
  const char * deviceName (void) const;

  /**
   * Sets a MidiMapper object to be used to modify the midi events before
   * sending them.
   *
   * @param map the MidiMapper to use.
   *
   * @see MidiMapper
   * @see midiMapFilename
   */
  void setMidiMapper	( MidiMapper *map );

  /**
   * See DeviceManager::noteOn()
   */
  virtual void noteOn	( uchar chn, uchar note, uchar vel );

  /**
   * See DeviceManager::noteOff()
   */
  virtual void noteOff	( uchar chn, uchar note, uchar vel );

  /**
   * See DeviceManager::keyPressure()
   */
  virtual void keyPressure	( uchar chn, uchar note, uchar vel );

  /**
   * See DeviceManager::chnPatchChange()
   */
  virtual void chnPatchChange	( uchar chn, uchar patch );

  /**
   * See DeviceManager::chnPressure()
   */
  virtual void chnPressure	( uchar chn, uchar vel );

  /**
   * See DeviceManager::chnPitchBender()
   */
  virtual void chnPitchBender	( uchar chn, uchar lsb,  uchar msb );

  /**
   * See DeviceManager::chnController()
   */
  virtual void chnController	( uchar chn, uchar ctl , uchar v );

  /**
   * See DeviceManager::sysex()
   */
  virtual void sysex		( uchar *data,ulong size);

  /**
   * Send a All Notes Off event to every channel
   */
  void allNotesOff(void);

  /**
   * Mutes all notes being played on a given channel.
   * @param chn the channel
   */
  virtual void channelSilence	( uchar chn );

  /**
   * Mute or "unmute" a given channel .
   * @param chn channel to work on
   * @param b if true, the device will ignore subsequent notes played on the chn
   * channel, and mute all notes being played on it. If b is false, the channel
   * is back to work.
   */
  virtual void channelMute	( uchar chn, int b );

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
   * Returns true if everything's ok and false if there has been any problem
   */
  int ok (void)
  { if (seqfd<0) return 0;
    return (_ok>0);
  };

  /**
   * Returns the path to the file where the current used MidiMapper object
   * reads the configuration from, or an empty string if there's no MidiMapper.
   */
  const char *midiMapFilename ();
 
  /**
   * Sends the buffer to the device and returns when it's played, so you can
   * synchronize
   * XXX: sync should be virtual after next bic release
   */
  void sync(int i=0);

};

#endif
