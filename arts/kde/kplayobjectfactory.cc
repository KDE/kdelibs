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

#include <kio/kmimetype.h>
#include "kplayobject.h"
#include "artskde.h"
#include "kplayobjectfactory.h"
#include "kplayobjectcreator.h"
#include "kioinputstream_impl.h"

#include <qfile.h>
#include <kdebug.h>

using namespace std;

KPlayObjectFactory::KPlayObjectFactory(Arts::SoundServerV2 server)
{
	m_server = server;
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

KPlayObject *KPlayObjectFactory::createPlayObject(const KURL& url, const QString &mimetype, bool createBUS)
{
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
	d->allowStreaming = true;
	d->isStream = false;
}

KDE::PlayObjectFactory::~PlayObjectFactory()
{
	delete d;
}

KDE::PlayObject *KDE::PlayObjectFactory::createPlayObject(const KURL& url, bool createBUS)
{
	KMimeType::Ptr mimetype = KMimeType::findByURL(url);
	return createPlayObject(url, mimetype->name(), createBUS);
}

KDE::PlayObject *KDE::PlayObjectFactory::createPlayObject(const KURL& url, const QString &mimetype, bool createBUS)
{

	// return a NULL playobject if the server is NULL
	if ( d->server.isNull() || url.isEmpty() )
		return new KDE::PlayObject();

	// decide if it's a local file
	if ( url.isLocalFile() || !d->allowStreaming ) {
		// we rely on the delivered mimetype if it's a local file
		return new KDE::PlayObject( d->server.createPlayObjectForURL( string( QFile::encodeName( url.path() ) ), string( mimetype.latin1() ), createBUS ), false );
	}
	// if non-local, let the KPlayObject figure out the mimetype itself
	// this invokes asynchronous creation automatically
	return new KDE::PlayObject( d->server, url, true, createBUS );
}

QStringList KDE::PlayObjectFactory::mimeTypes(void)
{
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
