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

#ifndef PHONON_MEDIAQUEUE_H
#define PHONON_MEDIAQUEUE_H

#include <kdelibs_export.h>
#include "mediaobject.h"

namespace Phonon
{

class MediaQueuePrivate;
/**
 * \brief Class that queues playback of media with high precision (e.g. gapless
 * playback)
 *
 * Using a %MediaQueue object you can achieve high precision queuing of media
 * doing either a predefined pause between tracks, gapless playback, overlapping
 * playback or crossfades.
 *
 * In general you'll set up one %MediaQueue object and connect to the
 * needNextMediaObject signal. When this signal is emitted you have to provide
 * the next MediaObject calling setNext.
 * \code
 * m_mediaQueue = new MediaQueue( this );
 * m_mediaQueue->setUrl( m_playlist->nextUrl() );
 * m_mediaQueue->setNextUrl( m_playlist->nextUrl() );
 * connect( m_mediaQueue, SIGNAL(needNextUrl()), SLOT(prepareNextUrl()) );
 * \endcode
 *
 * The slot could look like this:
 * \code
 * void MyClass::prepareNextUrl()
 * {
 *   m_mediaQueue->setNextUrl( m_playlist->nextUrl() );
 * }
 * \endcode
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONONCORE_EXPORT MediaQueue : public MediaObject
{
	K_DECLARE_PRIVATE( MediaQueue )
	Q_OBJECT
	PHONON_HEIR( MediaQueue )

	/**
	 * Time between the end of the first media and the start of the next
	 * media. A value of 0ms means gapless playback. A value less than zero
	 * means that the next song will start playing while the first is about
	 * to finish.
	 *
	 * Defaults to 0 (gapless playback)
	 */
	Q_PROPERTY( qint32 timeBetweenMedia READ timeBetweenMedia WRITE setTimeBetweenMedia )

	/**
	 * If the timeBetweenMedia is less than 0 this property determines whether a
	 * crossfade is done or not.
	 *
	 * Defaults to \c true (crossfades if timeBetweenMedia < 0)
	 */
	Q_PROPERTY( bool doCrossfade READ doCrossfade WRITE setDoCrossfade )

	/**
	 * The URL for the media to be played after the current one.
	 */
	Q_PROPERTY( KUrl nextUrl READ nextUrl WRITE setNextUrl )
	public:
		KUrl nextUrl() const;
		void setNextUrl( const KUrl& nextUrl );

		qint32 timeBetweenMedia() const;
		void setTimeBetweenMedia( qint32 milliseconds );

		bool doCrossfade() const;
		void setDoCrossfade( bool doCrossfade );

	Q_SIGNALS:
		void needNextUrl();

	private:
		Q_PRIVATE_SLOT( k_func(), void _k_needNextUrl() )
};

} // namespace Phonon

#endif // PHONON_MEDIAQUEUE_H

// vim: sw=4 ts=4 tw=80 noet
