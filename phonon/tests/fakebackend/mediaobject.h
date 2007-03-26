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
#include <kurl.h>
#include <phonon/mediaobjectinterface.h>

class KUrl;

namespace Phonon
{
namespace Fake
{
	class MediaObject : public AbstractMediaProducer, public MediaObjectInterface
	{
		Q_OBJECT
		Q_INTERFACES( Phonon::MediaObjectInterface )
		public:
			MediaObject( QObject* parent );
			~MediaObject();
			Q_INVOKABLE KUrl url() const;
			Q_INVOKABLE qint64 totalTime() const;
			Q_INVOKABLE qint32 aboutToFinishTime() const;
			Q_INVOKABLE void setUrl( const KUrl& url );
			Q_INVOKABLE void setAboutToFinishTime( qint32 newAboutToFinishTime );

			Q_INVOKABLE void play();
			Q_INVOKABLE void pause();
			Q_INVOKABLE void seek( qint64 time );

		public Q_SLOTS:
			void stop();

		Q_SIGNALS:
			void finished();
			void aboutToFinish( qint32 msec );
			void length( qint64 length );

		protected:
			virtual void emitTick();

        private Q_SLOTS:
            void loadingComplete();

		private:
			KUrl m_url;
			qint32 m_aboutToFinishTime;
			bool m_aboutToFinishNotEmitted;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_MEDIAOBJECT_H
