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

#include <QObject>

namespace Phonon {
	class MediaObject;
	class AudioPath;
	class FaderEffect;
	class AudioOutput;
}
class KUrl;
using namespace Phonon;

class Crossfader : public QObject
{
	Q_OBJECT
	public:
		Crossfader( QObject* parent = 0 );

	signals:
		void needNextUrl( KUrl& nextUrl );

	public slots:
		void start( const KUrl& firstUrl );
		void stop();

	private slots:
		void crossfade( long );
		void setupNext();

	private:
		MediaObject *m1, *m2;
		AudioPath *a1, *a2;
		FaderEffect *f1, *f2;
		AudioOutput *output;
};
