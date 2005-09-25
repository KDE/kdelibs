/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "kaudiomanagerplay.h"
#include "kartsserver.h"

#include <soundserver.h>
#include <string>

KAudioManagerPlay::KAudioManagerPlay( KArtsServer * server, const QString & title )
{
	d = new PrivateData;
	d->amanPlay = Arts::DynamicCast( server->server().createObject( "Arts::Synth_AMAN_PLAY" ) );
	d->started = false;
	setTitle( title );
}

KAudioManagerPlay::~KAudioManagerPlay()
{
	stop();
	delete d;
}

Arts::Synth_AMAN_PLAY KAudioManagerPlay::amanPlay()
{
	return d->amanPlay;
}

bool KAudioManagerPlay::isNull() const
{
	if( !this )
		return true;
	return d->amanPlay.isNull();
}

void KAudioManagerPlay::setTitle( const QString & title )
{
	d->amanPlay.title( std::string( title.toLocal8Bit() ) );
}

QString KAudioManagerPlay::title()
{
	return QString::fromLocal8Bit( d->amanPlay.title().c_str() );
}

void KAudioManagerPlay::setAutoRestoreID( const QString & autoRestoreID )
{
	d->amanPlay.autoRestoreID( std::string( autoRestoreID.toLocal8Bit() ) );
}

QString KAudioManagerPlay::autoRestoreID()
{
	return QString::fromLocal8Bit( d->amanPlay.autoRestoreID().c_str() );
}

void KAudioManagerPlay::start()
{
	if( d->started )
		return;

	d->started = true;
	d->amanPlay.start();
}

void KAudioManagerPlay::stop()
{
	if( !d->started )
		return;

	d->started = false;
	d->amanPlay.stop();
}

// vim: sw=4 ts=4
