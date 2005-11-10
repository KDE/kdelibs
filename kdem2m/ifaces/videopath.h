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
#ifndef KDEM2M_IFACES_VIDEOPATH_H
#define KDEM2M_IFACES_VIDEOPATH_H

class QObject;

namespace KDEM2M
{
namespace Ifaces
{
	class VideoEffect;
	class VideoOutputBase;

	class VideoPath
	{
		public:
			virtual ~VideoPath() {}

			// Operations:
			virtual bool insertEffect( VideoEffect* newEffect, VideoEffect* insertBefore = 0 ) = 0;
			virtual bool addOutput( VideoOutputBase* videoOutput ) = 0;
			virtual bool removeOutput( VideoOutputBase* videoOutput ) = 0;

		public:
			virtual QObject* qobject() = 0;
			virtual const QObject* qobject() const = 0;

		private:
			class Private;
			Private* d;
	};
}} //namespace KDEM2M::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // KDEM2M_IFACES_VIDEOPATH_H
