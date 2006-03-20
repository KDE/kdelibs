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
#ifndef Phonon_FAKE_VIDEOPATH_H
#define Phonon_FAKE_VIDEOPATH_H

#include <QObject>
#include <../../ifaces/videopath.h>
#include <QList>

namespace Phonon
{
namespace Fake
{
	class VideoEffect;
	class AbstractVideoOutput;

	class VideoPath : public QObject, virtual public Ifaces::VideoPath
	{
		Q_OBJECT
		public:
			VideoPath( QObject* parent );
			virtual ~VideoPath();

			// Operations:
			virtual bool addOutput( Ifaces::AbstractVideoOutput* videoOutput );
			virtual bool removeOutput( Ifaces::AbstractVideoOutput* videoOutput );
			virtual bool insertEffect( Ifaces::VideoEffect* newEffect, Ifaces::VideoEffect* insertBefore = 0 );
			virtual bool removeEffect( Ifaces::VideoEffect* effect );

		public:
			virtual QObject* qobject() { return this; }
			virtual const QObject* qobject() const { return this; }

		private:
			QList<VideoEffect*> m_effects;
			QList<Ifaces::AbstractVideoOutput*> m_outputs;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_FAKE_VIDEOPATH_H
