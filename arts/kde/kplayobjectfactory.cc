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
#include "kplayobjectfactory.moc"
#include "kioinputstream_impl.h"

#include <qfile.h>
#include <kapplication.h>

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

	// no need to go any further, and I hate deep indentation
	if (m_server.isNull() || url.isEmpty() )
		return new KPlayObject();

	// check if streaming is allowed or the URL is a local file
	if (m_allowStreaming && !url.isLocalFile())
	{
		// This is the RightWay(tm) according to stw
		Arts::KIOInputStream_impl* instream_impl = new Arts::KIOInputStream_impl();
		Arts::KIOInputStream instream = Arts::KIOInputStream::_from_base(instream_impl);

		// signal will be called once the ioslave knows the mime-type of the stream
		QObject::connect(instream_impl, SIGNAL(mimeTypeFound(const QString &)),
				 this, SLOT(slotMimeType(const QString &)));

		// GO!
		instream.openURL(url.url().latin1());
		instream.streamStart();

		m_eventLoopEntered = true;
		// FIXME: need to handle timeouts?
		// this is UGLY, but I cannot do anything else without changing the interface
		// the application blocks here, and restarts once slotMimeType() is called.
		kapp->enter_loop();
		
		// wb :D
		// some error occoured
		if (m_mimeType == "application/x-zerosize")
		    return new KPlayObject();

		// ok, now we know the mimetype of the stream
		m_stream = true;		
		return new KPlayObject(m_server.createPlayObjectForStream(instream, 
									  string(m_mimeType.latin1()), 
									  createBUS), true);
	}
	kdDebug() << "stream is local file: " << url.url() << endl;

	// usual stuff if we have a local file
	KMimeType::Ptr mimetype = KMimeType::findByURL(url);
	return new KPlayObject(m_server.createPlayObjectForURL(string(QFile::encodeName(url.path())), 
							       string(mimetype->name().latin1()), 
							       createBUS), 
			       false);
}

void KPlayObjectFactory::slotMimeType(const QString& mimetype)
{

	kdDebug() << "slotMimeType called: " << mimetype << endl;

	if ( mimetype == "application/octet-stream" )
	    m_mimeType = "audio/x-mp3";
	else
	    m_mimeType = mimetype;

	if (m_eventLoopEntered) {
		m_eventLoopEntered = false;
		kapp->exit_loop();
	}
}

KPlayObject *KPlayObjectFactory::createPlayObject(const KURL& url, const QString &mimetype, bool createBUS)
{
	if(!m_server.isNull())
	{
		kdDebug() << "createPlayObject: mimetype is " << mimetype << endl;

		if(mimetype == "application/octet-stream" && m_allowStreaming)
		{
			Arts::KIOInputStream instream;
			instream.openURL(url.url().latin1());
	    
			m_stream = true;
		
			// TODO: what else than hardcoding audio/x-mp3 ?
			return new KPlayObject(m_server.createPlayObjectForStream(instream, string("audio/x-mp3"), createBUS), true);
		}
		else if(mimetype == "video/mpeg" && m_allowStreaming)
		{
			Arts::KIOInputStream instream;
			instream.openURL(url.url().latin1());

			m_stream = true;

			return new KPlayObject(m_server.createPlayObjectForStream(instream, string("video/mpeg"), createBUS), true);
		}
		else
			return new KPlayObject(m_server.createPlayObjectForURL(string(QFile::encodeName(url.path())), string(mimetype.latin1()), createBUS), false);
	}
	else
		return new KPlayObject();
}
