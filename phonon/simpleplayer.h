/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#ifndef Phonon_SIMPLEPLAYER_H
#define Phonon_SIMPLEPLAYER_H

#include <QObject>
#include "phononnamespace.h"
#include <kdelibs_export.h>

class KUrl;

namespace Phonon
{

class PHONONCORE_EXPORT SimplePlayer : public QObject
{
	Q_OBJECT
	public:
		SimplePlayer( Phonon::Category category, QObject * parent = 0 );
		~SimplePlayer();

		long totalTime() const;
		long currentTime() const;
		float volume() const;

		bool isPlaying() const;
		bool isPaused() const;

	public Q_SLOTS:
		void play( const KUrl & url );
		void play();
		void pause();
		void stop();

		void seek( long ms );
		void setVolume( float volume );

	Q_SIGNALS:
		void finished();

	private Q_SLOTS:
		void stateChanged( Phonon::State, Phonon::State );

	private:
		class Private;
		Private * d;
};

} //namespace Phonon

#endif // Phonon_SIMPLEPLAYER_H
// vim: sw=4 ts=4 noet tw=80
