/*  midimapper.h  - The midi mapper object
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/
#ifndef _MIDIMAPPER_H
#define _MIDIMAPPER_H

#include <stdio.h>
#include <libkmid/dattypes.h>

#define KM_NAME_SIZE 30

/**
 * A Midi Mapper class which defines the way MIDI events are translated
 * (or "mapped") to different ones. This way, when two MIDI devices "talk"
 * in a somehow different way, they can still communicate.
 *
 * When the user has an external keyboard that is not compatible with the
 * General Midi standard, he can use a MIDI mapper file to play files
 * as if the synthesizer was GM compatible.
 *
 * Please see the KMid documentation 
 * ( http://www.arrakis.es/~rlarrosa/kmid.html ) for information on the
 * format of a MIDI mapper definition file, and how they work.
 * 
 * I created this class because I had one of those non-GM keyboards, 
 * so it can do everything I needed it to do for my keyboard to work
 * exactly as a GM synth, and a few more things. Currently, it's the most
 * featured MIDI mapper available.
 *
 * The usage of this class is quite simple, just create an object with
 * a correct filename in the constructor and then use this object as
 * parameter for @ref DeviceManager::setMidiMap().
 *
 * @short Midi Mapper
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>  
 */
class MidiMapper
{
  private:
    class MidiMapperPrivate;
    MidiMapperPrivate *d;

    /**
     * @internal
     * Internal definition for Keymaps
     */
    struct Keymap
    {
      char name[KM_NAME_SIZE];
      uchar key[128];
      struct Keymap *next;
    };

    int	_ok;

    uchar	channelmap[16]; 
    /**
     * @internal
     * It's a pointer to the Keymap to use for a channel
     * This variable is used to get faster a given Keymap
     * The index is the real channel (after mapping it)
     */
    Keymap *channelKeymap[16]; 

    /**
     * @internal
     * It's -1 if the channel doesn't have a forced patch,
     * else indicates the patch to force in the channel.
     */
    int	channelPatchForced[16]; 

    uchar	patchmap[128];

    /**
     * @internal
     * Same as channelKeymap
     */ 
    Keymap *patchKeymap[128];

    /**
     * @internal
     * Real linked list of keymaps used around the class.
     */
    Keymap *keymaps; 

    /**
     * @internal
     * Stores the name of the file from which the map was loaded
     */
    char *_filename; 

    /**
     * @internal
     * Simulate expression events with volume events
     */
    int mapExpressionToVolumeEvents; 

    /**
     * @internal
     * Map or not the Pitch Bender using @ref #pitchBenderRatio()
     */
    int mapPitchBender;

    /**
     * @internal
     * Indicates the ratio between the standard and the synthesizer's pitch
     * bender engine. The number sent to the synth is multiplied by this
     * and dividied by 4096. Thus if PitchBenderRatio is 4096, the synth's
     * pitch bender works as the standard one
     */
    int pitchBenderRatio; 

    void getValue(char *s,char *v);
    void removeSpaces(char *s);
    int  countWords(char *s);
    void getWord(char *t,char *s,int w); 
    // get from s the word in position  w and store it in t

    void deallocateMaps(void);
    Keymap *createKeymap(char *name,uchar use_same_note=0,uchar note=0);
    void readPatchmap(FILE *fh);
    void readKeymap(FILE *fh,char *first_line);
    void readChannelmap(FILE *fh);
    void readOptions(FILE *fh);

    void addKeymap(Keymap *newkm);
    Keymap *keymap(char *n);

  public:
    /**
     * Constructor. Loads a MIDI Mapper definition from a file.
     * @see #filename()
     */
    MidiMapper(const char *name);

    /**
     * Destructor.
     */
    ~MidiMapper();

    /**
     * Loads a MIDI Mapper definition file (you don't need to use this if you
     * used a correct filename in constructor). 
     */
    void loadFile(const char *name);	

    /**
     * Returns the status of the object.
     */
    int  ok(void) { return _ok; };

    /** 
     * Returns the channel which chn should be mapped to.
     */ 
    uchar channel(uchar chn) { return channelmap[chn];};

    /** 
     * Returns the patch which pgm used on channel chn should be mapped to.
     */ 
    uchar patch(uchar chn,uchar pgm);

    /** 
     * Returns the key that key note playing a pgm patch on channel chn should
     * be mapped to.
     */ 
    uchar key(uchar chn,uchar pgm, uchar note);

    /**
     * Returns the value which the pitch bender on channel chn should be
     * mapped to.
     */
    void  pitchBender(uchar chn,uchar &lsb,uchar &msb);

    /**
     * Returns the value which a given controller and its value should
     * be mapped to when played on channel chn.
     */ 
    void  controller(uchar chn,uchar &ctl,uchar &v);

    /**
     * Returns the path and name of the file which the object loaded the
     * mapper from.
     */
    const char *filename(void);

};

#endif
