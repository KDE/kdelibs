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

#ifndef KPLAYOBJECTFACTORY_H
#define KPLAYOBJECTFACTORY_H

#include <kurl.h>
#include "soundserver.h"
#include "kplayobject.h"

class KPlayObjectFactory
{
public:
	KPlayObjectFactory(Arts::SoundServerV2 server);
	~KPlayObjectFactory();

	KPlayObject *createPlayObject(const KURL& url, bool createBUS);
	KPlayObject *createPlayObject(const KURL& url, const QString &mimetype, bool createBUS);

	void setAllowStreaming(bool s) { m_allowStreaming = s; }
	bool allowStreaming() { return m_allowStreaming; }

	bool isAStream() { return m_stream; }
	
private:
	Arts::SoundServerV2 m_server;
	bool m_allowStreaming;
	bool m_stream;
};


namespace KDE {

	/**
	 * This class implements a factory to create KDE::PlayObjects for
	 * a given URL and mimetype. A detailed description about how to
	 * use the KDE multimedia framework can be found in the documentation
	 * for KDE::PlayObject.
	 */
class PlayObjectFactory
{
public:
	/**
	 * Creates a KDE::PlayObjectFactory. @p server is an 
	 * Arts::SoundServerV2
	 */
	PlayObjectFactory(Arts::SoundServerV2 server);
	~PlayObjectFactory();

	/**
	 * Creates a KDE::PlayObject to play back the file or stream
	 * @p url points to. Set @p createBUS to "true", if you want the
	 * PlayObject to be connected to a SYNTH_BUS_UPLINK on creation.
	 * This is usually the case. You only need to set this to "false"
	 * if you want to attach your own sound effects to the PlayObject.
	 *
	 * You don't need to know the mimetype of the file or stream, it
	 * will be detected automatically.
	 */
	KDE::PlayObject *createPlayObject(const KURL& url, bool createBUS);
	
	/**
	 * This method also creates a KDE::PlayObject, but does no automatic
	 * mimetype detection. Use this method with care.
	 */
	KDE::PlayObject *createPlayObject(const KURL& url, const QString &mimetype, bool createBUS);

	/**
	 * setAllowStreaming( true ) allows the factory to create a
	 * streaming PlayObject for a non-local URL. This is also the default.
	 * Usually, you should not need to set this to false.
	 */
	void setAllowStreaming(bool s) { d->allowStreaming = s; }

	/**
	 * returns "true" if the factory is allowed to create streaming
	 * PlayObjects.
	 */
	bool allowStreaming() { return d->allowStreaming; }

	/**
	 * Return the mimetypes that are playable
	 */
	static QStringList mimeTypes(void);

private:
	struct PrivateData {
		Arts::SoundServerV2 server;
		bool allowStreaming;
		bool isStream;
	};
	PrivateData* d;
};

}
#endif
