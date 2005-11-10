/*  This file is part of the KDE project
    Copyright (C) 2004-2005 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "statetest.h"

#include <QTimer>

#include <kdem2m/mediaobject.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kurl.h>
#include <cstdlib>
#include <klocale.h>

using namespace Kdem2m;

kdbgstream& operator<<( kdbgstream & stream, const Kdem2m::State state )
{
	switch( state )
	{
		case Kdem2m::ErrorState:
			stream << "Error";
			break;
		case Kdem2m::LoadingState:
			stream << "LoadingState";
			break;
		case Kdem2m::StoppedState:
			stream << "StoppedState";
			break;
		case Kdem2m::PlayingState:
			stream << "PlayingState";
			break;
		case Kdem2m::BufferingState:
			stream << "BufferingState";
			break;
		case Kdem2m::PausedState:
			stream << "PausedState";
			break;
	}
	return stream;
}

StateTester::StateTester( const KURL & url )
{
	m_url = new KURL( url );
}

void StateTester::run()
{
	/*
	check for correct states:

	- after construction:
	  LoadingState -> ( StoppedState, ErrorState ), StoppedState, ErrorState

	- play()
	  ( StoppedState, PausedState ) -> PlayingState

	- when playing:
	  PlayingState -> BufferingState -> ( PlayingState, ErrorState )

	- pause()
	  ( StoppedState, PlayingState, BufferingState ) -> PausedState

	- stop()
	  ( PlayingState, PausedState, BufferingState ) -> StoppedState
	*/

	//f = Factory::self();
	//kdDebug() << "using backend: " << f->backendName() <<
		//"\n Comment: " << f->backendComment() <<
		//"\n Version: " << f->backendVersion() << endl;

	//c = f->createChannel( "teststates" );

	kdDebug() << "loading " << *m_url << endl;
	player = new MediaObject( *m_url, this );
	connect( player, SIGNAL( stateChanged( Kdem2m::State, Kdem2m::State ) ),
			SLOT( stateChanged( Kdem2m::State, Kdem2m::State ) ) );
	connect( player, SIGNAL( finished() ), kapp, SLOT( quit() ) );

	if( player->state() == Kdem2m::LoadingState )
		kdDebug() << "wait until Kdem2m finished LoadingState" << endl;
	else if( player->state() == Kdem2m::StoppedState )
		testplaying();
	else if( player->state() == Kdem2m::ErrorState )
	{
		kdDebug() << "could not load media. exiting." << endl;
		exit( 0 );
	}
}

void StateTester::stateChanged( Kdem2m::State newstate, Kdem2m::State oldstate )
{
	kdDebug() << "stateChanged( new = " << newstate << ", old = " << oldstate << " )" << endl;
	switch( oldstate )
	{
		case Kdem2m::LoadingState:
			switch( newstate )
			{
				case Kdem2m::ErrorState:
					return;
				case Kdem2m::StoppedState:
					testplaying();
					return;
				default:
					break;
			}
			break;
		case Kdem2m::StoppedState:
			switch( newstate )
			{
				case Kdem2m::PlayingState:
				case Kdem2m::PausedState:
					return;
				default:
					break;
			}
			break;
		case Kdem2m::PlayingState:
			switch( newstate )
			{
				case Kdem2m::BufferingState:
					//testbuffering();
				case Kdem2m::PausedState:
				case Kdem2m::StoppedState:
					return;
				default:
					break;
			}
			break;
		case Kdem2m::BufferingState:
			switch( newstate )
			{
				case Kdem2m::PlayingState:
				case Kdem2m::StoppedState:
				case Kdem2m::PausedState:
				case Kdem2m::ErrorState:
					return;
				default:
					break;
			}
			break;
		case Kdem2m::PausedState:
			switch( newstate )
			{
				case Kdem2m::PlayingState:
				case Kdem2m::StoppedState:
					return;
				default:
					break;
			}
			break;
		case Kdem2m::ErrorState:
			break;
	}

	wrongStateChange();
}

void StateTester::testplaying()
{
	player->play();
	if( player->state() == Kdem2m::StoppedState )
	{
		kdDebug() << "could not play media. exiting." << endl;
		exit( 0 );
	}
	else if( player->state() == Kdem2m::PlayingState )
	{
		player->pause();
		if( player->state() != Kdem2m::PausedState )
			wrongStateChange();
		player->play();
		if( player->state() != Kdem2m::PlayingState )
			wrongStateChange();
		player->stop();
		if( player->state() != Kdem2m::StoppedState )
			wrongStateChange();
		player->play();
		if( player->state() != Kdem2m::PlayingState )
			wrongStateChange();
		player->pause();
		if( player->state() != Kdem2m::PausedState )
			wrongStateChange();
		player->stop();
		if( player->state() != Kdem2m::StoppedState )
			wrongStateChange();
		kdDebug() << "success! playing the last 1/5 of the file now and quit on the finished signal" << endl;
		player->play();
		player->seek( player->totalTime() * 4 / 5 );
	}
}

void StateTester::wrongStateChange()
{
	kdError() << "wrong state change in backend!\n" << kdBacktrace() << endl;
	exit( 1 );
}

static const KCmdLineOptions options[] =
{
	  { "+url", I18N_NOOP( "media file to play" ), 0 },
	  KCmdLineLastOption // End of options.
};

int main( int argc, char ** argv )
{
	KAboutData about( "kdem2mtest", "KDE Multimedia Test",
			"0.1", "Testprogram",
			KAboutData::License_LGPL, 0 );
	about.addAuthor( "Matthias Kretz", 0, "kretz@kde.org" );
	KCmdLineArgs::init( argc, argv, &about );
	KCmdLineArgs::addCmdLineOptions( options );
	KApplication app; // we need it for KTrader

	StateTester* tester = 0;
	if( KCmdLineArgs::parsedArgs()->count() > 0 )
	{
		tester = new StateTester( KCmdLineArgs::parsedArgs()->url( 0 ) );
		QTimer::singleShot( 0, tester, SLOT( run() ) );
	}
	else
	{
		KCmdLineArgs::usage();
		exit( 2 );
	}

	return app.exec();
}

#include "statetest.moc"

// vim: sw=4 ts=4 noet
