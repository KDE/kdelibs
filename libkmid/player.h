/*  player.h  - class MidiPlayer. Plays a set of tracks
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
#ifndef _PLAYER_H
#define _PLAYER_H

#include <libkmid/dattypes.h>
#include <libkmid/midfile.h>
#include <libkmid/deviceman.h>
#include <libkmid/track.h>
#include <libkmid/notearray.h>
#include <kdemacros.h>

/**
 * This struct stores text, lyrics and change tempo events among others.
 *
 * It includes the main information for an event. That is, the absolute
 * millisecond at which this event is played (from the beginning of the song),
 * the delta milliseconds from the previous SpecialEvent, an ID, etc. 
 *
 * This struct is used as nodes for a linked list, which you can get using
 * MidiPlayer::specialEvents().
 *
 * @short Struct used to store certain events
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */
struct SpecialEvent
{
  /**
   * An integer ID, that is assigned in order to each SpecialEvent.
   */
  int id;

  /**
   * Delta milliseconds from the previous SpecialEvent. 
   *
   * @see absmilliseconds
   */
  ulong diffmilliseconds;

  /**
   * The absolute millisecond (from the beginning of the song) at which this
   * SpecialEvent object is played.
   *
   * @see diffmilliseconds
   */
  ulong absmilliseconds;

  /**
   * MIDI ticks (from the beginning of the song) at which this event is played.
   */
  int ticks;

  /**
   * Type of event. This currently includes:
   *
   * @li 0 - Nothing, end of linked list.
   * @li 1 - Text Event . See text.
   * @li 3 - Change Tempo Event . See tempo.
   * @li 5 - Lyrics Event . See text.
   * @li 6 - Change number of beats per measure . See num and den.
   * @li 7 - Beat . See num and den.
   *
   * The "Change number of beats per measure" and "beat" events are not really
   * in the midi file, but they are added to the linked list in case you have
   * an use for it.
   */
  int type;

  /**
   * Text field . It has a meaning only for Text and Lyrics events.
   */
  char text[1024];

  /**
   * Tempo field . It has a meaning only for Change Tempo events.
   */
  ulong tempo;
  /**
   * Numerator . It has a meaning only for Change number of beats per measure and
   * beat events.
   */
  int num;
  /**
   * Denominator . It has a meaning only for Change number of beats per measure
   * and beat events.
   */
  int den;

  /**
   * This struct stores text, lyrics and change tempo events among others.
   *
   * It includes the main information for an event. That is, the absolute
   * millisecond at which this event is played (from the beginning of the song),
   * the delta milliseconds from the previous SpecialEvent, an ID, etc. 
   *
   * This struct is used as nodes for a linked list, which you can get using
   * MidiPlayer::specialEvents().
   */
  struct SpecialEvent *next;

  /**
   * Next node in the linked list.
   */
  
};


/**
 * PlayerController is a struct that is used by the MidiPlayer object
 * to tell other parts of the application about the status of the MIDI playing.
 *
 * @short Struct used to have control over the player engine
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */
struct PlayerController
{
  volatile ulong  ticksTotal;
  volatile ulong  ticksPlayed;
  volatile double millisecsPlayed;
  volatile ulong  beginmillisec;

  volatile int tempo;
  volatile int num;
  volatile int den;

  volatile int SPEVprocessed;
  volatile int SPEVplayed;

  /**
   * When pause is released, if the caller must know when the player has
   * opened the devices and is playing again, then it just has to check
   * to see when OK changes the value to 1
   */
  volatile int OK; 

  /**
   * When the player is playing (or paused), playing is set to 1.
   */
  volatile int playing;

  /**
   * When the player is paused, paused is set to 1.
   */
  volatile int paused;

  /**
   * When the player seeking the position of the song, moving is set to 1.
   */
  volatile int moving;

  /**
   * When the player has finished playing a song, finished is set to 1.
   */
  volatile int finished;

  /**
   * @internal
   * @deprecated
   * Not used
   */
  volatile int message KDE_DEPRECATED; // set one of the following :

#define PLAYER_DOPAUSE	1
#define PLAYER_DOSTOP	2
#define PLAYER_SETPOS	4
#define PLAYER_HALT	8

  volatile ulong gotomsec; //milliseconds to go to,if player_setpos is set

  /**
   * When error is 1, an error has ocurred (i.e. it coultn't open the device)
   */    
  volatile int error;

  /**
   * If gm is 1, the song follows the General Midi standard, if gm is 0, the song
   * is in MT 32 format.
   */
  volatile int gm;

  /**
   * 100 means no change, 50 halfs the volume, 200 doubles it (if possible), etc.
   *
   * @see DeviceManager::setVolumePercentage()
   */
  volatile int volumepercentage ; 

  /**
   * Activate or disactivate the force to use a patch for a given channel.
   * @see pgm
   */
  volatile bool forcepgm[16];

  /**
   * Force a given patch in each channel at "this" moment, as determined by
   * forcepgm.
   */
  volatile int pgm[16];

