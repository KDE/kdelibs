/*  midistat.h	- class midiStat, change it internally and then send it. 
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
#ifndef _MIDISTAT_H
#define _MIDISTAT_H

#include <libkmid/dattypes.h>

/**
 * Stores the status of a MIDI device . That is, current patch in each channel,
 * controller settings, pitch bender value, etc.
 *
 * This is used to "play" with all those values and then send them to the
 * MIDI device just by using sendData() 
 *
 * @short Stores the MIDI status.
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */
class MidiStatus
{
  private:
    class MidiStatusPrivate;
    MidiStatusPrivate *d;

    ulong tempo;

    unsigned char chn_patch   [16];
    int           chn_bender  [16];
    unsigned char chn_pressure[16];
    unsigned char chn_controller[16][256];

    int           chn_lastisvolumeev[16];

  public:
    /**
     * Constructor.
     */
    MidiStatus();

    /**
     * Destructor.
     */ 
    ~MidiStatus();


    //    void noteOn	( uchar chn, uchar note, uchar vel );
    //    void noteOff	( uchar chn, uchar note, uchar vel );

    /**
     * Stores a new value for the key aftertouch.
     * @see MidiOut::keyPressure()
     */
    void keyPressure	( uchar chn, uchar note, uchar vel );

    /**
     * Stores a new patch in channel @p chn.
     * @see chnPatch()
     * @see MidiOut::chnPatchChange()
     */
    void chnPatchChange	( uchar chn, uchar patch );

    /**
     * Returns the patch currently used in channel @p chn.
     */
    uchar chnPatch	( uchar chn ) { return chn_patch[chn]; };

    /**
     * Stores a new channel pressure value in channel @p chn.
     * @see MidiOut::chnPressure()
     */
    void chnPressure	( uchar chn, uchar vel );

    /**
     * Returns the pressure value currently used in channel @p chn.
     */
    uchar chnPressure	( uchar chn ) { return chn_pressure[chn]; };

    /**
     * Stores a new pitch bender value in channel chn
     */
    void chnPitchBender	( uchar chn, uchar lsb, uchar msb );

    /**
     * Returns the pitch bender value used in channel @p chn
     */
    int chnPitchBender	( uchar chn) { return chn_bender[chn]; };

    /**
     * Stores a new value for controller @p ctl in channel @p chn.
     */
    void chnController	( uchar chn, uchar ctl , uchar v ); 

    /**
     * Returns the value used for controller @p ctl in channel @p chn
     */
    uchar chnController	( uchar chn, uchar ctl ) 
	{ return chn_controller[chn][ctl]; };

    /**
     * Stores a sysex message that will be send in the next call to sendData
     */ 
    void sysex		( uchar *data, ulong size);

    /**
     * Sets the tempo.
     *
     * @see DeviceManager::tmrSetTempo()
     */
    void tmrSetTempo	( int v );


    /**
     * Sends the current MIDI state to the DeviceManager object used as
     * parameter (you should have already set the default device to the one you
     * want to use). The @p gm parameter specifies if the patches used follow
     * the GM standard (1), or follow the MT32 standard (0), in which case, they
     * will be converted to GM before being sent.
     */
    void sendData	( class DeviceManager *midi, int gm=1 );
};

#endif
