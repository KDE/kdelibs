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

#include <kmimetype.h>
#include "kplayobject.h"
#include "kplayobjectfactory.h"

KPlayObjectFactory::KPlayObjectFactory(Arts::SimpleSoundServer server)
{
    m_factory = Arts::DynamicCast(server.createObject("Arts::KWrapperFactory"));
}

KPlayObjectFactory::~KPlayObjectFactory()
{
}

KPlayObject *KPlayObjectFactory::createPlayObject(KURL url, bool createBUS)
{
    QString mimetypename;
    bool isStream;
    if(!m_factory.isNull())
    {
	KMimeType::Ptr mimetype = KMimeType::findByURL(url);
	// ICEcast/SHOUTcast
	if(mimetype->name() == "application/octet-stream")
	{
	    mimetypename = "audio/x-mp3";
	    isStream = true;
	}
	else
	{
	    mimetypename = mimetype->name();
	    isStream = false;
	}
	return new KPlayObject(m_factory.createPlayObject(string(url.path().latin1()), string(mimetypename.latin1()), createBUS), isStream);
    }
    else
	return new KPlayObject();
}
