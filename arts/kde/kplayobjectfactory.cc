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

using namespace std;

KPlayObjectFactory::KPlayObjectFactory(Arts::SoundServerV2 server)
{
	m_server = server;
	m_allowStreaming = true;
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
	    
			// TODO: what else than hardcoding audio/x-mp3 ?
			return new KPlayObject(m_server.createPlayObjectForStream(instream, string("audio/x-mp3"), createBUS), true);
		}
		else
			return new KPlayObject(m_server.createPlayObjectForURL(string(url.path().latin1()), string(mimetype.latin1()), createBUS), false);
	}
	else
		return new KPlayObject();
}
