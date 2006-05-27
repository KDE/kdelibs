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

#ifndef PHONON_IFACES_MEDIAQUEUE_H
#define PHONON_IFACES_MEDIAQUEUE_H

#include <QObject>
#include <kdelibs_export.h>
#include "base.h"

namespace Phonon
{
namespace Ifaces
{
class MediaObject;

class MediaQueue : public Base
{
	public:
		virtual void setNext( Ifaces::MediaObject* ) = 0;

		virtual qint32 timeBetweenMedia() const = 0;
		virtual void setTimeBetweenMedia( qint32 milliseconds ) = 0;

		virtual bool doCrossfade() const = 0;
		virtual void setDoCrossfade( bool doCrossfade ) = 0;

	protected:
	//Q_SIGNALS:
		virtual void needNextMediaObject() = 0;
};

}} // namespace Phonon::Ifaces

#endif // PHONON_IFACES_MEDIAQUEUE_H

// vim: sw=4 ts=4 tw=80 noet
