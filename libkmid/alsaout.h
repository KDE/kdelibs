/*  alsaout.cc   - class AlsaOut which represents an alsa client/port pair
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
#ifndef _ALSAOUT_H
#define _ALSAOUT_H

#include <libkmid/midiout.h>

struct snd_seq_event;
typedef struct snd_seq_event snd_seq_event_t;

/**
 * @short Sends MIDI events to a MIDI devices using ALSA
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */
class AlsaOut : public MidiOut
{
  friend class DeviceManager;

  protected:

/**
 * @internal
 * Total number of devices.
 */
  int ndevs;

/**
 * @internal
 * Total number of midi ports
 */
  int nmidiports;

  double count;
  double lastcount;
  double lasttime;
  double begintime;
  int m_rate;

/**
 * @internal
 * A "constant" used to convert from milliseconds to the computer rate
 */
  double convertrate;

  long int time;

  virtual void seqbuf_dump (void);
  virtual void seqbuf_clean(void);
  void eventInit(snd_seq_event_t *ev);
  void eventSend(snd_seq_event_t *ep);
  void timerEventSend(int type);

  public:

  /**
   * Constructor. After constructing a MidiOut device, you must open it
   * (using openDev() ). Additionally you may want to initialize it
   * (with initDev() ),
   */
  AlsaOut(int d, int client=64, int port=0, const char *cname="", const char *pname="");

  /**
   * Destructor. It doesn't matter if you close the device ( closeDev() )
   * before you destruct the object because in other case, it will be closed
   * here.
   */
  virtual ~AlsaOut();

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
  virtual const char * deviceName (void) const;

  /**
   * @internal
   */
  int  rate    (void) { return m_rate; };

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

  virtual void wait        (double ticks);
  virtual void tmrSetTempo (int v);
  virtual void tmrStart    (int tpcn);
  virtual void tmrStart    () { tmrStart(-1); }
  virtual void tmrStop     ();
  virtual void tmrContinue ();
  /**
   * @internal
   * If i==1 syncronizes by cleaning the buffer instead of sending it (in fact,
   * this is what synchronizing really means :-) )
   */
  void sync        (int i=0);

  class AlsaOutPrivate;
  AlsaOutPrivate *di;
};

#endif // _ALSAOUT_H
