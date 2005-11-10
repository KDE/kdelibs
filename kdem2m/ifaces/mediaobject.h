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
#ifndef Kdem2m_IFACES_MEDIAOBJECT_H
#define Kdem2m_IFACES_MEDIAOBJECT_H

#include "mediaproducer.h"

class KURL;

namespace Kdem2m
{
namespace Ifaces
{
	/**
	 * \short Interface for accessing media playback functions
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 * \since 4.0
	 */
	class MediaObject : virtual public MediaProducer
	{
		public:
			MediaObject() : MediaProducer() {}
			virtual ~MediaObject() {}

			// Attributes Getters:
			/**
			 * Set the URL the MediaObject should use as media data source.
			 *
			 * @param url The URL to the media data.
			 */
			virtual KURL url() const = 0;
			/**
			 * Get the total time (in milliseconds) of the file currently being played.
			 *
			 * \see length
			 */
			virtual long totalTime() const = 0;
			/**
			 * Get the remaining time (in milliseconds) of the file currently being played.
			 */
			virtual long remainingTime() const = 0;
			virtual long aboutToFinishTime() const = 0;

			// Attributes Setters:
			virtual void setURL( const KURL& url ) = 0;
			virtual long setAboutToFinishTime( long newAboutToFinishTime ) = 0;

		//signals:
			/**
			 * Emitted when the file has finished playing on its own.
			 * I.e. it is not emitted if you call stop(), pause() or
			 * load(), but only on end-of-file or a critical error.
			 */
			virtual void finished() = 0;
			/**
			 * @param msec The remaining time until the playback finishes
			 */
			virtual void aboutToFinish( long msec ) = 0;
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
			virtual void length( long length ) = 0;

		private:
			class Private;
			Private* d;
	};
}} //namespace Kdem2m::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_IFACES_MEDIAOBJECT_H
