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
#ifndef Kdem2m_IFACES_VIDEOOUTPUTBASE_H
#define Kdem2m_IFACES_VIDEOOUTPUTBASE_H

class QObject;
class QString;

namespace Kdem2m
{
namespace Ifaces
{
	class VideoOutputBase
	{
		public:
			virtual ~VideoOutputBase() {}

			// Attributes Getters:
			virtual QString name() const = 0;

			// Attributes Setters:
			virtual QString setName( const QString& newName ) = 0;

		public:
			virtual QObject* qobject() = 0;
			virtual const QObject* qobject() const = 0;

		private:
			class Private;
			Private* d;
	};
}} //namespace Kdem2m::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_IFACES_VIDEOOUTPUTBASE_H
