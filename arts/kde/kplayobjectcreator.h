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
	the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.

	*/

#ifndef KPLAYOBJECTCREATOR_H
#define KPLAYOBJECTCREATOR_H

#include <kurl.h>
#include <qobject.h>
#include "soundserver.h"
#include "kioinputstream_impl.h"

namespace KDE {

class PlayObjectCreator : public QObject
{
Q_OBJECT
public:
	PlayObjectCreator(Arts::SoundServerV2 server);
	~PlayObjectCreator();

	bool create(const KURL& url, bool createBUS, const QObject* receiver, const char* slot);

Q_SIGNALS:
	void playObjectCreated(Arts::PlayObject playObject);

private Q_SLOTS:
	void slotMimeType(const QString &mimetype);

private:
	Arts::SoundServerV2 m_server;
	Arts::KIOInputStream m_instream;
	Arts::PlayObject playObject;
	bool m_createBUS;
};

}
#endif

