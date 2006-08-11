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

#ifndef PHONON_FAKE_MEDIAQUEUE_H
#define PHONON_FAKE_MEDIAQUEUE_H

#include <QObject>
#include <kurl.h>

namespace Phonon
{
namespace Fake
{

class MediaQueue : public QObject
{
	Q_OBJECT
	public:
		MediaQueue( QObject* parent );

		Q_INVOKABLE KUrl nextUrl() const;
		Q_INVOKABLE void setNextUrl( const KUrl & );

		Q_INVOKABLE qint32 timeBetweenMedia() const;
		Q_INVOKABLE void setTimeBetweenMedia( qint32 milliseconds );

		Q_INVOKABLE bool doCrossfade() const;
		Q_INVOKABLE void setDoCrossfade( bool doCrossfade );

	Q_SIGNALS:
		void needNextMediaObject();

	private:
		qint32 m_timeBetweenMedia;
		bool m_doCrossfade;
		KUrl m_nextUrl;
};

}} // namespace Phonon::Fake

#endif // PHONON_FAKE_MEDIAQUEUE_H
// vim: sw=4 ts=4 tw=80 noet