  /**
   * Ratio to multiply the tempo to.
   */
  volatile double ratioTempo;

  /**
   * @internal Used to stop the main pid until the child has finished to 
   * send the all notes off event
   */
  volatile bool isSendingAllNotesOff;

  volatile MidiEvent *ev;
};


/**
 * MIDI file player routines . This class reads a MIDI file and
 * play it using a DeviceManager object.
 *
 * To use it, just call loadSong() with the filename you want to play,
 * and then play().
 * 
 * Please have a look at the note in the play() documentation.
 *
 * MidiPlayer will write information about the playing process on a 
 * PlayerController() structure that you must supply to the constructor
 *
 * Alternatively, if everything you want is to play a midi file in a game or
 * any other application that doesn't need to fine tune the midi playing, just
 * use the kmidplay() function.
 *
 * @see KMidSimpleAPI
 *
 * @short The MIDI file player engine
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org>
 */
class KMID_EXPORT MidiPlayer
{
  class MidiPlayerPrivate;
  MidiPlayerPrivate *d;

  DeviceManager *midi;
  MidiFileInfo *info;
  MidiTrack **tracks;
  SpecialEvent *spev;
  NoteArray *na;

  int songLoaded;

  PlayerController *ctl;

  bool parsesong;
  bool generatebeats;

  void removeSpecialEvents(void);
  void parseSpecialEvents(void);
  void insertBeat(SpecialEvent *ev,ulong ms,int num,int den);
  void generateBeats(void);

  //NoteArray *parseNotes(void);
  void debugSpecialEvents(void);
  public:

  /**
   * Constructor . You must construct and pass a DeviceManager object and a 
   * PlayerController structure. None of them will be destroyed by this
   * object, so you should do it after destroying the MidiPlayer object.
   */
  MidiPlayer(DeviceManager *midi_,PlayerController *pctl);

  /**
   * Destructor.
   */
  ~MidiPlayer();

  /**
   * Loads a Song, and parses it (it the parse wasn't disabled with 
   * setParseSong() ) . It also generates the Beat events (see 
   * SpecialEvent::type() ) if you enabled this by using 
   * setGenerateBeats() .
   */
  int loadSong(const char *filename);

  /**
   * Unloads the current song, so that every internal variable is empty and clean
   * for further usage.
   */
  void removeSong(void); 

  /**
   * Returns true if there's a song already loaded (with a previous call to 
   * loadSong() ) and false if not.
   */
  int isSongLoaded(void) { return songLoaded; };

  /**
   * Returns the linked list of SpecialEvents objects . For this to work,
   * the parse should be enabled (the default), by using setParseSong().
   */
  SpecialEvent *specialEvents() { return spev; };

  /**
   * Returns and array with the notes playen through the song . MidiPlayer must
   * parse the song to get the notes, so be sure not to disable the parsing of
   * the song.
   *
   * Returns an array of notes, (just note on and note off events), in the form
   * of a NoteArray object
   */
  NoteArray *noteArray(void) { return na; };


  /**
   * Plays the song using the DeviceManager object supplied in the
   * constructor. It should be already configured, as play doesn't change the
   * volume, nor midi mapper, for example.
   *
   * Note: Calling this function will block the execution of your application
   * until the song finishes playing. The solution for this is simple, fork
   * before calling it, and create the PlayerController object on shared
   * memory.
   *
   * As alternative, if everything you want is playing a midi file in a game or
   * any other application that doesn't need to fine tune the midi playing, just
   * use the KMidSimpleAPI::kMidPlay() function. 
   *
   * @see KMidSimpleAPI::kMidInit
   * @see KMidSimpleAPI::kMidPlay
   * @see KMidSimpleAPI::kMidStop
   * @see KMidSimpleAPI::kMidDestruct
   */
  void play(bool calloutput=false,void output(void) = 0);

  /**
   * Enables or disables the parsing of the song when loading it. This affects
   * the SpecialEvents ( specialEvents() ) and the NoteArray 
   * ( noteArray() ).
   */
  void setParseSong(bool b = true);

  /**
   * Enables or disables the generation of beats event in a song when loading
   * it. 
   */
  void setGenerateBeats(bool b = false);

  /**
   * Returns information about the current MIDI file.
   *
   * @see loadSong
   */
  MidiFileInfo *information(void) { return info; };

  /**
   * Sets the position in a song. 
   * @param gotomsec the number of milliseconds to go to . A subsequent call to
   * play() will start playing the song from that moment, instead of the
   * beginning.
   * @param midistat a MidiStatus object that will contain the status in
   * which the MIDI device would be if it would have arrived to this situation by
   * a normal playing of the song.
   */
  void setPos(ulong gotomsec, class MidiStatus *midistat);

  /**
   * Changes the speed at which a song is played. The song's tempo is multiplied
   * by the specified ratio.
   */
  void setTempoRatio(double ratio);

};

#endif
