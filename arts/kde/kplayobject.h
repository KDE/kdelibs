	/*

	Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>

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

	*/

#ifndef KPLAYOBJECT_H
#define KPLAYOBJECT_H

#include "kmedia2.h"
#include "soundserver.h"
#include <kurl.h>
#include <qobject.h>

class KPlayObject : public QObject
{
Q_OBJECT
public:
	KPlayObject();
	KPlayObject(Arts::PlayObject playobject, bool isStream);
	~KPlayObject();

	/**
	  * Sets the internal Arts::PlayObject
	  * to @playObject
	  */
	void setObject(Arts::PlayObject playObject);
	
	/**
	  * Returns the internal Arts::PlayObject
	  */
	Arts::PlayObject object();

	/**
	 * return true if both this != 0, and object.isNull()
	 *
	 * in essence, ((KPlayObject*)0)->isNull() will not 
	 * crash
	 **/
	bool isNull();

	/**
	 * returns true if the internally playobject
	 * is used to play a stream
	 */
    	bool stream();

	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	void play();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	void seek(Arts::poTime newTime);
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	void pause();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	 
	void halt();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	QString description();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	Arts::poTime currentTime();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	Arts::poTime overallTime();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	Arts::poCapabilities capabilities();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	QString mediaName();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	Arts::poState state();

private:
	Arts::PlayObject m_playObject;
	bool m_isStream;
};



namespace KDE {

class PlayObjectFactory;

/**
  * This class acts as a general interface to the KDE multimedia framework.
  * You basically point the Playobject to an URL and say "play", and it will
  * automatically decode and play and / or display the file or stream.
  * For non-local media, it will make extensive use of KIOInputStream to
  * directly play the content from the remote location without having to
  * download it to a temporary local file first.
  *
  * A KDE::PlayObject is never created directly with new, but only through
  * a KDE::PlayObjectFactory.
  *
  * Basically, it is used like this:
  * \code
  * KArtsDispatcher dispatcher;
  * KArtsServer server;
  * KDE::PlayObjectFactory factory( server.server() );
  * KDE::PlayObject* playobj = factory.createPlayObject( someURL, true );
  * playobj->play();
  * \endcode
  *
  * Internally, the KDE::PlayObject acts as a wrapper for an Arts::PlayObject.
  *
  * Special care needs to be taken for non-local media. In general, you
  * cannot safely figure out the mimetype of the remote media content, by
  * looking at the URL alone. You need to download some data to analyze
  * the content. Since KDE::PlayObject is only a wrapper for an
  * Arts::PlayObject, and an Arts::PlayObject needs to know the mimetype
  * of the data it plays in order to pick the correct decoder, one cannot
  * directly create an Arts::PlayObject and attach it to a stream. Therefore,
  * the following approach is used.
  *
  * Whenever a the factory creates a KDE::PlayObject for a non-local content,
  * it first generates a so called "Proxy" Playobject. This is a
  * KDE::PlayObject that does not contain a real Arts::PlayObject yet.
  * As soon as you invoke the play() method, a connection to the media
  * source is made, and as soon as the mimetype is known, the appropriate
  * Arts::PlayObject is created.
  *
  * This has some side effects that developers need to be aware of:
  * Until the real Arts::PlayObject got created,
  * - the capabilities() method returns "zero" capabilities,
  * - description() and mediaName() will return a null QString,
  * - currentTime() and overallTime() will return "zero",
  * - despite the fact that isNull() returns "false", object().isNull()
  *   will return "true". If you need to directly access methods of the
  *   internal Arts::PlayObject, be sure to use object().isNull() to guard
  *   your access.
  *
  * A KDE::PlayObject will emit the signal playObjectCreated()
  * as soon as the real internal Arts::PlayObject got created. This is also
  * true for local media files. So you can generally connect to this signal
  * and act on it if your application needs to know about the real capabilities
  * of the Arts::PlayObject.
  *
  * However, KDE::PlayObject will try to act reasonable on calls to play(),
  * halt(), pause() and state(). If you call play() and then pause() 
  * before the connection to the media source was established, it will 
  * not start playing once the connection got established. Calling halt()
  * will cancel the connection process. KDE::PlayObject will maintain
  * an internal state variable, and calling state() will return this
  * internal state until the real Arts::PlayObject got created, afterwards
  * the state of the Arts::PlayObject will be returned.
  */
class PlayObject : public QObject
{
Q_OBJECT
public:
	~PlayObject();

	/**
	  * Returns the internal Arts::PlayObject
	  */
	Arts::PlayObject object();

	/**
	 * return true if this != 0.
	 *
	 * in essence, ((KDE::PlayObject*)0)->isNull() will not 
	 * crash
	 **/
	bool isNull();

	/**
	 * returns "true" if the content to play is delivered as a stream.
	 */
    	bool stream();

	/**
	 * causes the PlayObject to start the play back.
	 */
	void play();
	
	/**
	 * causes the PlayObject to skip to the time @p newTime. 
	 * You don't need to stop or restart the play back after calling seek.
	 */
	void seek(Arts::poTime newTime);
	
	/**
	 * causes the PlayObject to pause play back immediately. It will not
	 * restart until you call play(). This also works on streams, the
	 * connection to the media source will be maintained while the 
	 * PlayObject is paused.
	 */
	void pause();
	
	/**
	 * immediately stops the play back and resets the media to the
	 * start of the content. If playing from a stream, halt() causes
	 * the connection to be cancelled.
	 */
	 
	void halt();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	QString description();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	Arts::poTime currentTime();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	Arts::poTime overallTime();
	
	/**
	 * returns the capabilities of the PlayObject. The return value is
	 * a binary OR of Arts::capSeek and Arts::capPause, or 0.
	 */
	Arts::poCapabilities capabilities();
	
	/**
	 * Reimplemented (Arts::PlayObject Wrapper)
	 */
	QString mediaName();
	
	/**
	 * returns the internal state of the PlayObject. The state can be
	 * either Arts::posIdle, Arts::posPaused or Arts::posPlaying. A
	 * PlayObject in state Arts::posIdle is stopped. Once you call
	 * play(), the state changes to Arts::posPlaying. pause() causes
	 * the PlayObject to change to Arts::posPaused.
	 */
	Arts::poState state();

signals:
	/**
	 * this signal is emitted as soon as the internal Arts::PlayObject
	 * is created and ready to play. It is granted that the Arts::PlayObject
	 * has not started playing, but KDE::PlayObject will call
	 * object().play() immediately after emitting this signal, so you
	 * need not do it yourself.
	 */
	void playObjectCreated();

private slots:
	void attachPlayObject( Arts::PlayObject );

private:
	Arts::PlayObject m_playObject;
	bool m_isStream;

	struct PrivateData;
	PrivateData* d;

	/* private constructors, to prevent instantiation and copying */
	PlayObject();
	PlayObject( const PlayObject& ) : QObject() {};
	PlayObject(Arts::PlayObject playobject, bool isStream);
	PlayObject( Arts::SoundServerV2 server, const KURL& url, bool isStream, bool createBUS );

	friend class KDE::PlayObjectFactory;

};

}

#endif
