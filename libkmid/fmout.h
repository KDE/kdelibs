/*  fmout.h	- class fmOut which handles the /dev/sequencer device
			for FM synths
    This file is part of LibKMid 0.9.5
    Copyright (C) 1998,99,2000  Antonio Larrosa Jimenez
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
#ifndef _FMOUT_H
#define _FMOUT_H

#include <libkmid/midiout.h>
#include <libkmid/voiceman.h>

/**
 * FM device output class . FMOut is used to send MIDI events to 
 * FM devices, such as AdLib cards, or OPL3 synthesizers.
 *
 * FMOut inherits MidiOut and supports the same simple API.
 * 
 * The preferred way to use this class is by selecting a FM device
 * on the MidiManager and using a MidiManager object directly
 *
 * @short Sends MIDI events to FM devices
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */
class KMID_EXPORT FMOut : public MidiOut
{
  private:
    class FMOutPrivate;
    FMOutPrivate *di;

    int patchloaded[256];
    /**
     * Takes a value of 2 or 3, for FM or OPL3 support
     */
    int opl;
    int nvoices;

    VoiceManager *vm;

    void modifyPatch(char *buf, int key);
    void loadFMPatches  (void);

  public:
    /**
     * Constructor. See MidiOut::MidiOut() for more information.
     */
    FMOut  ( int d=0, int total =12 );

    /**
     * Destructor. 
     */
    ~FMOut ();

    /**
     * See MidiOut::openDev()
     */
    virtual void openDev	( int sqfd );

    /**
     * See MidiOut::closeDev()
     */
    virtual void closeDev	( void );

    /**
     * See MidiOut::initDev()
     */
    virtual void initDev	( void );

    /**
     * See MidiOut::noteOn()
     */
    virtual void noteOn		( uchar chn, uchar note, uchar vel );

    /**
     * See MidiOut::noteOff()
     */
    virtual void noteOff		( uchar chn, uchar note, uchar vel );

    /**
     * See MidiOut::keyPressure()
     */
    virtual void keyPressure	( uchar chn, uchar note, uchar vel );

    /**
     * See MidiOut::chnPatchChange()
     */
    virtual void chnPatchChange	( uchar chn, uchar patch );

    /**
     * See MidiOut::chnPressure()
     */
    virtual void chnPressure	( uchar chn, uchar vel );

    /**
     * See MidiOut::chnPitchBender()
     */
    virtual void chnPitchBender	( uchar chn, uchar lsb,  uchar msb );

    /**
     * See MidiOut::chnController()
     */
    virtual void chnController	( uchar chn, uchar ctl , uchar v ); 

    /**
     * It's an empty function, as FM devices don't support System Exclusive
     * messages
     */
    virtual void sysex		( uchar *data,ulong size);

    /**
     * See MidiOut::setVolumePercentage()
     */
    virtual void setVolumePercentage    ( int i );

    /**
     * Returns @p p if the patch p has been loaded, or another patch (already loaded)
     * if @p p hasn't been loaded. 
     */
    int patch(int p);

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
