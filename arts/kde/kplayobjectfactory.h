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

	KPlayObject *createPlayObject(KURL url, bool createBUS);
	KPlayObject *createPlayObject(KURL url, const QString &mimetype, bool createBUS);

	bool setAllowStreaming(bool s) { m_allowStreaming = s; }
	bool allowStreaming() { return m_allowStreaming; }

private:
	Arts::SoundServerV2 m_server;
	bool m_allowStreaming;
};

#endif
