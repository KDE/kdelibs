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

#include "mediacontrols.h"
#include "../abstractmediaproducer.h"
#include "../audiooutput.h"
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <QHBoxLayout>
#include <QToolButton>
#include "seekslider.h"
#include "volumeslider.h"

namespace Phonon
{

class MediaControls::Private
{
	public:
		Private()
			: layout( 0 )
			, playButton( 0 )
			, pauseButton( 0 )
			, stopButton( 0 )
			, loopButton( 0 )
			, seekSlider( 0 )
			, volumeSlider( 0 )
			, media( 0 )
		{
		}

		QHBoxLayout* layout;
		QToolButton* playButton;
		QToolButton* pauseButton;
		QToolButton* stopButton;
		QToolButton* loopButton;
		SeekSlider*  seekSlider;
		VolumeSlider* volumeSlider;
		AbstractMediaProducer* media;
};

MediaControls::MediaControls( QWidget* parent )
	: QWidget( parent )
	, d( new Private )
{
	d->playButton = new QToolButton( this );
	d->playButton->setIcon( SmallIcon( "player_play" ) );
	d->playButton->setText( i18n( "&Play" ) );
	d->playButton->setFixedSize( 20, 20 );
	d->playButton->setAutoRaise( true );

	d->pauseButton = new QToolButton( this );
	d->pauseButton->setIcon( SmallIcon( "player_pause" ) );
	d->pauseButton->setText( i18n( "&Pause" ) );
	d->pauseButton->setEnabled( false );
	d->pauseButton->setFixedSize( 20, 20 );
	d->pauseButton->setAutoRaise( true );

	d->stopButton = new QToolButton( this );
	d->stopButton->setIcon( SmallIcon( "player_stop" ) );
	d->stopButton->setText( i18n( "&Stop" ) );
	d->stopButton->setFixedSize( 20, 20 );
	d->stopButton->setAutoRaise( true );

	d->seekSlider = new SeekSlider( this );

	d->volumeSlider = new VolumeSlider( this );
	d->volumeSlider->setOrientation( Qt::Horizontal );
	d->volumeSlider->setFixedWidth( 70 );
	d->volumeSlider->hide();

	d->layout = new QHBoxLayout( this );
	d->layout->setMargin( 0 );
	d->layout->setSpacing( 2 );
	d->layout->addWidget( d->playButton );
	d->layout->addWidget( d->pauseButton );
	d->layout->addWidget( d->stopButton );
	d->layout->addSpacing( 4 );
	d->layout->addWidget( d->seekSlider, 1 );
	d->layout->addSpacing( 4 );
	d->layout->addWidget( d->volumeSlider );
	d->layout->addStretch();
}

MediaControls::~MediaControls()
{
	delete d;
}

bool MediaControls::isSeekSliderVisible() const
{
	return d->seekSlider->isVisible();
}

bool MediaControls::isVolumeControlVisible() const
{
	return d->volumeSlider->isVisible();
}

bool MediaControls::isLoopControlVisible() const
{
	return d->loopButton->isVisible();
}

void MediaControls::setMediaProducer( AbstractMediaProducer* media )
{
	if( !media )
		return;

	d->media = media;
	connect( d->media, SIGNAL( destroyed() ), SLOT( mediaDestroyed() ) );
	connect( d->media, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
		   	SLOT( stateChanged( Phonon::State, Phonon::State ) ) );
	connect( d->playButton, SIGNAL( clicked() ), d->media, SLOT( play() ) );
	connect( d->pauseButton, SIGNAL( clicked() ), d->media, SLOT( pause() ) );
	connect( d->stopButton, SIGNAL( clicked() ), d->media, SLOT( stop() ) );

	d->seekSlider->setMediaProducer( d->media );
}

void MediaControls::setAudioOutput( AudioOutput* audioOutput )
{
	d->volumeSlider->setAudioOutput( audioOutput );
	d->volumeSlider->show();
}

void MediaControls::setSeekSliderVisible( bool vis )
{
	d->seekSlider->setVisible( vis );
}

void MediaControls::setVolumeControlVisible( bool vis )
{
	d->volumeSlider->setVisible( vis );
}

void MediaControls::setLoopControlVisible( bool vis )
{
	d->loopButton->setVisible( vis );
}

void MediaControls::stateChanged( State newstate, State )
{
	switch( newstate )
	{
		case Phonon::LoadingState:
		case Phonon::PausedState:
		case Phonon::StoppedState:
			d->playButton->setEnabled( true );
			d->pauseButton->setEnabled( false );
			break;
		case Phonon::BufferingState:
		case Phonon::PlayingState:
			d->playButton->setEnabled( false );
			d->pauseButton->setEnabled( true );
			break;
		case Phonon::ErrorState:
			return;
	}
}

void MediaControls::mediaDestroyed()
{
	d->media = 0;
}

} // namespace Phonon

#include "mediacontrols.moc"
// vim: sw=4 ts=4 noet
