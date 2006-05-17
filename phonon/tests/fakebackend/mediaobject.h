/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/
#ifndef Phonon_FAKE_MEDIAOBJECT_H
#define Phonon_FAKE_MEDIAOBJECT_H

#include "abstractmediaproducer.h"
#include "../../ifaces/mediaobject.h"
#include <kurl.h>

class KUrl;

namespace Phonon
{
namespace Fake
{
	class MediaObject : public AbstractMediaProducer, virtual public Ifaces::MediaObject
	{
		Q_OBJECT
		public:
			MediaObject( QObject* parent );
			virtual ~MediaObject();
			virtual KUrl url() const;
			virtual qint64 totalTime() const;
			//virtual qint64 remainingTime() const;
			virtual qint32 aboutToFinishTime() const;
			virtual void setUrl( const KUrl& url );
			virtual void setAboutToFinishTime( qint32 newAboutToFinishTime );

			virtual void play();
			virtual void pause();
			virtual void seek( qint64 time );

		public Q_SLOTS:
			virtual void stop();

		Q_SIGNALS:
			void finished();
			void aboutToFinish( qint32 msec );
			void length( qint64 length );

		protected:
			virtual void emitTick();

		private:
			KUrl m_url;
			qint32 m_aboutToFinishTime;
			bool m_aboutToFinishNotEmitted;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_FAKE_MEDIAOBJECT_H
