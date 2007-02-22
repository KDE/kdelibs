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

#ifndef PHONON_BYTESTREAMINTERFACE_H
#define PHONON_BYTESTREAMINTERFACE_H

#include <kdelibs_export.h>

#include <QtCore/QObject>

namespace Phonon
{
class PHONONCORE_EXPORT ByteStreamInterface
{
	public:
		virtual ~ByteStreamInterface() {}
		virtual qint64 totalTime() const = 0;
		virtual void writeData( const QByteArray& ) = 0;
		virtual void endOfData() = 0;
};
}

Q_DECLARE_INTERFACE( Phonon::ByteStreamInterface, "org.kde.Phonon.ByteStreamInterface/0.1" )

#endif // PHONON_BYTESTREAMINTERFACE_H
// vim: sw=4 ts=4 tw=80
