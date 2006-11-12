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

#include "seekslider.h"
#include "seekslider_p.h"
#include "../abstractmediaproducer.h"
#include <kdebug.h>

namespace Phonon
{

SeekSlider::SeekSlider( QWidget* parent )
	: QWidget( parent )
	, d_ptr( new SeekSliderPrivate( this ) )
{
	Q_D( SeekSlider );
	d->layout.setMargin( 0 );
	d->layout.setSpacing( 2 );
	d->setEnabled( false );
	d->layout.addWidget( &d->icon );
	d->layout.addWidget( &d->slider );
}

SeekSlider::SeekSlider( SeekSliderPrivate& _d, QWidget* parent )
	: QWidget( parent )
	, d_ptr( &_d )
{
	Q_D( SeekSlider );
	d->layout.setMargin( 0 );
	d->layout.setSpacing( 2 );
	d->setEnabled( false );
	d->layout.addWidget( &d->icon );
	d->layout.addWidget( &d->slider );
}

SeekSlider::~SeekSlider()
{
	delete d_ptr;
}

void SeekSlider::setMediaProducer( AbstractMediaProducer* media )
{
	if( !media )
		return;

	Q_D( SeekSlider );
	d->media = media;
	connect( media, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
			SLOT( stateChanged( Phonon::State ) ) );
	connect( &d->slider, SIGNAL( valueChanged( int ) ), SLOT( seek( int ) ) );
	connect( media, SIGNAL( length( qint64 ) ), SLOT( length( qint64 ) ) );
	connect( media, SIGNAL( tick( qint64 ) ), SLOT( tick( qint64 ) ) );
	stateChanged( media->state() );
}

void SeekSlider::seek( int msec )
{
	Q_D( SeekSlider );
	if( ! d->ticking && d->media )
		d->media->seek( msec );
}

void SeekSlider::tick( qint64 msec )
{
	Q_D( SeekSlider );
	d->ticking = true;
	d->slider.setValue( msec );
	d->ticking = false;
}

void SeekSlider::length( qint64 msec )
{
	Q_D( SeekSlider );
	d->slider.setRange( 0, msec );
}

void SeekSliderPrivate::setEnabled( bool x )
{
	slider.setEnabled( x );
	icon.setPixmap( x ? iconPixmap : disabledIconPixmap );
}

void SeekSlider::stateChanged( State newstate )
{
	Q_D( SeekSlider );
	if( !d->media->isSeekable() )
	{
		d->setEnabled( false );
		return;
	}
	switch( newstate )
	{
		case Phonon::PlayingState:
			if( d->media->tickInterval() == 0 )
			{
				// if the tick signal is not enabled the slider is useless
				// set the tickInterval to some common value
				d->media->setTickInterval( 350 );
			}
		case Phonon::BufferingState:
		case Phonon::PausedState:
			d->setEnabled( true );
			break;
		case Phonon::StoppedState:
		case Phonon::LoadingState:
		case Phonon::ErrorState:
			d->setEnabled( false );
			d->slider.setValue( 0 );
			break;
	}
}

void SeekSlider::mediaDestroyed()
{
	Q_D( SeekSlider );
	d->media = 0;
	d->setEnabled( false );
}

bool SeekSlider::hasTracking() const
{
	return d_ptr->slider.hasTracking();
}

void SeekSlider::setTracking( bool tracking )
{
	d_ptr->slider.setTracking( tracking );
}

int SeekSlider::pageStep() const
{
	return d_ptr->slider.pageStep();
}

void SeekSlider::setPageStep( int milliseconds )
{
	d_ptr->slider.setPageStep( milliseconds );
}

int SeekSlider::singleStep() const
{
	return d_ptr->slider.singleStep();
}

void SeekSlider::setSingleStep( int milliseconds )
{
	d_ptr->slider.setSingleStep( milliseconds );
}

bool SeekSlider::isIconVisible() const
{
	Q_D( const SeekSlider );
	return d->icon.isVisible();
}

void SeekSlider::setIconVisible( bool vis )
{
	Q_D( SeekSlider );
	d->icon.setVisible( vis );
}

} // namespace Phonon

#include "seekslider.moc"
// vim: sw=4 ts=4 noet
