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
#include <phonon/experimental/videoframe.h>
#include <QList>

namespace Phonon
{
namespace Fake
{
	class VideoEffect;
	class AbstractVideoOutput;

	class VideoPath : public QObject
	{
		Q_OBJECT
		public:
			VideoPath( QObject* parent );
			~VideoPath();

			// Operations:
			Q_INVOKABLE bool addOutput( QObject* videoOutput );
			Q_INVOKABLE bool removeOutput( QObject* videoOutput );
			Q_INVOKABLE bool insertEffect( QObject* newEffect, QObject* insertBefore = 0 );
			Q_INVOKABLE bool removeEffect( QObject* effect );

			// fake specific
            void processFrame(Phonon::Experimental::VideoFrame &frame);

		private:
			QList<VideoEffect*> m_effects;
			QList<AbstractVideoOutput*> m_outputs;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_VIDEOPATH_H
