/*  gusout.h   - class gusOut which implements support for Gravis
         Ultrasound cards through a /dev/sequencer device
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/
#ifndef _GUSOUT_H
#define _GUSOUT_H

#include <libkmid/midiout.h>
#include <libkmid/voiceman.h>

/**
 * Gravis Ultrasound synthesizer output class . This class is used to send midi
 * events to synthesizers on GUS cards.
 *
 * GUSOut inherits @ref MidiOut and supports the same simple API.
 *
 * The recommended way to use this class is by using a @ref DeviceManager
 * object, and use the @ref DeviceManager::setPatchesToUse() member which will
 * call the @ref #setPatchesToUse() member in this class.
 *
 * @short Sends MIDI events to GUS synths
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */ 
class GUSOut : public MidiOut
{
  private:
    class GUSOutPrivate;
    GUSOutPrivate *di;

    int patchloaded[256];
    int nvoices;

    int use8bit; // Use 8 bit patches, instead of 16 bits to use less memory
    VoiceManager *vm;

    int totalmemory; // Total memory in soundcard
    int freememory; // Free memory


    void patchesLoadingOrder(int *patchesused,int *patchesordered);
    const char *patchName(int pgm);

  public:
    /**
     * Constructor. See @ref MidiOut::MidiOut() for more information.
     */
    GUSOut(int d=0,int total =12);

    /**
     * Destructor.
     */
    ~GUSOut();

    /**
     * See @ref MidiOut::openDev()
     */
    virtual void openDev	(int sqfd);

    /**
     * See @ref MidiOut::closeDev()
     */
    virtual void closeDev	(void);

    /**
     * See @ref MidiOut::initDev()
     */
    virtual void initDev	(void);

    /**
     * See @ref MidiOut::noteOn()
     */
    virtual void noteOn		( uchar chn, uchar note, uchar vel );

    /**
     * See @ref MidiOut::noteOff()
     */
    virtual void noteOff	( uchar chn, uchar note, uchar vel );

    /**
     * See @ref MidiOut::keyPressure()
     */
    virtual void keyPressure	( uchar chn, uchar note, uchar vel );

    /**
     * See @ref MidiOut::chnPatchChange()
     */
    virtual void chnPatchChange	( uchar chn, uchar patch );

    /**
     * See @ref MidiOut::chnPressure()
     */
    virtual void chnPressure	( uchar chn, uchar vel );

    /**
     * See @ref MidiOut::chnPitchBender()
     */
    virtual void chnPitchBender	( uchar chn, uchar lsb,  uchar msb );

    /**
     * See @ref MidiOut::chnController()
     */
    virtual void chnController	( uchar chn, uchar ctl , uchar v ); 

    /**
     * It's an empty function, as GUS synths don't support System Exclusive
     * messages
     */
    virtual void sysex		( uchar *data,ulong size);

    /**
     * See @ref DeviceManager::setPatchesToUse() . All the information about this
     * member is explained there because it's (for now) just a simple call to this
     * function when the device used is a GUS device, and you're supposed to use
     * a DeviceManager object instead of a GUSOut object except in rare ocassions.
     *
     * @see #patch()
     * @see #loadPatch()
     */
    void setPatchesToUse(int *patchesused);

    /**
     * Loads a single patch on the synthesizer memory.
     * @param pgm is the number of the GM patch when pgm is between 0 and 127.
     * Values from 128 to 255 are used to represent the percussion instruments.
     * @return 0 if OK and -1 if there was an error (patch not found, not enough
     * memory, etc.)
     *
     * @see #patch()
     * @see #setPatchesToUse()
     */
    int loadPatch  (int pgm);

    /**
     * Returns p if the patch with number p has been correctly loaded.
     * In the case it hasn't been loaded, it returns the number of another patch
     * that is loaded and that should be used instead.
     *
     * @see #loadPatch()
     * @see #setPatchesToUse()
     */
    int patch(int p); 

  private:
    static const char *GUS_patches_directory;
    static int delete_GUS_patches_directory;

  public:
    /**
     * Sets the directory where the GUS patches are stored, that is, where the
     * acpiano.pat, ... files can be found.
     *
     * It will store a copy of the parameter, so you should delete the memory
     * used by the parameter you passed.
     */ 
    static void setGUSPatchesDirectory(const char *dir);

};

#endif
