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

#include "phononnamespace.h"
#include <kdebug.h>
#include <klocale.h>

namespace Phonon
{
	QString categoryToString( Category c )
	{
		switch( c )
		{
			case Phonon::UnspecifiedCategory:
				return i18n( "Unspecified" );
			case Phonon::NotificationCategory:
				return i18n( "Notifications" );
			case Phonon::MusicCategory:
				return i18n( "Music" );
			case Phonon::VideoCategory:
				return i18n( "Video" );
			case Phonon::CommunicationCategory:
				return i18n( "Communication" );
			case Phonon::GameCategory:
				return i18n( "Games" );
		}
		return QString();
	}
}
/*
kdbgstream& operator<<( kdbgstream & stream, const Phonon::State state )
{
	switch( state )
	{
		case Phonon::ErrorState:
			stream << "Error";
			break;
		case Phonon::LoadingState:
			stream << "LoadingState";
			break;
		case Phonon::StoppedState:
			stream << "StoppedState";
			break;
		case Phonon::PlayingState:
			stream << "PlayingState";
			break;
		case Phonon::BufferingState:
			stream << "BufferingState";
			break;
		case Phonon::PausedState:
			stream << "PausedState";
			break;
	}
	return stream;
}
*/
// vim: sw=4 ts=4 noet
