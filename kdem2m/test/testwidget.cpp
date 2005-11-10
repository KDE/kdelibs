/*  This file is part of the KDE project
    Copyright (C) 2004 Matthias Kretz <kretz@kde.org>

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

#include "testwidget.h"

#include <kdem2m/mediaobject.h>
#include <kdem2m/audiopath.h>
#include <kdem2m/audiooutput.h>
#include <kdem2m/videopath.h>
#include <kdem2m/videooutput.h>

#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <qslider.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <klineedit.h>
#include <kurlcompletion.h>

using namespace Kdem2m;

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

	( new QHBoxLayout( this ) )->setAutoAdd( true );

	m_volslider = new QSlider( this );
	m_volslider->setRange( 0, 150 );
	m_volslider->setValue( ( int )( 100 * m_output->volume() ) );
	connect( m_volslider, SIGNAL( valueChanged( int ) ), SLOT( volchanged( int ) ) );
	connect( m_output, SIGNAL( volumeChanged( float ) ), SLOT( slotVolumeChanged( float ) ) );

	QFrame * frame = new QFrame( this );
	( new QVBoxLayout( frame ) )->setAutoAdd( true );

	m_seekslider = new QSlider( frame );
	m_seekslider->setOrientation( Qt::Horizontal );
	connect( m_seekslider, SIGNAL( valueChanged( int ) ), SLOT( seek( int ) ) );

	m_statelabel = new QLabel( frame );

	m_pause = new QPushButton( frame );
	m_pause->setText( "pause" );

	m_play = new QPushButton( frame );
	m_play->setText( "play" );

	m_stop = new QPushButton( frame );
	m_stop->setText( "stop" );

	KLineEdit * file = new KLineEdit( frame );
	file->setCompletionObject( new KURLCompletion( KURLCompletion::FileCompletion ) );
	connect( file, SIGNAL( returnPressed( const QString & ) ), SLOT( loadFile( const QString & ) ) );

	QFrame * frame2 = new QFrame( this );
	( new QVBoxLayout( frame2 ) )->setAutoAdd( true );

	m_volumelabel = new QLabel( frame2 );
	m_volumelabel->setText( QString::number( m_output->volume() ) );

	m_totaltime = new QLabel( frame2 );
	
	m_currenttime = new QLabel( frame2 );
	
	m_remainingtime = new QLabel( frame2 );
	
	show();

	//loadFile( "/home/mkretz/Videos/qt4_music_video_VCD.mpg" );
	loadFile( "/home/mkretz/Musik/qt23.mp3" );
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

void TestWidget::stateChanged( Kdem2m::State newstate )
{
	switch( newstate )
	{
		case Kdem2m::ErrorState:
			m_statelabel->setText( "Error" );
			break;
		case Kdem2m::LoadingState:
			m_statelabel->setText( "Loading" );
			break;
		case Kdem2m::StoppedState:
			m_statelabel->setText( "Stopped" );
			break;
		case Kdem2m::PausedState:
			m_statelabel->setText( "Paused" );
			break;
		case Kdem2m::BufferingState:
			m_statelabel->setText( "Buffering" );
			break;
		case Kdem2m::PlayingState:
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
	m_media = new MediaObject( KURL( file ), this );
	m_media->setTickInterval( 100 );
	m_media->addAudioPath( path );
	if( m_media->hasVideo() )
	{
		vout = new VideoOutput( this );
		vpath = new VideoPath( vout );
		vpath->addOutput( vout );
		m_media->addVideoPath( vpath );
	}
	stateChanged( m_media->state() );

	connect( m_pause, SIGNAL( clicked() ), m_media, SLOT( pause() ) );
	connect( m_play, SIGNAL( clicked() ), m_media, SLOT( play() ) );
	connect( m_stop, SIGNAL( clicked() ), m_media, SLOT( stop() ) );

	connect( m_media, SIGNAL(   tick( long ) ), SLOT(   tick( long ) ) );
	connect( m_media, SIGNAL( length( long ) ), SLOT( length( long ) ) );
	length( m_media->totalTime() );
	connect( m_media, SIGNAL( stateChanged( Kdem2m::State, Kdem2m::State ) ),
			SLOT( stateChanged( Kdem2m::State ) ) );
	connect( m_media, SIGNAL( finished() ), qApp, SLOT( quit() ) );
}

#include "testwidget.moc"
// vim: sw=4 ts=4 noet
