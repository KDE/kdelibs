	/*

	Copyright (C) 2002 Matthias Welwarsky <mwelwarsky@web.de>

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
#include "artskde.h"
#include "kplayobjectcreator.h"
#include "kplayobjectcreator.moc"
#include "kioinputstream_impl.h"

#include <qfile.h>

#include <kdebug.h>

KDE::PlayObjectCreator::PlayObjectCreator(Arts::SoundServerV2 server)
{
	m_server = server;
}

KDE::PlayObjectCreator::~PlayObjectCreator()
{
}

bool KDE::PlayObjectCreator::create(const KURL& url, bool createBUS, const QObject* receiver, const char* slot)
{
	// no need to go any further, and I hate deep indentation
	if (m_server.isNull() || url.isEmpty() )
		return false;

	connect( this, SIGNAL( playObjectCreated( Arts::PlayObject ) ),
			receiver, slot );

	// check if the URL is a local file
	if (!url.isLocalFile())
	{
		m_createBUS = createBUS;

		// This is the RightWay(tm) according to stw
		Arts::KIOInputStream_impl* instream_impl = new Arts::KIOInputStream_impl();
		m_instream = Arts::KIOInputStream::_from_base(instream_impl);

		// signal will be called once the ioslave knows the mime-type of the stream
		connect(instream_impl, SIGNAL(mimeTypeFound(const QString &)),
				 this, SLOT(slotMimeType(const QString &)));

		// GO!
		m_instream.openURL(url.url().latin1());
		m_instream.streamStart();

		return true;
	}
	kdDebug() << "stream is local file: " << url.url() << endl;

	// usual stuff if we have a local file
	KMimeType::Ptr mimetype = KMimeType::findByURL(url);
	emit playObjectCreated (
		m_server.createPlayObjectForURL(string(QFile::encodeName(url.path())), 
						 string(mimetype->name().latin1()), 
						 createBUS)
		);
	return true;
}

void KDE::PlayObjectCreator::slotMimeType(const QString& mimetype)
{

	kdDebug() << "slotMimeType called: " << mimetype << endl;

	QString mimetype_copy = mimetype;

	if ( mimetype_copy == "application/octet-stream" )
	    mimetype_copy = QString("audio/x-mp3");

	if (mimetype_copy == "application/x-zerosize")
		emit playObjectCreated(Arts::PlayObject::null());

	emit playObjectCreated (
		m_server.createPlayObjectForStream(
		m_instream, string(mimetype_copy.latin1()), m_createBUS)
		);
}
