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

#include <QObject>
#include <kdelibs_export.h>
#include "base.h"
#include "mediaobject.h"

namespace Phonon
{
namespace Ifaces
{
	class MediaQueue;
}

class MediaQueuePrivate;
class PHONONCORE_EXPORT MediaQueue : public QObject, public Base
{
	K_DECLARE_PRIVATE( MediaQueue )
	Q_OBJECT
	PHONON_OBJECT( MediaQueue )
	Q_PROPERTY( qint32 timeBetweenMedia READ timeBetweenMedia WRITE setTimeBetweenMedia )
	Q_PROPERTY( bool doCrossfade READ doCrossfade WRITE setDoCrossfade )
	public:
		MediaObject* current() const;
		MediaObject* next() const;

		void setNext( MediaObject* next );

		qint32 timeBetweenMedia() const;
		/**
		 * Time between the end of the first media and the start of the next
		 * media. A value of 0ms means gapless playback. A value less than zero
		 * means that the next song will start playing while the first is about
		 * to finish.
		 */
		void setTimeBetweenMedia( qint32 milliseconds );

		bool doCrossfade() const;
		void setDoCrossfade( bool doCrossfade );

	Q_SIGNALS:
		void needNextMediaObject();

	private:
		Q_PRIVATE_SLOT( k_func(), void _k_needNextMediaObject() )
};

} // namespace Phonon

#endif // PHONON_MEDIAQUEUE_H

// vim: sw=4 ts=4 tw=80 noet
