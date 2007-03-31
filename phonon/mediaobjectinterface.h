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

#ifndef PHONON_MEDIAOBJECTINTERFACE_H
#define PHONON_MEDIAOBJECTINTERFACE_H

#include <phonon/phonon_export.h>
#include "mediaobject.h"
#include <kurl.h>
#include <QtCore/QObject>

namespace Phonon
{
/**
 * \short Interface for MediaObject objects
 *
 * \ingroup Backend
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONONCORE_EXPORT MediaObjectInterface
{
	public:
		virtual ~MediaObjectInterface() {}
		virtual qint64 totalTime() const = 0;
		virtual KUrl url() const = 0;
		virtual void setUrl( const KUrl& url ) = 0;
        virtual void openMedia(MediaObject::Media media) = 0;
};
}

Q_DECLARE_INTERFACE(Phonon::MediaObjectInterface, "MediaObjectInterface02.phonon.kde.org")

#endif // PHONON_MEDIAOBJECTINTERFACE_H
// vim: sw=4 ts=4 tw=80
