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

#include "kplayobject.h"
#include "kplayobject.moc"
#include "kplayobjectcreator.h"
#include <kdebug.h>

KPlayObject::KPlayObject() : QObject()
{
	m_playObject = Arts::PlayObject::null();
	m_isStream = false;
}

KPlayObject::KPlayObject(Arts::PlayObject playobject, bool isStream) : QObject()
{
	m_playObject = playobject;
	m_isStream = isStream;
}

KPlayObject::~KPlayObject()
{
}

void KPlayObject::play()
{
	object().play();
}

void KPlayObject::seek(Arts::poTime newTime)
{
	if(!m_isStream)
		object().seek(newTime);
	else
		kdDebug() << "Seeking in a Stream? huh?" << endl;
}

void KPlayObject::pause()
{
	object().pause();
}

void KPlayObject::halt()
{
	object().halt();
}

QString KPlayObject::description()
{
	return QString::fromLatin1(object().description().c_str());
}

Arts::poTime KPlayObject::currentTime()
{
	return object().currentTime();
}

Arts::poTime KPlayObject::overallTime()
{
	return object().overallTime();
}

Arts::poCapabilities KPlayObject::capabilities()
{
	return object().capabilities();
}

QString KPlayObject::mediaName()
{
	return QString::fromLatin1(object().mediaName().c_str());
}

Arts::poState KPlayObject::state()
{
	return object().state();
}

Arts::PlayObject KPlayObject::object()
{
	return m_playObject;
}

bool KPlayObject::isNull()
{
	return object().isNull();
}

void KPlayObject::setObject(Arts::PlayObject playObject)
{
	m_playObject = playObject;
}

bool KPlayObject::stream()
{
	return m_isStream;
}

struct KDE::PlayObject::PrivateData
{
	PrivateData() : creator( 0 ), 
			isProxy( false ),
			internalState( Arts::posIdle ) {}
	~PrivateData() {
		delete creator;
	}
	Arts::SoundServerV2 server;
	KDE::PlayObjectCreator* creator;
	bool createBUS;
	bool isProxy;
	Arts::poState internalState;
	KURL url;
};

KDE::PlayObject::PlayObject() : QObject()
{
	m_playObject = Arts::PlayObject::null();
	m_isStream = false;
	d = new PrivateData;
}

KDE::PlayObject::PlayObject(Arts::PlayObject playobject, bool isStream) : QObject()
{
	m_playObject = playobject;
	m_isStream = isStream;
	d = new PrivateData;
	emit playObjectCreated();
}

KDE::PlayObject::PlayObject( Arts::SoundServerV2 server, const KURL& url, bool isStream, bool createBUS ) : QObject()
{
	kdDebug( 400 ) << "KDE::PlayObject: created as proxy for URL " << url.url()<< endl;

	m_playObject = Arts::PlayObject::null();
	m_isStream = isStream;
	d = new PrivateData;
	d->server = server;
	d->url = url;
	d->createBUS = createBUS;
	d->isProxy = true;
}

KDE::PlayObject::~PlayObject()
{
	kdDebug( 400 ) << "KDE::PlayObject: destroyed" << endl;

	delete d;
}

void KDE::PlayObject::play()
{
	kdDebug( 400 ) << "KDE::PlayObject::play()" << endl;

	if ( object().isNull() ) {
		if ( m_isStream ) {
			if ( d->creator )
				delete d->creator;
			d->creator = new KDE::PlayObjectCreator( d->server );
			d->creator->create( d->url, d->createBUS, this, SLOT( attachPlayObject( Arts::PlayObject ) ) );
			kdDebug( 400 ) << "KDE::PlayObject::play(): creator called" << endl;
			d->internalState = Arts::posPlaying;
		}
		return;
	}
	object().play();
}

void KDE::PlayObject::attachPlayObject( Arts::PlayObject playObject )
{
	kdDebug( 400 ) << "KDE::PlayObject::attachPlayObject()" << endl;

	m_playObject = playObject;
	emit playObjectCreated();
	switch ( d->internalState ) {
	case Arts::posIdle:
		object().halt();
		break;
	case Arts::posPaused:
		object().pause();
		break;
	case Arts::posPlaying:
		object().play ();
		break;
	}
}

void KDE::PlayObject::seek(Arts::poTime newTime)
{
	if ( object().isNull() )
		return;
	if(!m_isStream)
		object().seek(newTime);
	else
		kdDebug( 400 ) << "Seeking in a Stream? huh?" << endl;
}

void KDE::PlayObject::pause()
{
	if ( !object().isNull() )
		object().pause();
	d->internalState = Arts::posPaused;
}

void KDE::PlayObject::halt()
{
	kdDebug( 400 ) << "KDE::PlayObject::halt()" << endl;
	if ( !object().isNull() )
		object().halt();
	else if ( d->creator ) {
		delete d->creator;
		d->creator = 0;
		kdDebug( 400 ) << "KDE::PlayObject::halt(): creator destroyed" << endl;
	}
	d->internalState = Arts::posIdle;
}

QString KDE::PlayObject::description()
{
	if ( object().isNull() )
		return QString();
	return QString::fromLatin1(object().description().c_str());
}

Arts::poTime KDE::PlayObject::currentTime()
{
	if ( object().isNull() )
		return Arts::poTime( 0, 0, -1, "" );
	return object().currentTime();
}

Arts::poTime KDE::PlayObject::overallTime()
{
	if ( object().isNull() )
		return Arts::poTime( 0, 0, -1, "" );
	return object().overallTime();
}

Arts::poCapabilities KDE::PlayObject::capabilities()
{
	if ( object().isNull() )
		return static_cast<Arts::poCapabilities>( 0 );
	return object().capabilities();
}

QString KDE::PlayObject::mediaName()
{
	if ( object().isNull() )
		return QString();
	return QString::fromLatin1(object().mediaName().c_str());
}

Arts::poState KDE::PlayObject::state()
{
	if ( object().isNull() )
		return d->internalState;
	return object().state();
}

Arts::PlayObject KDE::PlayObject::object()
{
	return m_playObject;
}

bool KDE::PlayObject::isNull()
{
	if ( !this )
		return true;
	if ( d->isProxy )
		return false;
	return object().isNull();
}

bool KDE::PlayObject::stream()
{
	return m_isStream;
}
