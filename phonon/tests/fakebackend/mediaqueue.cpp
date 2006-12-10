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

#include "mediaqueue.h"

namespace Phonon
{
namespace Fake
{

MediaQueue::MediaQueue( QObject* parent )
	: QObject( parent )
{
}

KUrl MediaQueue::nextUrl() const
{
	return m_nextUrl;
}

void MediaQueue::setNextUrl( const KUrl &url )
{
	m_nextUrl = url;
}

qint32 MediaQueue::timeBetweenMedia() const
{
	return m_timeBetweenMedia;
}

void MediaQueue::setTimeBetweenMedia( qint32 milliseconds )
{
	m_timeBetweenMedia = milliseconds;
}

bool MediaQueue::doCrossfade() const
{
	return m_doCrossfade;
}

void MediaQueue::setDoCrossfade( bool doCrossfade )
{
	m_doCrossfade = doCrossfade;
}

}} // namespace Phonon::Fake
#include "mediaqueue.moc"
// vim: sw=4 ts=4
