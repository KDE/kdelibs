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

#include "testwidget.h"

#include <phonon/mediaobject.h>
#include <phonon/audiopath.h>
#include <phonon/audiooutput.h>
#include <phonon/videopath.h>
#include <phonon/backendcapabilities.h>
#include <phonon/ui/videowidget.h>

#include <QSlider>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klineedit.h>
#include <kurlcompletion.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>

#include <cstdlib>

using namespace Phonon;

TestWidget::TestWidget()
	: QWidget( 0 )
	, m_media( 0 )
	, m_ticking( false )
	, m_ignoreVolumeChange( false )
	, m_length( -1 )
{
	m_output = new AudioOutput( this );
	if( ! m_output )
		exit( 1 );
	m_output->setName( "GUI-Test" );
	path = new AudioPath( this );
	if( ! path )
		exit( 1 );
	path->addOutput( m_output );

	QHBoxLayout* hlayout = new QHBoxLayout( this );

	m_volslider = new QSlider( this );
	m_volslider->setRange( 0, 150 );
	m_volslider->setValue( ( int )( 100 * m_output->volume() ) );
	connect( m_volslider, SIGNAL( valueChanged( int ) ), SLOT( volchanged( int ) ) );
	connect( m_output, SIGNAL( volumeChanged( float ) ), SLOT( slotVolumeChanged( float ) ) );
	hlayout->addWidget( m_volslider );

	QFrame * frame = new QFrame( this );
	hlayout->addWidget( frame );
	QVBoxLayout* vlayout = new QVBoxLayout( frame );

	m_seekslider = new QSlider( frame );
	m_seekslider->setOrientation( Qt::Horizontal );
	connect( m_seekslider, SIGNAL( valueChanged( int ) ), SLOT( seek( int ) ) );
	vlayout->addWidget( m_seekslider );

	m_statelabel = new QLabel( frame );
	vlayout->addWidget( m_statelabel );

	m_pause = new QPushButton( frame );
	m_pause->setText( "pause" );
	vlayout->addWidget( m_pause );

	m_play = new QPushButton( frame );
	m_play->setText( "play" );
	vlayout->addWidget( m_play );

	m_stop = new QPushButton( frame );
	m_stop->setText( "stop" );
	vlayout->addWidget( m_stop );

	KLineEdit * file = new KLineEdit( frame );
	file->setCompletionObject( new KUrlCompletion( KUrlCompletion::FileCompletion ) );
	connect( file, SIGNAL( returnPressed( const QString & ) ), SLOT( loadFile( const QString & ) ) );
	vlayout->addWidget( file );

	QFrame * frame2 = new QFrame( this );
	hlayout->addWidget( frame2 );
	QVBoxLayout* vlayout2 = new QVBoxLayout( frame2 );

	m_volumelabel = new QLabel( frame2 );
	m_volumelabel->setText( QString::number( m_output->volume() ) );
	vlayout2->addWidget( m_volumelabel );

	m_totaltime = new QLabel( frame2 );
	vlayout2->addWidget( m_totaltime );
	
	m_currenttime = new QLabel( frame2 );
	vlayout2->addWidget( m_currenttime );
	
	m_remainingtime = new QLabel( frame2 );
	vlayout2->addWidget( m_remainingtime );
	
	show();

	loadFile( getenv( "PHONON_TESTURL" ) );
}

void TestWidget::volchanged( int v )
{
	m_ignoreVolumeChange = true;
	m_output->setVolume( ( ( float )v ) / 100 );
	m_ignoreVolumeChange = false;
}

void TestWidget::slotVolumeChanged( float v )
{
	m_volumelabel->setText( QString::number( v ) );
	if( !m_ignoreVolumeChange )
		m_volslider->setValue( ( int )( 100 * v ) );
}

void TestWidget::tick( long t )
{
	m_ticking = true;
	m_seekslider->setValue( t );
	m_currenttime->setText( QString::number( t ) );
	m_remainingtime->setText( QString::number( m_length - t ) );
	m_ticking = false;
}

void TestWidget::stateChanged( Phonon::State newstate )
{
	switch( newstate )
	{
		case Phonon::ErrorState:
			m_statelabel->setText( "Error" );
			break;
		case Phonon::LoadingState:
			m_statelabel->setText( "Loading" );
			break;
		case Phonon::StoppedState:
			m_statelabel->setText( "Stopped" );
			break;
		case Phonon::PausedState:
			m_statelabel->setText( "Paused" );
			break;
		case Phonon::BufferingState:
			m_statelabel->setText( "Buffering" );
			break;
		case Phonon::PlayingState:
			m_statelabel->setText( "Playing" );
			break;
	}
}

void TestWidget::seek( int ms )
{
	if( ! m_ticking && m_media )
		m_media->seek( ms );
}

void TestWidget::length( long ms )
{
	m_length = ms;
	m_seekslider->setRange( 0, ms );
	m_totaltime->setText( QString::number( m_length ) );
	m_currenttime->setText( QString::number( m_media->currentTime() ) );
	m_remainingtime->setText( QString::number( m_media->remainingTime() ) );
}

void TestWidget::loadFile( const QString & file )
{
	delete m_media;
	m_media = new MediaObject( this );
	m_media->setUrl( KUrl( file ) );
	m_media->setTickInterval( 100 );
	m_media->addAudioPath( path );
	if( m_media->hasVideo() && BackendCapabilities::self()->supportsVideo() )
	{
		vpath = new VideoPath( vout );
		m_media->addVideoPath( vpath );
		vout = new Ui::VideoWidget( this );
		vpath->addOutput( vout );

		vout->setMinimumSize( 160, 120 );
		vout->setFullScreen( false );
		vout->show();
	}
	stateChanged( m_media->state() );

	connect( m_pause, SIGNAL( clicked() ), m_media, SLOT( pause() ) );
	connect( m_play, SIGNAL( clicked() ), m_media, SLOT( play() ) );
	connect( m_stop, SIGNAL( clicked() ), m_media, SLOT( stop() ) );

	connect( m_media, SIGNAL(   tick( long ) ), SLOT(   tick( long ) ) );
	connect( m_media, SIGNAL( length( long ) ), SLOT( length( long ) ) );
	length( m_media->totalTime() );
	connect( m_media, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
			SLOT( stateChanged( Phonon::State ) ) );
	connect( m_media, SIGNAL( finished() ), qApp, SLOT( quit() ) );
}

#include "testwidget.moc"
// vim: sw=4 ts=4 noet
