	/*

	Copyright (C) 2002 Nikolas Zimmermann <wildfox@kde.org>

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

#ifndef KAUDIOCONVERTER_H
#define KAUDIOCONVERTER_H

#include <qobject.h>
#include "artskde.h"
#include "kplayobject.h"

class KUrl;
class QString;

class KAudioConverter : public QObject
{
Q_OBJECT
public:
	KAudioConverter();
	~KAudioConverter();

	bool setup(int samplingRate);
	void requestPlayObject(const KUrl &url);

	void start();
	void stop();

Q_SIGNALS:
	void rawStreamStart();

	void newBlockSize(long blockSize);
	void newBlockPointer(long blockPointer);
	void newData();
	
	void rawStreamFinished();

private Q_SLOTS:
	void slotMimeType(const QString &mimeType);
	
private:
	Arts::KDataRequest m_request;
	KPlayObject *m_incoming;
	QString m_mimeType;

	bool m_started;
};

#endif
