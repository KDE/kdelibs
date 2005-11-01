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
	the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.

	*/

#include <kio/kmimetype.h>
#include "kplayobject.h"
#include "artskde.h"
#include "kplayobjectfactory.h"
#include "kplayobjectfactory_p.h"
#include "kplayobjectcreator.h"
#include "kioinputstream_impl.h"
#include "kartsdispatcher.h"
#include "kartsserver.h"

#include <qfile.h>
#include <kdebug.h>
#include "kaudiomanagerplay.h"
#include <flowsystem.h>
#include <kio/netaccess.h>

using namespace std;

KPlayObjectFactory::KPlayObjectFactory(Arts::SoundServerV2 server)
{
	m_server = server;
	m_allowStreaming = true;
	m_stream = false;
}

KPlayObjectFactory::KPlayObjectFactory(KArtsServer* server)
{
	m_server = server->server();
	m_allowStreaming = true;
	m_stream = false;
}

KPlayObjectFactory::~KPlayObjectFactory()
{
}

KPlayObject *KPlayObjectFactory::createPlayObject(const KURL& url, bool createBUS)
{
	KMimeType::Ptr mimetype = KMimeType::findByURL(url);
	return createPlayObject(url, mimetype->name(), createBUS);
}


KPlayObject *KPlayObjectFactory::createPlayObject(const KURL& _url, const QString &mimetype, bool createBUS)
{
	// WHY DOES BROKEN KIO_MEDIA GIVE WRONG URLS?
	// I hate it
	// I hate it
	// It sucks
	// kio_media please die
	KURL url = KIO::NetAccess::mostLocalURL(_url, 0);

	if(!m_server.isNull())
	{
		if(mimetype == "application/octet-stream" && m_allowStreaming)
		{
			Arts::KIOInputStream instream;
			instream.openURL(url.url().latin1());

			m_stream = true;

			// TODO: what else than hardcoding audio/x-mp3 ?
			return new KPlayObject(m_server.createPlayObjectForStream(instream, string("audio/x-mp3"), createBUS), true);
		}
		else
			return new KPlayObject(m_server.createPlayObjectForURL(string(QFile::encodeName(url.path())), string(mimetype.latin1()), createBUS), false);
	}
	else
		return new KPlayObject();
}



//

KDE::PlayObjectFactory::PlayObjectFactory(Arts::SoundServerV2 server)
{
	d = new PrivateData;
	d->server = server;
	d->amanPlay = 0;
	d->helper = 0;
	d->allowStreaming = true;
	d->isStream = false;
}

KDE::PlayObjectFactory::PlayObjectFactory(KArtsServer* server)
{
	d = new PrivateData;
	d->server = server->server();
	d->amanPlay = 0;
	d->helper = 0;
	d->allowStreaming = true;
	d->isStream = false;
}

KDE::PlayObjectFactory::~PlayObjectFactory()
{
	delete d->helper;
	delete d;
}

void KDE::PlayObjectFactory::setAudioManagerPlay( KAudioManagerPlay * amanPlay )
{
	d->amanPlay = amanPlay;
	if( ! d->helper )
		d->helper = new POFHelper;
}

KDE::PlayObject *KDE::PlayObjectFactory::createPlayObject(const KURL& url, bool createBUS)
{
	KMimeType::Ptr mimetype = KMimeType::findByURL(url);
	return createPlayObject(url, mimetype->name(), createBUS);
}

KDE::PlayObject *KDE::PlayObjectFactory::createPlayObject(const KURL& _url, const QString &mimetype, bool createBUS)
{
	// WHY DOES BROKEN KIO_MEDIA GIVE WRONG URLS?
	// I hate it
	// I hate it
	// It sucks
	// kio_media please die
	KURL url = KIO::NetAccess::mostLocalURL(_url, 0);
	
	// return a NULL playobject if the server is NULL
	if ( d->server.isNull() || url.isEmpty() )
		return new KDE::PlayObject();

	// if the program wants to use it's own Synth_AMAN_PLAY we don't need a
	// bus
	if( d->amanPlay && createBUS )
	{
		kdWarning( 400 ) << "KDE::PlayObjectFactory was instructed to use a Synth_AMAN_PLAY for output but the program also asked for a Synth_BUS_UPLINK" << endl;
		createBUS = false;
	}

	// decide if it's a local file. mpeglib provides cdda reading and decoding, so we prefer that over kio_audiocd
	if ( url.isLocalFile() || !d->allowStreaming || (url.protocol() == "audiocd" && mimetype == "application/x-cda" && mimeTypes().contains( "application/x-cda" ) ) )
	{
		// we rely on the delivered mimetype if it's a local file
		d->playObj = new KDE::PlayObject( d->server.createPlayObjectForURL( string( QFile::encodeName( url.path() ) ), string( mimetype.latin1() ), createBUS ), false );
	}
	else
	{
		// if non-local, let the KPlayObject figure out the mimetype itself
		// this invokes asynchronous creation automatically
		d->playObj = new KDE::PlayObject( d->server, url, true, createBUS );
	}

	if( d->playObj->isNull() )
	{
		delete d->playObj;
		d->playObj = 0;
		return new KDE::PlayObject(); // return a NULL playobject
	}

	if( d->amanPlay )
	{
		d->helper->po = d->playObj;
		d->helper->ap = d->amanPlay;
		if( d->playObj->object().isNull() && d->amanPlay )
			QObject::connect( d->playObj, SIGNAL( playObjectCreated() ), d->helper, SLOT( connectAmanPlay() ) );
		else
			d->helper->connectAmanPlay();
	}

	return d->playObj;
}

QStringList KDE::PlayObjectFactory::mimeTypes(void)
{
	KArtsDispatcher dispatcher; // we need such a thing, otherwise we crash
	Arts::TraderQuery query;
	vector<Arts::TraderOffer> *offers = query.query();

	QStringList results;
	for(vector<Arts::TraderOffer>::iterator offer = offers->begin();
	    offer != offers->end(); ++offer)
	{
		vector<string> *mimetypes = (*offer).getProperty("MimeType");

		for(vector<string>::iterator mimetype = mimetypes->begin();
		    mimetype != mimetypes->end(); ++mimetype)
		{
			QString name = QString::fromLocal8Bit((*mimetype).c_str()).stripWhiteSpace();
			if(KMimeType::mimeType(name))
				results.append(name);
		}

		delete mimetypes;
	}
	delete offers;

	// clean out duplicates
	results.sort();
	for(QStringList::iterator result = results.begin(); result != results.end(); )
	{
		QStringList::iterator previous = result;
		++result;
		if(result != results.end() && *result == *previous)
		{
			results.remove(result);
			result = previous;
		}
	}

	return results;
}

/* ### KDE4
void KDE::PlayObjectFactory::connectAmanPlay()
{
	kdDebug( 400 ) << k_funcinfo << endl;
	if( d->playObj->object().isNull() )
		return;

	d->amanPlay->start();
	d->playObj->object()._node()->start();
	Arts::connect( d->playObj->object(), "left" , d->amanPlay->amanPlay(), "left" );
	Arts::connect( d->playObj->object(), "right", d->amanPlay->amanPlay(), "right" );
}
*/

void KDE::POFHelper::connectAmanPlay()
{
	kdDebug( 400 ) << k_funcinfo << endl;
	if( po->object().isNull() )
		return;

	ap->start();
	po->object()._node()->start();
	Arts::connect( po->object(), "left" , ap->amanPlay(), "left" );
	Arts::connect( po->object(), "right", ap->amanPlay(), "right" );
}

#include "kplayobjectfactory_p.moc"

// vim: sw=4 ts=4 noet
