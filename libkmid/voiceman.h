/*  voiceman.h - The VoiceManager class handles a set of voices for synths
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
#ifndef _VOICEMAN_H
#define _VOICEMAN_H

/**
 * @internal
 * Manages the voices used by synthesizers.
 *
 * @short Manages internally the voices used by synth devices.
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */ 
class VoiceManager
{
  private:
    class VoiceManagerPrivate;
    VoiceManagerPrivate *d;

    /**
     * Number of voices managed by this object.
     */
    int nvoices;

    /**
     * @internal
     */
    struct voice
    {
      int     id;
      int	channel;
      int	note;
      int	used;

      struct voice *prev;
      struct voice *next;
    };

    /**
     * Points to the beginning of the voice list, that is, to
     * the older voice which is ready to be used.
     */
    voice *FirstVoice; 

    /**
     * Points to the last voice, that is, the latest (most recently) used voice.
     */
    voice *LastVoice; 

    /**
     * Points to the latest (list order) not used voice,
     * that is, to where deallocated voices will be moved.
     */
    voice *LastnotusedVoice; 

    /**
     * Array with pointers to the voices, arranged by ID for allow faster searches.
     */
    voice **VoiceList; 

    /**
     * @internal
     * This variable is used to search channels.
     */
    voice *searcher; 

    /**
     * @internal
     * An auxiliary variable for simpler searches.
     */ 
    voice *searcher_aid; 

  public:
    /**
     * Cronstructor.
     */
    VoiceManager(int totalvoices);

    /**
     * Destructor.
     */
    ~VoiceManager();

    /**
     * Allocates a voice used in channel @p chn, and playing key @p key
     * @return the voice that should be used.
     *
     * @see deallocateVoice
     */

    int allocateVoice(int chn,int key);

    /**
     * Deallocates the voice with ID @p id.
     *
     * @see allocateVoice
     */ 
    void deallocateVoice(int id);

    /**
     * initSearch() must be called always before search() to initialize
     * internal variables.
     * 
     * @see search
     */
    void initSearch(void); 

    /**
     * Returns -1 if channel chn is not currently used, or a voice using
     * channel @p chn if any.
     *
     * Calling search repeteadly, will return all the voices using channel 
     * @p chn, and a -1 after the last one.
     *
     * @see initSearch
     */
    int search(int chn); 
    //returns -1 if channel chn is not currently used
    // Continue searching for more voices which
    // use the channel chn

    /**
     * This is a convenience function that differs from the above in that it also
     * looks for a specific note (the second parameter)
     * 
     * @see initSearch
     */
    int search(int chn,int note);

    /**
     * Returns the channel that voice @p v is using.
     */
    int channel(int v) {return VoiceList[v]->channel;};

    /**
     * Returns the note that voice @p v is playing.
     */
    int note(int v) {return VoiceList[v]->note;};

    /**
     * Returns true or false if the voice @p v is being used or not respectively.
     */
    int used(int v) {return VoiceList[v]->used;};

    /**
     * Clears the lists of used voices.
     */
    void clearLists(void);
};

#endif
