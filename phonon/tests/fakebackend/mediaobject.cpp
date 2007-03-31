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

#include "mediaobject.h"
#include <QTimer>
#include <kdebug.h>

namespace Phonon
{
namespace Fake
{
MediaObject::MediaObject( QObject* parent )
	: AbstractMediaProducer( parent )
	, m_aboutToFinishNotEmitted( true )
{
	//kDebug( 604 ) << k_funcinfo << endl;
}

MediaObject::~MediaObject()
{
	//kDebug( 604 ) << k_funcinfo << endl;
}

KUrl MediaObject::url() const
{
	//kDebug( 604 ) << k_funcinfo << endl;
	return m_url;
}

qint64 MediaObject::totalTime() const
{
	//kDebug( 604 ) << k_funcinfo << endl;
	return 1000*60*3; // 3 minutes
}

qint32 MediaObject::aboutToFinishTime() const
{
	//kDebug( 604 ) << k_funcinfo << endl;
	return m_aboutToFinishTime;
}

void MediaObject::setUrl( const KUrl& url )
{
	//kDebug( 604 ) << k_funcinfo << endl;
    m_aboutToFinishNotEmitted = true;
	m_url = url;
    setState(Phonon::LoadingState);
    if (!m_url.isValid() || m_url.isEmpty()) {
        return;
    }
	emit length( totalTime() );
	QMultiMap<QString, QString> metaData;
	metaData.insert( "TITLE", "Fake video" );
	metaData.insert( "ARTIST", "Matthias Kretz" );
	emit metaDataChanged( metaData );
    QTimer::singleShot(50, this, SLOT(loadingComplete()));
}

void MediaObject::openMedia(Phonon::MediaObject::Media media)
{
    switch (media) {
    case Phonon::MediaObject::None:
        setUrl(KUrl());
        break;
    case Phonon::MediaObject::CD:
    case Phonon::MediaObject::DVD:
    case Phonon::MediaObject::VCD:
        break;
    default:
        kError(604) << "media " << media << " not implemented" << endl;
    }
}

void MediaObject::loadingComplete()
{
    if (state() == Phonon::LoadingState) {
        setState(Phonon::StoppedState);
    } else if (state() == Phonon::BufferingState) {
        setState(Phonon::PlayingState);
    }
}

void MediaObject::setAboutToFinishTime( qint32 newAboutToFinishTime )
{
	//kDebug( 604 ) << k_funcinfo << endl;
	m_aboutToFinishTime = newAboutToFinishTime;
	if( currentTime() < totalTime() - m_aboutToFinishTime ) // not about to finish
		m_aboutToFinishNotEmitted = true;
}

void MediaObject::play()
{
	//kDebug( 604 ) << k_funcinfo << endl;
	AbstractMediaProducer::play();
}

void MediaObject::pause()
{
	//kDebug( 604 ) << k_funcinfo << endl;
    if (state() == Phonon::PlayingState || state() == Phonon::BufferingState) {
        AbstractMediaProducer::pause();
    }
}

void MediaObject::stop()
{
    //kDebug(604) << k_funcinfo << endl;
    if (state() == Phonon::PlayingState || state() == Phonon::BufferingState || state() == Phonon::PausedState) {
        AbstractMediaProducer::stop();
        m_aboutToFinishNotEmitted = true;
    }
}

void MediaObject::seek( qint64 time )
{
	//kDebug( 604 ) << k_funcinfo << endl;
	AbstractMediaProducer::seek( time );

	if( currentTime() < totalTime() - m_aboutToFinishTime ) // not about to finish
		m_aboutToFinishNotEmitted = true;
}

void MediaObject::emitTick()
{
	AbstractMediaProducer::emitTick();
	if( currentTime() >= totalTime() - m_aboutToFinishTime ) // about to finish
	{
		if( m_aboutToFinishNotEmitted )
		{
			m_aboutToFinishNotEmitted = false;
			emit aboutToFinish( totalTime() - currentTime() );
		}
	}
	if( currentTime() >= totalTime() ) // finished
	{
		stop();
		emit finished();
	}
}

}}

#include "mediaobject.moc"
// vim: sw=4 ts=4
