/*  This file is part of the KDE project
    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

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
#ifndef Kdem2m_MEDIAOBJECT_H
#define Kdem2m_MEDIAOBJECT_H

#include "mediaproducer.h"
#include <kmimetype.h>

class KURL;

namespace Kdem2m
{
	namespace Ifaces
	{
		class MediaObject;
	}

	/**
	 * \short Interface for accessing media playback functions
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 * \since 4.0
	 */
	class MediaObject : public MediaProducer
	{
		Q_OBJECT
		public:
			MediaObject( const KURL& url, QObject* parent = 0 );
			~MediaObject();

			// Attributes Getters:
			/**
			 * Set the URL the MediaObject should use as media data source.
			 *
			 * @param url The URL to the media data.
			 */
			KURL url() const;
			/**
			 * Get the total time (in milliseconds) of the file currently being played.
			 *
			 * \see length
			 */
			long totalTime() const;
			/**
			 * Get the remaining time (in milliseconds) of the file currently being played.
			 */
			long remainingTime() const;
			long aboutToFinishTime() const;

		public slots:
			// Attributes Setters:
			void setAboutToFinishTime( long newAboutToFinishTime );

		signals:
			/**
			 * Emitted when the file has finished playing on its own.
			 * I.e. it is not emitted if you call stop(), pause() or
			 * load(), but only on end-of-file or a critical error.
			 */
			void finished();
			/**
			 * @param msec The remaining time until the playback finishes
			 */
			void aboutToFinish( long msec );
			/**
			 * This signal is emitted as soon as the length of the media file is
			 * known or has changed. For most non-local media data the length of
			 * the media can only be known after some time. At that time the
			 * totalTime function can not return usefull information. You have
			 * to wait for this signal to know the real length.
			 *
			 * @param length The length of the media file in milliseconds.
			 *
			 * \see totalTime
			 */
			void length( long length );

		protected:
			MediaObject( Ifaces::MediaProducer* iface, const KURL& url, QObject* parent );
			virtual bool aboutToDeleteIface();
			virtual void ifaceDeleted();
			virtual Ifaces::MediaProducer* createIface( bool initialized = true );
			virtual void setupIface();

		private:
			Ifaces::MediaObject* iface();
			Ifaces::MediaObject* m_iface;
			class Private;
			Private* d;
	};
} //namespace Kdem2m

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_MEDIAOBJECT_H
