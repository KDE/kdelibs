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
#ifndef Kdem2m_IFACES_AUDIOPATH_H
#define Kdem2m_IFACES_AUDIOPATH_H

class QObject;

namespace Kdem2m
{
namespace Ifaces
{
	class AudioEffect;
	class AudioOutputBase;

	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 * \since 4.0
	 */
	class AudioPath
	{
		public:
			virtual ~AudioPath() {}

			// Operations:
			virtual bool insertEffect( AudioEffect* newEffect, AudioEffect* insertBefore = 0 ) = 0;
			virtual bool addOutput( AudioOutputBase* audioOutput ) = 0;
			virtual bool removeOutput( AudioOutputBase* audioOutput ) = 0;

		public:
			virtual QObject* qobject() = 0;
			virtual const QObject* qobject() const = 0;

		private:
			class Private;
			Private* d;
	};
}} //namespace Kdem2m::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_IFACES_AUDIOPATH_H
