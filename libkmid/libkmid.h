/*  libkmid.h     - class KMidSimpleAPI that makes it easy to use libkmid
			and a C wrapper.
    This file is part of LibKMid 0.9.5
    Copyright (C) 2000  Antonio Larrosa Jimenez
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
#ifndef _LIBKMID_H
#define _LIBKMID_H 

#ifdef __cplusplus

#include <kdelibs_export.h>

/**
 * Simple API covering most of the uses of libkmid.
 *
 * You can use the members of this class in pure C applications, just by using
 * the same name as the corresponding function member. 
 * 
 * Suppose you're developing a game and you want to play some background music
 * while the user is playing. You only have to call :
 * 
 * @li kMidInit();
 * @li kMidLoad("RideOfTheValkyries.mid");
 * @li kMidPlay();
 *
 * When the user decides to quit the game, use 
 *
 * @li kMidStop();
 * @li kMidDestruct();
 *
 * to stop the music and release the memory allocated by libkmid.
 *
 * @short A very simple API around the rest of libkmid.
 * @version 0.9.5 17/01/2000
 * @author Antonio Larrosa Jimenez <larrosa@kde.org> 
 */
class KMID_EXPORT KMidSimpleAPI 
{
  private:
    class KMidSimpleAPIPrivate;
    KMidSimpleAPIPrivate *d;

  public:

    /**
     * Initializes libkmid. Creates the DeviceManager object, and initializes
     * some variables that will be used later.
     *
     * @return 0 if OK, and a positive number when there's any error (for
     * example, because the /dev/sequencer device couldn't be opened, be it
     * because it was already opened by another application, or because the
     * sound card wasn't configured)
     */
    static int kMidInit(void);

    /**
     * Loads a song that will be played with the next call to kMidPlay().
     */
    static int kMidLoad(const char *filename);

    /**
     * Plays the song currently loaded with kMidLoad().
     * kMidPlay forks in order to play the song in a different process, it
     * exits inmediately, so that the application can follow the normal
     * execution flow while the sone is played.
     *
     * If loop is 0 the song is played once and then the child process
     * finishes. If loop is 1, the song is played repeatedly until
     * kMidStop() is called. You can call kMidStop() anytime you want
     * (also if loop is 0) to stop the song and kill the child process.
     *
     * @see kMidStop
     * @see kMidIsPlaying
     */
    static int kMidPlay(int loop=0);

    /**
     * Stops playing a song inmediatly. It doesn't return until the child
     * process that is playing the song is terminated.
     *
     * @see kMidPlay
     */
    static int kMidStop(void);

    /**
     * Releases the memory allocated by libkmid. To continue playing, you must
     * first make a(nother) call to kMidInit().
     */
    static void kMidDestruct(void);

    /**
     * Returns 1 if the library is playing a song, and 0 if it's not.
     * @see kMidPlay
     */
    static int kMidIsPlaying(void);

    /**
     * Returns the number of MIDI devices  ( MIDI ports + synthesizers )
     * @see DeviceManager::midiPorts
     * @see DeviceManager::synthDevices
     * @see kMidName
     * @see kMidType
     */
    static int kMidDevices(void);

    /**
     * Returns the name of the i-th device . In case libkmid wasn't yet
     * initialized ( see kMidInit() ), the return value is NULL, and in
     * case the parameter has a value out of the valid range 
     * ( see kMidDevices() ) it returns an empty string.
     *
     * @see kMidDevices
     * @see kMidType
     */
    static const char *kMidName(int i);

    /**
     * Returns the type of the i-th device . In case libkmid wasn't yet
     * initialized ( see kMidInit() ), the return value is NULL, and in
     * case the parameter has a value out of the valid range 
     * ( see kMidDevices() ) it returns an empty string.
     *
     * @see kMidDevices
     * @see kMidName
     */
    static const char *kMidType(int i);

    /**
     * Sets the MIDI device to use when playing a song.
     * @see kMidDevices
     * @see kMidName
     * @see DeviceManager
     */
    static void kMidSetDevice(int i);

    /**
     * Sets the Midi Mapper to use. Most of the users won't need a midi mapper,
     * but there're still non-General Midi synthesizers out there, and people
     * with one of those will get much better sound quality by using a MIDI
     * mapper. 
     *
     * Please have a look at KMid's documentation for more information
     * about MIDI mappers and how to write a MIDI mapper for your keyboard.
     */
    static void kMidSetMidiMapper(const char *mapfilename);

    /**
     * Returns the version number of libkmid, i.e. "0.9.5" or "1.0 Beta"
     */
    static const char *kMidVersion(void);

    /**
     * Returns the copyright notice that applications using libkmid should print
     * to the user in an about box or somewhere visible.
     * I.e. 
     *
     * "LibKMid 0.9.5 (C) 1997-2000 Antonio Larrosa Jimenez <larrosa@kde.org>. Spain"
     */ 
    static const char *kMidCopyright(void);

};



extern "C" {

#else
#define KMID_EXPORT
#endif

 
KMID_EXPORT  int    kMidInit(void);
KMID_EXPORT  int    kMidLoad(const char *filename);
KMID_EXPORT  int    kMidPlay(void);
KMID_EXPORT  int    kMidStop(void);
KMID_EXPORT  void   kMidDestruct(void);
KMID_EXPORT  int    kMidIsPlaying(void);
KMID_EXPORT  int    kMidDevices(void);
KMID_EXPORT  const char * kMidName(int i);
KMID_EXPORT  const char * kMidType(int i);
KMID_EXPORT  void   kMidSetDevice(int i);
KMID_EXPORT  void   kMidSetMidiMapper(const char *mapfilename);
KMID_EXPORT  const char * kMidVersion(void);
KMID_EXPORT  const char * kMidCopyright(void);



#ifdef __cplusplus

}

/**
 * @internal
 */
extern struct kMidData 
{
  class DeviceManager *midi;
  class MidiPlayer *player;
  class MidiMapper *map;
  struct PlayerController *pctl;
  int pctlsmID;
  int pid;
} kMid;
#endif


#endif
